# ESP32 Camera Project - 脚本快速使用指南

## 🚀 快速开始

### 1️⃣ 配置路径（首次使用或迁移项目）
编辑 `config.bat` 文件，修改以下路径：
```batch
set IDF_PATH=d:\workspace\esp\v5.5\esp-idf
set IDF_TOOLS_PATH=d:\workspace\idftools
set PYTHON_PATH=d:\workspace\idftools\tools\idf-python\3.11.2\python.exe
```

### 2️⃣ 验证配置
```cmd
config.bat verify
```

### 3️⃣ 设置开发环境
```cmd
setup_env.bat
```

### 4️⃣ 修复VS Code（如果需要）
```cmd
fix_vscode.bat
```

### 5️⃣ 测试编译
```cmd
build_test.bat
```

---

## 📋 脚本功能一览

| 脚本 | 功能 | 使用场景 |
|------|------|----------|
| `config.bat` | 路径配置 | 项目迁移、首次设置 |
| `setup_env.bat` | 环境设置 | 开始开发、日常使用 |
| `fix_vscode.bat` | 修复VS Code | 头文件错误、代码补全问题 |
| `build_test.bat` | 编译测试 | 验证编译、排查问题 |
| `diagnose_headers.bat` | 头文件诊断 | 快速定位头文件问题 |
| `fix_paths.py` | 路径修复 | compile_commands.json路径错误 |

---

## 🔧 常见问题解决

### ❌ 头文件找不到
```cmd
diagnose_headers.bat
fix_vscode.bat
```

### ❌ VS Code IntelliSense不工作
```cmd
fix_vscode.bat
# 然后在VS Code中: Ctrl+Shift+P -> "Developer: Reload Window"
```

### ❌ 编译失败
```cmd
build_test.bat
# 查看详细错误信息和解决建议
```

### ❌ 项目迁移到新电脑
1. 修改 `config.bat` 中的路径
2. 运行 `config.bat verify`
3. 运行 `setup_env.bat`
4. 运行 `fix_vscode.bat`

---

## 💡 使用技巧

- **日常开发**: 只需运行 `setup_env.bat` 启动环境
- **问题诊断**: 先运行 `diagnose_headers.bat` 快速定位问题
- **配置验证**: 定期运行 `config.bat verify` 检查环境
- **VS Code重置**: 遇到IntelliSense问题就运行 `fix_vscode.bat`

---

## 📖 详细文档

完整的脚本说明请查看：[SCRIPTS_DOCUMENTATION.md](SCRIPTS_DOCUMENTATION.md)
