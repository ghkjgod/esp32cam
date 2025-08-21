@echo off
echo ESP32开发环境设置
echo ==================

REM 加载配置
call config.bat

echo 设置ESP-IDF环境变量...
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python_env\idf5.5_py3.11_env
set PYTHONPATH=%IDF_PATH%\tools

echo 验证工具可用性...
call config.bat verify

echo.
echo 启动ESP-IDF环境...
if exist "%IDF_PATH%\export.bat" (
    call "%IDF_PATH%\export.bat"
    echo ✓ ESP-IDF环境已设置
    echo.
    echo 可用命令：
    echo   idf.py build          - 编译项目
    echo   idf.py flash          - 烧录固件
    echo   idf.py monitor        - 监控串口
    echo   idf.py flash monitor  - 烧录并监控
    echo   idf.py menuconfig     - 配置菜单
    echo.
    echo 环境设置完成！请在此窗口中使用ESP-IDF命令。
    cmd /k
) else (
    echo ✗ ESP-IDF export.bat文件不存在
    echo 请检查config.bat中的IDF_PATH设置
    pause
)
