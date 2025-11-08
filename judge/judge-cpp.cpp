#include <bits/stdc++.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;

// ---- Tien ich nho ----
static inline bool file_exists(const string& p){
    return ::access(p.c_str(), F_OK) == 0;
}

int compile_cpp(const string& src, const string& outExe, string logPath){
    string cmd = "g++ -std=c++17 -O2 -pipe \"" + src + "\" -o \"" + outExe + "\" 2>\"" + logPath + "\"";
    return ::system(cmd.c_str());
}

struct RunResult {
    int exit_code = -1;
    bool signaled = false;
    int term_signal = 0;
    bool tle = false;
    bool mle = false;
    double wall_time_sec = 0.0; // do bang CLOCK_MONOTONIC
    long long max_rss_kb = 0;   // tu getrusage
};

// Doc ru_maxrss tu getrusage(RUSAGE_CHILDREN) sau khi wait
static inline long long get_children_maxrss_kb(){
    struct rusage ru{};
    if (getrusage(RUSAGE_CHILDREN, &ru) == 0){
        return (long long)ru.ru_maxrss; // Linux: KB
    }
    return 0;
}

// Chay chuong trinh con duoi gioi han, do thoi gian, bo nho
RunResult run_with_limits(const string& exe,
                          const string& inFile,
                          const string& outFile,
                          double timeLimitSec, // wall time
                          long long memLimitMB){
    RunResult rr;
    // Mo file in/out
    int fin = ::open(inFile.c_str(), O_RDONLY);
    if (fin < 0){ perror("Mo input that bai"); }
    int fout = ::open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fout < 0){ perror("Mo output that bai"); }

    // Gioi han bo nho (RLIMIT_AS) -> bytes
    rlimit mem{};
    mem.rlim_cur = mem.rlim_max = (rlim_t)memLimitMB * 1024ull * 1024ull;

    // Thoi diem bat dau
    timespec t0{}, t1{};
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pid_t pid = fork();
    if (pid == -1){
        perror("fork that bai");
        if (fin>=0) close(fin);
        if (fout>=0) close(fout);
        return rr;
    }
    if (pid == 0){
        // Child: dat gioi han, chuyen huong IO, exec
        if (setrlimit(RLIMIT_AS, &mem) != 0){
            // khong dat duoc gioi han -> van thu chay
        }
        // Co the gioi han thoi luong CPU (nguyen giay) neu muon:
        // rlimit cpu{}; cpu.rlim_cur = cpu.rlim_max = (rlim_t)ceil(timeLimitSec);
        // setrlimit(RLIMIT_CPU, &cpu);

        if (fin >= 0) dup2(fin, STDIN_FILENO);
        if (fout >= 0) dup2(fout, STDOUT_FILENO);
        // Dong cac fd thua
        if (fin >= 0) close(fin);
        if (fout >= 0) close(fout);

        // Thuc thi
        execl(exe.c_str(), exe.c_str(), (char*)nullptr);
        // Neu exec loi
        _exit(127);
    }

    // Parent: theo doi thoi gian, kill neu qua han
    if (fin>=0) close(fin);
    if (fout>=0) close(fout);

    const int poll_ms = 5; // tan so kiem tra
    int status = 0;
    while (true){
        pid_t r = waitpid(pid, &status, WNOHANG);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
        if (r == 0){
            // con dang chay
            if (elapsed > timeLimitSec){
                rr.tle = true;
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                break;
            }
            // ngu 1 chut
            this_thread::sleep_for(chrono::milliseconds(poll_ms));
        } else if (r == pid){
            // da ket thuc
            break;
        } else {
            // loi
            break;
        }
    }

    // Cap nhat thoi gian
    clock_gettime(CLOCK_MONOTONIC, &t1);
    rr.wall_time_sec = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;

    // Trang thai thoat
    if (WIFEXITED(status)){
        rr.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)){
        rr.signaled = true;
        rr.term_signal = WTERMSIG(status);
    }
    // Peak RSS (KB) cua tat ca child da doi
    rr.max_rss_kb = get_children_maxrss_kb();

    // Phan doan MLE: neu peak RSS > gioi han (tinh theo MB)
    double usedMB = rr.max_rss_kb / 1024.0;
    if (usedMB > memLimitMB) rr.mle = true;

    return rr;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string modelCpp, genCpp, solCpp;
    string timeStr;
    double timeLimit = 0.0;
    long long memLimitMB = 0;

    cout << "=== Cau hinh ===\n";
    cout << "Duong dan file code mau (.cpp): ";
    getline(cin, modelCpp);
    cout << "Duong dan file sinh input (.cpp): ";
    getline(cin, genCpp);
    cout << "Duong dan file can check (.cpp): ";
    getline(cin, solCpp);
    cout << "Gioi han bo nho (MB): ";
    {
        string tmp; getline(cin, tmp);
        memLimitMB = atoll(tmp.c_str());
    }
    cout << "Gioi han thoi gian (vi du 0.5s hoac 0.5): ";
    getline(cin, timeStr);
    {
        string s = timeStr;
        if (!s.empty() && s.back()=='s') s.pop_back();
        timeLimit = atof(s.c_str());
        if (timeLimit <= 0){
            cerr << "Thoi gian khong hop le.\n";
            return 1;
        }
    }

    // Kiem tra file ton tai
    if (!file_exists(modelCpp) || !file_exists(genCpp) || !file_exists(solCpp)){
        cerr << "Khong tim thay mot hoac nhieu file .cpp.\n";
        return 1;
    }

    // Thu muc tam
    char templ[] = "/tmp/judgeXXXXXX";
    char* wd = mkdtemp(templ);
    if (!wd){ perror("mkdtemp"); return 1; }
    string WORKDIR = wd;
    string modelExe = WORKDIR + "/model";
    string genExe   = WORKDIR + "/gen";
    string solExe   = WORKDIR + "/sol";
    string inputTxt = WORKDIR + "/input.txt";
    string ansTxt   = WORKDIR + "/answer.txt";
    string outTxt   = WORKDIR + "/output.txt";

    cout << "\n=== Bien dich ===\n";
    cout << "- Bien dich code mau...\n";
    if (compile_cpp(modelCpp, modelExe, WORKDIR+"/compile_model.log") != 0){
        cerr << "Loi bien dich code mau. Xem " << WORKDIR << "/compile_model.log\n";
        return 1;
    }
    cout << "- Bien dich sinh input...\n";
    if (compile_cpp(genCpp, genExe, WORKDIR+"/compile_gen.log") != 0){
        cerr << "Loi bien dich sinh input. Xem " << WORKDIR << "/compile_gen.log\n";
        return 1;
    }
    cout << "- Bien dich code can check...\n";
    if (compile_cpp(solCpp, solExe, WORKDIR+"/compile_sol.log") != 0){
        cerr << "Loi bien dich code can check. Xem " << WORKDIR << "/compile_sol.log\n";
        return 1;
    }

    cout << "\n=== Sinh du lieu va dap an ===\n";
    {
        // gen -> input.txt
        RunResult rg = run_with_limits(genExe, "/dev/null", inputTxt, 10.0, 1024 /*1GB de rong rai*/);
        if (rg.exit_code != 0 || rg.signaled){
            cerr << "Loi khi chay file sinh input. Ma thoat: " << rg.exit_code << "\n";
            return 1;
        }
    }
    {
        // model < input -> answer.txt
        // cho model han 10s va 1024 MB cho an toan
        RunResult rm = run_with_limits(modelExe, inputTxt, ansTxt, 10.0, 1024);
        if (rm.exit_code != 0 || rm.signaled){
            cerr << "Loi khi chay code mau. Ma thoat: " << rm.exit_code << "\n";
            return 1;
        }
    }

    cout << "\n=== Chay code can check voi gioi han ===\n";
    cout << "Gioi han thoi gian: " << timeLimit << "s\n";
    cout << "Gioi han bo nho: " << memLimitMB << " MB\n";

    RunResult rs = run_with_limits(solExe, inputTxt, outTxt, timeLimit, memLimitMB);

    string verdict;
    if (rs.tle){
        verdict = "TLE";
    } else if (rs.mle){
        verdict = "MLE";
    } else if (rs.signaled){
        // Neu chet do SIGSEGV hoac khac, khong TLE/MLE, xem la WA (hoac RTE tuy y)
        verdict = "WA";
    } else {
        // So sanh ket qua
        // So sanh nhi phan: giong y he
        string diffCmd = "diff -q \"" + ansTxt + "\" \"" + outTxt + "\" > /dev/null 2>&1";
        int d = ::system(diffCmd.c_str());
        verdict = (d==0) ? "AC" : "WA";
    }

    cout << "=== Ket qua ===\n";
    cout << "Phan dinh: " << verdict << "\n";
    cout << "Thoi gian su dung: " << fixed << setprecision(3) << rs.wall_time_sec << "s\n";
    cout << "Bo nho dinh cao: " << (rs.max_rss_kb/1024.0) << " MB (gioi han " << memLimitMB << " MB)\n";
    cout << "\nTep tam: " << WORKDIR << "\n";
    cout << "Dang xuat:\n";
    cout << "  - " << WORKDIR << "/compile_model.log\n";
    cout << "  - " << WORKDIR << "/compile_gen.log\n";
    cout << "  - " << WORKDIR << "/compile_sol.log\n";

    return 0;
}

