@echo off
REM ESP32 Project Configuration File
REM Modify these paths according to your installation

set IDF_PATH=d:\workspace\esp\v5.5\esp-idf
set IDF_TOOLS_PATH=d:\workspace\idftools
set PYTHON_PATH=d:\workspace\idftools\tools\idf-python\3.11.2\python.exe

if "%1"=="verify" (
    echo Verifying configuration...
    echo IDF_PATH=%IDF_PATH%
    echo IDF_TOOLS_PATH=%IDF_TOOLS_PATH%
    echo PYTHON_PATH=%PYTHON_PATH%
    echo.
    
    if exist "%IDF_PATH%" (echo [OK] ESP-IDF path exists) else (echo [ERROR] ESP-IDF path not found)
    if exist "%IDF_TOOLS_PATH%" (echo [OK] Tools path exists) else (echo [ERROR] Tools path not found)
    if exist "%PYTHON_PATH%" (echo [OK] Python path exists) else (echo [ERROR] Python path not found)
)
