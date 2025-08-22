# Flash and monitor script for ESP32-S3
Write-Host "ESP32-S3 Flash and Monitor Script"

# Set environment variables
$env:IDF_PATH = "d:\workspace\esp\v5.5\esp-idf"
$env:IDF_TOOLS_PATH = "d:\workspace\idftools"
$env:PYTHON = "d:\workspace\idftools\python_env\idf5.5_py3.11_env\Scripts\python.exe"
$env:PYTHONPATH = "d:\workspace\esp\v5.5\esp-idf\tools"

# Add tools to PATH
$env:PATH = "d:\workspace\idftools\tools\ninja\1.12.1;d:\workspace\idftools\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;$env:PATH"

# Check if COM port is provided
if ($args.Length -eq 0) {
    Write-Host "Usage: .\flash_and_monitor.ps1 <COM_PORT>"
    Write-Host "Example: .\flash_and_monitor.ps1 COM3"
    exit 1
}

$COM_PORT = $args[0]
Write-Host "Using COM port: $COM_PORT"

# Flash the device
Write-Host "Flashing device..."
$python_path = "d:\workspace\idftools\python_env\idf5.5_py3.11_env\Scripts\python.exe"
$esptool_path = "d:\workspace\esp\v5.5\esp-idf\components\esptool_py\esptool\esptool.py"

& $python_path $esptool_path --chip esp32s3 --port $COM_PORT --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 build\bootloader\bootloader.bin 0x10000 build\espcam.bin 0x8000 build\partition_table\partition-table.bin

if ($LASTEXITCODE -eq 0) {
    Write-Host "Flash successful! Starting monitor..."
    Write-Host "Press Ctrl+] to exit monitor"
    
    # Start monitor
    & $python_path $esptool_path --chip esp32s3 --port $COM_PORT --baud 115200 monitor
} else {
    Write-Host "Flash failed with exit code: $LASTEXITCODE"
}
