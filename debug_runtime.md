# 运行时错误调试指南

## 🔧 最新修复 (v2)

### 问题分析
之前的修复方法不够彻底，仍然存在重复初始化的问题：
- **Wrapper代码**: 在`wifi_manager_wrapper_init()`中初始化网络栈
- **esp32-wifi-manager**: 在`wifi_manager_start()`中也初始化网络栈

### 新的解决方案
完全重构了wrapper的初始化逻辑：

**之前的代码 (有问题):**
```c
esp_err_t wifi_manager_wrapper_init(wifi_manager_callback_t callback)
{
    // ... 
    ESP_ERROR_CHECK(esp_netif_init());                    // ❌ 重复初始化
    ESP_ERROR_CHECK(esp_event_loop_create_default());     // ❌ 重复初始化
    // ...
}
```

**修复后的代码:**
```c
esp_err_t wifi_manager_wrapper_init(wifi_manager_callback_t callback)
{
    s_callback = callback;
    s_wifi_connected = false;

    // 只初始化NVS，让esp32-wifi-manager处理网络初始化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    return ESP_OK;
}
```

## 🧪 测试新固件

### 1. 烧录新固件
```bash
esptool.py --chip esp32s3 --port COM_PORT write_flash \
  0x0 build/bootloader/bootloader.bin \
  0x8000 build/partition_table/partition-table.bin \
  0x10000 build/espcam.bin
```

### 2. 预期的正常启动日志
```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON_RESET),boot:0x8 (SPI_FAST_FLASH_BOOT)
...
I (29) boot: ESP-IDF v5.5 2nd stage bootloader
...
ESP32 Camera with WiFi Manager and BLE Scanner starting...
I (XXX) wifi_manager_wrapper: Initializing WiFi manager wrapper...
I (XXX) wifi_manager_wrapper: WiFi manager wrapper initialized successfully
I (XXX) wifi_manager_wrapper: Starting esp32-wifi-manager...
I (XXX) wifi_manager: wifi_manager_start
I (XXX) wifi_manager: Starting WiFi Manager
...
```

### 3. 如果仍然出现错误
如果您仍然看到以下错误：
```
ESP_ERROR_CHECK failed: esp_err_t 0x103 (ESP_ERR_INVALID_STATE) at 0x42051267
```

**可能的原因和解决方案：**

#### 原因1: 缓存的旧固件
- **解决方案**: 完全擦除flash
```bash
esptool.py --chip esp32s3 --port COM_PORT erase_flash
```
然后重新烧录固件。

#### 原因2: esp32-wifi-manager版本兼容性
- **解决方案**: 检查esp32-wifi-manager是否与ESP-IDF 5.5兼容
- 可能需要使用特定的分支或版本

#### 原因3: 其他组件的初始化冲突
- **解决方案**: 检查main.c中的初始化顺序
- 确保在调用`wifi_manager_wrapper_start()`之前没有其他网络相关的初始化

## 🔍 调试步骤

### 1. 检查编译时间戳
确保使用的是最新编译的固件：
```
I (29) boot: compile time Aug 21 2025 14:09:48  # 应该是最新时间
```

### 2. 检查组件版本
在编译输出中确认：
```
NOTICE: [2/3] espressif/mdns (1.8.2)
```

### 3. 监控完整启动过程
使用串口监控工具观察完整的启动过程，特别关注：
- NVS初始化
- WiFi Manager启动
- 事件循环创建

## 📋 修复历史

### v1 (第一次尝试)
- 在wrapper中添加了错误检查
- 问题：仍然有重复初始化

### v2 (当前版本)
- 完全重构wrapper初始化逻辑
- 只在wrapper中处理NVS初始化
- 让esp32-wifi-manager处理所有网络相关初始化

## 🎯 成功标志

如果修复成功，您应该看到：
1. ✅ 设备正常启动，无崩溃
2. ✅ WiFi Manager成功启动
3. ✅ 如果没有保存的WiFi凭据，会看到"esp32" AP
4. ✅ 可以通过Web界面配置WiFi

## 📞 如果问题持续存在

如果新固件仍然有问题，请提供：
1. 完整的串口启动日志
2. 确认使用的是最新编译的固件
3. ESP32-S3开发板的具体型号

这将帮助进一步诊断问题。
