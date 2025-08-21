@echo off
echo 头文件诊断工具
echo ================

REM 加载配置
call config.bat

echo 步骤1：检查基本环境...
echo IDF_PATH=%IDF_PATH%
echo IDF_TOOLS_PATH=%IDF_TOOLS_PATH%
echo.

echo 步骤2：检查关键目录...
if exist "%IDF_PATH%" (
    echo ✓ ESP-IDF目录存在
) else (
    echo ✗ ESP-IDF目录不存在: %IDF_PATH%
    echo 请检查config.bat中的IDF_PATH设置
    goto end
)

if exist "%IDF_PATH%\components" (
    echo ✓ ESP-IDF组件目录存在
) else (
    echo ✗ ESP-IDF组件目录不存在
    goto end
)

if exist "managed_components" (
    echo ✓ 项目managed_components目录存在
) else (
    echo ✗ 项目managed_components目录不存在
    echo 请运行 idf.py reconfigure 来下载组件
)

echo.
echo 步骤3：检查关键头文件...

REM 检查ESP-IDF核心头文件
set "CORE_HEADERS=esp_err.h esp_log.h esp_system.h freertos/FreeRTOS.h"
for %%h in (%CORE_HEADERS%) do (
    if exist "%IDF_PATH%\components\*\include\%%h" (
        echo ✓ 找到核心头文件: %%h
    ) else (
        echo ✗ 未找到核心头文件: %%h
        echo   搜索路径: %IDF_PATH%\components\*\include\
    )
)

REM 检查相机头文件
if exist "managed_components\espressif__esp32-camera\driver\include\esp_camera.h" (
    echo ✓ 找到相机头文件: esp_camera.h
) else (
    echo ✗ 未找到相机头文件: esp_camera.h
    echo   预期路径: managed_components\espressif__esp32-camera\driver\include\
    if exist "managed_components\espressif__esp32-camera" (
        echo   esp32-camera组件存在，但头文件路径可能不正确
        dir /s "managed_components\espressif__esp32-camera\*.h" | findstr "esp_camera.h"
    ) else (
        echo   esp32-camera组件不存在，请运行 idf.py reconfigure
    )
)

echo.
echo 步骤4：检查编译配置...
if exist "build\compile_commands.json" (
    echo ✓ compile_commands.json存在
    
    REM 检查文件大小
    for %%A in ("build\compile_commands.json") do (
        if %%~zA GTR 1000 (
            echo ✓ compile_commands.json大小正常: %%~zA 字节
        ) else (
            echo ⚠ compile_commands.json文件过小: %%~zA 字节
            echo   可能配置不完整，建议重新生成
        )
    )
    
    REM 检查路径是否正确
    findstr /C:"d:/workspace/project/espcam" "build\compile_commands.json" >nul
    if %ERRORLEVEL% EQU 0 (
        echo ✓ compile_commands.json包含正确的项目路径
    ) else (
        echo ⚠ compile_commands.json可能包含错误路径
        echo   建议运行 fix_vscode.bat 修复
    )
) else (
    echo ✗ compile_commands.json不存在
    echo   请运行 idf.py reconfigure 生成
)

echo.
echo 步骤5：检查VS Code配置...
if exist ".vscode\c_cpp_properties.json" (
    echo ✓ VS Code C/C++配置存在
    
    REM 检查配置内容
    findstr /C:"ESP-IDF" ".vscode\c_cpp_properties.json" >nul
    if %ERRORLEVEL% EQU 0 (
        echo ✓ 配置包含ESP-IDF设置
    ) else (
        echo ⚠ 配置可能不完整
    )
) else (
    echo ✗ VS Code C/C++配置不存在
    echo   建议运行 fix_vscode.bat 创建配置
)

echo.
echo 步骤6：检查项目文件...
if exist "main\main.c" (
    echo ✓ 主程序文件存在
    
    REM 检查包含的头文件
    echo 检查main.c中包含的头文件：
    findstr /C:"#include" "main\main.c"
) else (
    echo ✗ 主程序文件不存在
)

if exist "main\CMakeLists.txt" (
    echo ✓ main组件CMakeLists.txt存在
) else (
    echo ✗ main组件CMakeLists.txt不存在
)

echo.
echo 步骤7：快速修复建议...
echo 根据检查结果，建议执行以下操作：
echo.

if not exist "build\compile_commands.json" (
    echo 1. 重新配置项目：
    echo    setup_env.bat
    echo    然后在新窗口中运行: idf.py reconfigure
    echo.
)

if not exist ".vscode\c_cpp_properties.json" (
    echo 2. 修复VS Code配置：
    echo    fix_vscode.bat
    echo.
)

if not exist "managed_components\espressif__esp32-camera" (
    echo 3. 下载缺失组件：
    echo    setup_env.bat
    echo    然后在新窗口中运行: idf.py reconfigure
    echo.
)

echo 4. 如果问题仍然存在：
echo    - 重新加载VS Code窗口 (Ctrl+Shift+P -> Developer: Reload Window)
echo    - 重置IntelliSense数据库 (Ctrl+Shift+P -> C/C++: Reset IntelliSense Database)

:end
echo.
pause
