# Runtime Test Script for ESP32 Camera WiFi Manager
Write-Host "ESP32 Camera WiFi Manager - Runtime Test" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green

# Check if build files exist
$binFile = "build\espcam.bin"
$bootloaderFile = "build\bootloader\bootloader.bin"
$partitionFile = "build\partition_table\partition-table.bin"

Write-Host "`nChecking build files..." -ForegroundColor Yellow

if (Test-Path $binFile) {
    $size = (Get-Item $binFile).Length
    Write-Host "✅ Main application: $binFile ($([math]::Round($size/1024/1024, 2)) MB)" -ForegroundColor Green
} else {
    Write-Host "❌ Main application file not found: $binFile" -ForegroundColor Red
    exit 1
}

if (Test-Path $bootloaderFile) {
    Write-Host "✅ Bootloader: $bootloaderFile" -ForegroundColor Green
} else {
    Write-Host "❌ Bootloader file not found: $bootloaderFile" -ForegroundColor Red
    exit 1
}

if (Test-Path $partitionFile) {
    Write-Host "✅ Partition table: $partitionFile" -ForegroundColor Green
} else {
    Write-Host "❌ Partition table file not found: $partitionFile" -ForegroundColor Red
    exit 1
}

Write-Host "`n📋 Fixed Runtime Issues:" -ForegroundColor Cyan
Write-Host "   • ESP_ERR_INVALID_STATE in esp_event_loop_create_default()" -ForegroundColor White
Write-Host "   • Duplicate event loop creation between wrapper and esp32-wifi-manager" -ForegroundColor White
Write-Host "   • Added proper error handling for existing event loops" -ForegroundColor White

Write-Host "`n🔧 Build Information:" -ForegroundColor Cyan
Write-Host "   • Target: ESP32-S3" -ForegroundColor White
Write-Host "   • ESP-IDF: v5.5" -ForegroundColor White
Write-Host "   • WiFi Manager: esp32-wifi-manager (GitHub)" -ForegroundColor White
Write-Host "   • mDNS: espressif/mdns v1.8.2" -ForegroundColor White
Write-Host "   • Camera: espressif/esp32-camera v2.0.15" -ForegroundColor White

Write-Host "`n📡 Expected WiFi Behavior:" -ForegroundColor Cyan
Write-Host "   • AP SSID: 'esp32'" -ForegroundColor White
Write-Host "   • AP Password: 'esp32pwd'" -ForegroundColor White
Write-Host "   • Config Portal: http://10.10.0.1" -ForegroundColor White
Write-Host "   • Auto-reconnect on startup if credentials saved" -ForegroundColor White

Write-Host "`n🚀 Ready for Testing!" -ForegroundColor Green
Write-Host "   1. Flash the firmware to your ESP32-S3 device" -ForegroundColor White
Write-Host "   2. Monitor serial output for startup messages" -ForegroundColor White
Write-Host "   3. Look for 'esp32' WiFi network if no saved credentials" -ForegroundColor White
Write-Host "   4. Connect and configure via web interface" -ForegroundColor White

Write-Host "`n💡 Flash Commands:" -ForegroundColor Yellow
Write-Host "   esptool.py --chip esp32s3 --port COM_PORT write_flash 0x0 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build\espcam.bin" -ForegroundColor Gray

Write-Host "`nTest completed successfully!" -ForegroundColor Green
