# WiFi配网功能测试指南

## 编译状态
✅ **编译成功** - 所有编译错误已解决：
- 修复了WiFi Manager头文件包含问题
- 解决了组件依赖问题（添加了esp32-wifi-manager组件）
- 修复了事件常量未定义问题
- 添加了mDNS组件支持（ESP-IDF 5.x要求）
- 修复了重复创建事件循环的问题（ESP_ERR_INVALID_STATE）

## 生成的文件
- `build/espcam.bin` - 主应用程序二进制文件 (1,344,688 bytes)
- `build/bootloader/bootloader.bin` - 引导加载程序
- 应用程序大小：1.3MB，剩余空间：13%

## WiFi配网功能测试步骤

### 1. 烧录固件
```bash
# 使用ESP-IDF工具烧录
idf.py flash

# 或使用esptool.py
esptool.py --chip esp32s3 --port COM_PORT write_flash 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/espcam.bin
```

### 2. 监控串口输出
```bash
idf.py monitor
```

### 3. 首次启动测试
当ESP32首次启动时（没有保存的WiFi凭据）：

**预期行为：**
- ESP32会启动AP模式
- AP名称：`esp32`
- AP密码：`esp32pwd`
- AP IP地址：`10.10.0.1`
- 串口会显示："WiFi config portal started"
- 串口会显示："Connect to 'esp32' AP and configure WiFi via web interface"

### 4. 连接到配置AP
1. 在手机或电脑上搜索WiFi网络
2. 连接到名为"esp32"的网络
3. 输入密码：`esp32pwd`

### 5. 访问配置界面
1. 连接成功后，浏览器应自动打开配置页面
2. 如果没有自动打开，手动访问：`http://10.10.0.1`
3. 应该看到WiFi配置界面，显示可用的WiFi网络列表

### 6. 配置WiFi连接
1. 在界面中选择要连接的WiFi网络
2. 输入WiFi密码
3. 点击"Connect"按钮
4. 观察串口输出

**预期行为：**
- ESP32会尝试连接到指定的WiFi网络
- 连接成功后，串口显示："WiFi connected successfully"
- 显示获取到的IP地址
- AP模式会在60秒后自动关闭（可配置）

### 7. 重启测试
1. 重启ESP32设备
2. 观察串口输出

**预期行为：**
- ESP32会自动尝试连接到之前保存的WiFi网络
- 如果连接成功，不会启动AP模式
- 如果连接失败，会重新启动AP模式进行配置

### 8. 应用程序功能测试
WiFi连接成功后，应用程序会：
1. 初始化摄像头
2. 启动BLE扫描器，寻找目标设备"car_1234"
3. 当发现目标设备时，拍照并上传到服务器

## 配置参数

### WiFi Manager配置
- **AP SSID**: esp32
- **AP密码**: esp32pwd
- **AP IP**: 10.10.0.1
- **最大重试次数**: 3次
- **重试间隔**: 5秒
- **AP关闭延时**: 60秒

### 应用程序配置
- **目标BLE设备**: car_1234
- **RSSI阈值**: -50 dBm
- **上传URL**: http://192.168.33.93:8000/upload
- **BLE扫描持续时间**: 30秒

## 故障排除

### 常见问题
1. **无法看到"esp32" AP**
   - 检查WiFi频道设置（默认频道1）
   - 确认设备已正确启动

2. **无法访问配置页面**
   - 确认已连接到"esp32" AP
   - 手动访问 http://10.10.0.1
   - 检查设备防火墙设置

3. **WiFi连接失败**
   - 检查WiFi密码是否正确
   - 确认WiFi网络可用
   - 查看串口输出的错误信息

4. **配置后无法连接**
   - 重启设备重试
   - 清除保存的配置（需要代码支持）
   - 检查WiFi网络是否稳定

### 运行时错误修复
**已修复的运行时错误：**
- **ESP_ERR_INVALID_STATE (0x103)** 在 `esp_event_loop_create_default()`
  - **原因**: wrapper和esp32-wifi-manager都尝试创建默认事件循环
  - **解决方案**: 重构wrapper代码，只处理NVS初始化，让esp32-wifi-manager处理所有网络初始化
  - **位置**: `main/wifi_manager.c` 第52-69行
  - **修复方法**: 移除了wrapper中的`esp_netif_init()`和`esp_event_loop_create_default()`调用

### 调试信息
串口输出会显示详细的调试信息，包括：
- WiFi连接状态
- IP地址分配
- 错误代码和描述
- 应用程序状态转换

## 下一步测试
1. 测试WiFi断线重连功能
2. 测试多次配置场景
3. 测试不同WiFi网络的兼容性
4. 验证摄像头和BLE功能
5. 测试HTTP上传功能
