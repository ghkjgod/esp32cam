@echo off
echo ESP32项目编译测试
echo ==================

REM 加载配置
call config.bat

echo 验证环境配置...
call config.bat verify

echo.
echo 设置ESP-IDF环境...
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python_env\idf5.5_py3.11_env
set PYTHONPATH=%IDF_PATH%\tools

if exist "%IDF_PATH%\export.bat" (
    call "%IDF_PATH%\export.bat"
    echo ✓ ESP-IDF环境已设置
) else (
    echo ✗ ESP-IDF环境设置失败
    echo 请检查config.bat中的路径配置
    pause
    exit /b 1
)

echo.
echo 检查项目配置...
if not exist "build\build.ninja" (
    echo 项目未配置，正在配置...
    idf.py reconfigure
    
    if %ERRORLEVEL% NEQ 0 (
        echo ✗ 项目配置失败
        pause
        exit /b 1
    )
    echo ✓ 项目配置完成
) else (
    echo ✓ 项目已配置
)

echo.
echo 开始编译测试...
echo 使用参数：
echo   -j1    : 单线程编译（减少内存使用）
echo   --verbose : 显示详细信息
echo.

idf.py build -j1 --verbose

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =============================
    echo ✓ 编译测试成功！
    echo =============================
    echo.
    echo 生成的文件：
    if exist "build\espcam.bin" (
        echo ✓ 固件文件: build\espcam.bin
        for %%A in ("build\espcam.bin") do echo   大小: %%~zA 字节
    )
    if exist "build\espcam.elf" (
        echo ✓ ELF文件: build\espcam.elf
    )
    if exist "build\partition_table\partition-table.bin" (
        echo ✓ 分区表: build\partition_table\partition-table.bin
    )
    
    echo.
    echo 下一步可以：
    echo   idf.py flash          - 烧录到设备
    echo   idf.py monitor        - 监控串口输出
    echo   idf.py flash monitor  - 烧录并监控
    
) else (
    echo.
    echo =============================
    echo ✗ 编译测试失败
    echo =============================
    echo 错误代码: %ERRORLEVEL%
    echo.
    echo 常见问题解决方案：
    echo.
    echo 1. 内存不足：
    echo    - 关闭其他程序释放内存
    echo    - 确保至少有4GB可用内存
    echo.
    echo 2. 头文件问题：
    echo    - 运行 fix_vscode.bat 修复IntelliSense
    echo    - 检查managed_components是否完整
    echo.
    echo 3. 配置问题：
    echo    - 删除build目录重新配置
    echo    - 检查sdkconfig配置
    echo.
    echo 4. 工具链问题：
    echo    - 检查config.bat中的路径设置
    echo    - 确保所有工具都已正确安装
    echo.
    echo 如需详细错误信息，请查看上方的编译输出
)

echo.
pause
