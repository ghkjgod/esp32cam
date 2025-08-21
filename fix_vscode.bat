@echo off
echo VS Code IntelliSense修复工具
echo ============================

REM 加载配置
call config.bat

echo 步骤1：修复compile_commands.json路径...
if exist "fix_paths.py" (
    if exist "%PYTHON_PATH%" (
        "%PYTHON_PATH%" fix_paths.py
    ) else (
        python fix_paths.py
    )
    echo ✓ 路径修复完成
) else (
    echo ⚠ fix_paths.py文件不存在，跳过路径修复
)

echo.
echo 步骤2：重新生成compile_commands.json...
if exist build (
    echo 清理旧的编译配置...
    if exist build\compile_commands.json (
        del build\compile_commands.json
    )
)

echo 设置环境变量...
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python_env\idf5.5_py3.11_env
set PYTHONPATH=%IDF_PATH%\tools

echo 重新配置项目...
if exist "%CMAKE_PATH%" (
    "%CMAKE_PATH%" -G "Ninja" -DPYTHON_DEPS_CHECKED=1 -DESP_PLATFORM=1 -DIDF_TARGET=esp32s3 -DIDF_PATH=%IDF_PATH% -B build -S .
    
    if %ERRORLEVEL% EQU 0 (
        echo ✓ 项目重新配置成功
        
        if exist "build\compile_commands.json" (
            echo ✓ compile_commands.json已生成
            
            REM 再次修复路径
            if exist "fix_paths.py" (
                if exist "%PYTHON_PATH%" (
                    "%PYTHON_PATH%" fix_paths.py
                ) else (
                    python fix_paths.py
                )
            )
        ) else (
            echo ✗ compile_commands.json生成失败
        )
    ) else (
        echo ✗ 项目配置失败
    )
) else (
    echo ✗ CMake未找到，请检查config.bat配置
)

echo.
echo 步骤3：更新VS Code配置...
if not exist ".vscode" mkdir .vscode

echo 创建/更新c_cpp_properties.json...
(
echo {
echo     "configurations": [
echo         {
echo             "name": "ESP-IDF",
echo             "compilerPath": "%GCC_PATH:\=\\%",
echo             "compileCommands": "${workspaceFolder}/build/compile_commands.json",
echo             "forcedInclude": [
echo                 "${workspaceFolder}/build/config/sdkconfig.h"
echo             ],
echo             "includePath": [
echo                 "${workspaceFolder}/build/config",
echo                 "${workspaceFolder}/main",
echo                 "${workspaceFolder}/managed_components/**",
echo                 "%IDF_PATH:\=/%/components/**"
echo             ],
echo             "defines": [
echo                 "ESP_PLATFORM=1",
echo                 "IDF_VER=\"v5.5\"",
echo                 "CONFIG_IDF_TARGET_ESP32S3=1",
echo                 "CONFIG_IDF_TARGET=\"esp32s3\""
echo             ],
echo             "cStandard": "gnu17",
echo             "cppStandard": "gnu++17",
echo             "intelliSenseMode": "gcc-x64"
echo         }
echo     ],
echo     "version": 4
echo }
) > .vscode\c_cpp_properties.json

echo ✓ c_cpp_properties.json已更新

echo.
echo 步骤4：更新VS Code设置...
(
echo {
echo     "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
echo     "C_Cpp.errorSquiggles": "enabled",
echo     "files.associations": {
echo         "*.h": "c",
echo         "*.c": "c",
echo         "esp_camera.h": "c",
echo         "freertos.h": "c",
echo         "esp_err.h": "c",
echo         "esp_log.h": "c"
echo     }
echo }
) > .vscode\settings_intellisense.json

echo ✓ VS Code设置已更新

echo.
echo =============================
echo ✓ VS Code IntelliSense修复完成
echo =============================
echo.
echo 下一步：
echo 1. 在VS Code中按 Ctrl+Shift+P
echo 2. 输入 "Developer: Reload Window"
echo 3. 执行命令重新加载窗口
echo 4. 如果仍有错误，运行 "C/C++: Reset IntelliSense Database"
echo.
pause
