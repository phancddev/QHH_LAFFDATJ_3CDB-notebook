# Hướng dẫn cài đặt

## 1. Tạo môi trường ảo Python

```bash
python3 -m venv venv
source venv/bin/activate  # macOS/Linux
```

## 2. Cài đặt dependencies Python (nếu có)

```bash
pip install -r requirements.txt
```

## 3. Cài đặt LaTeX (lualatex)

**LƯU Ý QUAN TRỌNG:** `lualatex` KHÔNG thể cài qua pip vì nó là binary executable, không phải Python package.

### Cách 1: Cài MacTeX qua Homebrew (Khuyến nghị - nhẹ hơn)

```bash
brew install --cask mactex-no-gui
```

Sau khi cài xong, thêm vào PATH:

```bash
export PATH="/Library/TeX/texbin:$PATH"
```

Hoặc thêm vào `~/.zshrc` hoặc `~/.bash_profile`:

```bash
echo 'export PATH="/Library/TeX/texbin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Cách 2: Tải MacTeX trực tiếp (Full version - ~4GB)

Tải từ: https://www.tug.org/mactex/mactex-download.html

### Cách 3: Cài BasicTeX (Nhẹ nhất - ~100MB)

```bash
brew install --cask basictex
```

Sau đó cài thêm các package cần thiết:

```bash
sudo tlmgr update --self
sudo tlmgr install collection-fontsrecommended
sudo tlmgr install fontspec
sudo tlmgr install import
```

## 4. Kiểm tra cài đặt

```bash
which lualatex
lualatex --version
```

## 5. Build notebook

```bash
python make.py
```

Hoặc build thủ công:

```bash
lualatex build.tex
lualatex build.tex  # Chạy 2 lần để cập nhật mục lục
```

## 6. Kết quả

File PDF sẽ được tạo tại: `build.pdf`

## Troubleshooting

### Lỗi: `lualatex: command not found`

- Kiểm tra PATH: `echo $PATH | grep texbin`
- Nếu không có, thêm vào PATH như hướng dẫn ở trên
- Restart terminal sau khi cài đặt

### Lỗi: Missing packages

```bash
sudo tlmgr install <package-name>
```

### Lỗi: Permission denied

```bash
sudo chown -R $(whoami) /usr/local/texlive
```

