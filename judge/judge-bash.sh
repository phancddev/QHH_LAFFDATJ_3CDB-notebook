#!/usr/bin/env bash
set -u

# Kiem tra cong cu can thiet (Linux)
for cmd in g++ timeout /usr/bin/time diff awk sed; do
  if ! command -v ${cmd%% *} >/dev/null 2>&1; then
    echo "Thieu cong cu: $cmd. Vui long cai dat truoc." >&2
    echo "Go y cai dat tren Ubuntu: sudo apt-get update && sudo apt-get install -y g++ time coreutils diffutils"
    exit 1
  fi
done

echo "=== Cau hinh ==="
read -rp "Duong dan file code mau (.cpp): " MODEL_CPP
read -rp "Duong dan file sinh input (.cpp): " GEN_CPP
read -rp "Duong dan file can check (.cpp): " SOL_CPP
read -rp "Gioi han bo nho (MB): " MEM_MB
read -rp "Gioi han thoi gian (vi du 0.5s): " TL_IN

# Chuan hoa thoi gian
if [[ "$TL_IN" =~ ^[0-9]+(\.[0-9]+)?s$ ]]; then
  TL="$TL_IN"
elif [[ "$TL_IN" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
  TL="${TL_IN}s"
else
  echo "Dinh dang thoi gian khong hop le. Vi du: 1, 0.5, 0.5s" >&2
  exit 1
fi

# Kiem tra file
for f in "$MODEL_CPP" "$GEN_CPP" "$SOL_CPP"; do
  [[ -f "$f" ]] || { echo "Khong tim thay file: $f" >&2; exit 1; }
done

WORKDIR="$(mktemp -d -t judge-XXXXXX)"
cleanup(){ rm -rf "$WORKDIR"; }
trap cleanup EXIT

MODEL_EXE="$WORKDIR/model"
GEN_EXE="$WORKDIR/gen"
SOL_EXE="$WORKDIR/sol"

INPUT_TXT="$WORKDIR/input.txt"
ANS_TXT="$WORKDIR/answer.txt"
OUT_TXT="$WORKDIR/output.txt"
METRICS_TXT="$WORKDIR/metrics.txt"
RUN_LOG="$WORKDIR/run.log"

echo
echo "=== Bien dich (Linux) ==="
echo "- Bien dich code mau..."
if ! g++ -std=c++17 -O2 -pipe "$MODEL_CPP" -o "$MODEL_EXE" 2>"$WORKDIR/compile_model.log"; then
  echo "Loi bien dich code mau. Xem $WORKDIR/compile_model.log" >&2; exit 1
fi
echo "- Bien dich sinh input..."
if ! g++ -std=c++17 -O2 -pipe "$GEN_CPP" -o "$GEN_EXE" 2>"$WORKDIR/compile_gen.log"; then
  echo "Loi bien dich sinh input. Xem $WORKDIR/compile_gen.log" >&2; exit 1
fi
echo "- Bien dich code can check..."
if ! g++ -std=c++17 -O2 -pipe "$SOL_CPP" -o "$SOL_EXE" 2>"$WORKDIR/compile_sol.log"; then
  echo "Loi bien dich code can check. Xem $WORKDIR/compile_sol.log" >&2; exit 1
fi

echo
echo "=== Sinh du lieu va dap an ==="
echo "- Chay file sinh input -> $INPUT_TXT"
if ! "$GEN_EXE" > "$INPUT_TXT" 2>"$WORKDIR/gen.log"; then
  echo "Loi khi chay file sinh input. Xem $WORKDIR/gen.log" >&2; exit 1
fi

echo "- Chay code mau -> $ANS_TXT"
if ! "$MODEL_EXE" < "$INPUT_TXT" > "$ANS_TXT" 2>"$WORKDIR/model.log"; then
  echo "Loi khi chay code mau. Xem $WORKDIR/model.log" >&2; exit 1
fi

echo
echo "=== Chay code can check voi gioi han Linux ==="
echo "Gioi han thoi gian: $TL"
echo "Gioi han bo nho: ${MEM_MB} MB"
: > "$METRICS_TXT"; : > "$RUN_LOG"

# Gioi han bo nho bang ulimit (KB)
MEM_KB_LIMIT=$(( MEM_MB * 1024 ))

# Thuc thi trong subshell de ulimit chi anh huong tien trinh con
(
  ulimit -v "$MEM_KB_LIMIT" 2>/dev/null || true
  /usr/bin/time -f "TIME:%e\nMEM_KB:%M" -o "$METRICS_TXT" \
    timeout --preserve-status "$TL" "$SOL_EXE" < "$INPUT_TXT" > "$OUT_TXT" 2> "$RUN_LOG"
)
EXEC_STATUS=$?

TIME_SEC=$(sed -n 's/^TIME:\(.*\)$/\1/p' "$METRICS_TXT")
MEM_KB=$(sed -n 's/^MEM_KB:\(.*\)$/\1/p' "$METRICS_TXT")
[[ -z "${TIME_SEC:-}" ]] && TIME_SEC="0"
[[ -z "${MEM_KB:-}" ]] && MEM_KB="0"
MEM_MB_USED=$(awk -v kb="$MEM_KB" 'BEGIN{printf "%.2f", kb/1024}')

if [[ $EXEC_STATUS -eq 124 ]]; then
  VERDICT="TLE"
else
  # Danh gia MLE: du do MEM_KB (RSS dinh cao) > gioi han nhap
  MLE_FLAG=$(awk -v used="$MEM_MB_USED" -v lim="$MEM_MB" 'BEGIN{if (used>lim) print 1; else print 0}')
  if [[ "$MLE_FLAG" -eq 1 ]]; then
    VERDICT="MLE"
  else
    if diff -q "$ANS_TXT" "$OUT_TXT" >/dev/null 2>&1; then
      VERDICT="AC"
    else
      VERDICT="WA"
    fi
  fi
fi

echo "=== Ket qua ==="
echo "Phan dinh: $VERDICT"
echo "Thoi gian su dung: ${TIME_SEC}s"
echo "Bo nho dinh cao: ${MEM_MB_USED} MB (gioi han ${MEM_MB} MB)"
echo
echo "Tep tam: $WORKDIR"
echo "Log:"
echo "  - $WORKDIR/compile_model.log"
echo "  - $WORKDIR/compile_gen.log"
echo "  - $WORKDIR/compile_sol.log"
echo "  - $WORKDIR/gen.log"
echo "  - $WORKDIR/model.log"
echo "  - $RUN_LOG"

