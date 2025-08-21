@echo off
REM ESP32开发环境配置文件
REM 请根据您的实际安装路径修改以下变量

REM ESP-IDF安装路径 (请修改为您的实际路径)
set IDF_PATH=d:\workspace\esp\v5.5\esp-idf

REM ESP-IDF工具路径 (请修改为您的实际路径)
set IDF_TOOLS_PATH=d:\workspace\idftools

REM Python环境路径
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python_env\idf5.5_py3.11_env

REM 验证配置
if "%1"=="verify" (
    echo 验证ESP-IDF配置...
    echo.
    echo IDF_PATH: %IDF_PATH%
    if exist "%IDF_PATH%" (
        echo ✓ ESP-IDF路径存在
    ) else (
        echo ✗ ESP-IDF路径不存在: %IDF_PATH%
        echo 请修改config.bat中的IDF_PATH设置
    )
    
    echo.
    echo IDF_TOOLS_PATH: %IDF_TOOLS_PATH%
    if exist "%IDF_TOOLS_PATH%" (
        echo ✓ ESP-IDF工具路径存在
    ) else (
        echo ✗ ESP-IDF工具路径不存在: %IDF_TOOLS_PATH%
        echo 请修改config.bat中的IDF_TOOLS_PATH设置
    )
    
    echo.
    echo Python环境: %IDF_PYTHON_ENV_PATH%
    if exist "%IDF_PYTHON_ENV_PATH%" (
        echo ✓ Python环境存在
    ) else (
        echo ✗ Python环境不存在: %IDF_PYTHON_ENV_PATH%
        echo 请检查ESP-IDF安装是否完整
    )
    
    echo.
    if exist "%IDF_PATH%\export.bat" (
        echo ✓ export.bat文件存在
    ) else (
        echo ✗ export.bat文件不存在
        echo 请检查ESP-IDF安装是否完整
    )
)
