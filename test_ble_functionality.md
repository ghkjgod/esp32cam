# BLE功能测试指南

## 测试准备

1. **确保ESP32-S3开发板已连接**
2. **确认COM端口号**
3. **准备HTTP服务器接收图片上传**

## 测试步骤

### 1. 烧录固件
```bash
# 设置环境
.\setup_env.bat

# 烧录固件（替换COMx为实际端口）
idf.py -p COMx flash

# 监控串口输出
idf.py -p COMx monitor
```

### 2. 观察启动日志
应该看到以下关键日志：
```
ESP32 Camera with WiFi Manager and BLE Scanner starting...
Initializing BLE scanner for device: BLE_NL, RSSI threshold: -80
BLE scanner initialized successfully (simulation mode)
```

### 3. WiFi配置
- 如果是首次运行，设备会创建"ESP32-Camera" AP
- 连接到该AP，访问 http://192.168.4.1 配置WiFi
- 配置完成后设备会连接到指定WiFi网络

### 4. 观察正常模式
WiFi连接成功后，应该看到：
```
System ready - BLE scanner active, looking for 'BLE_NL' devices
Normal mode: will upload images every 5 seconds
Time to upload image (normal mode, interval: 5000 ms)
```

### 5. 观察BLE触发模式
大约30秒后，应该看到模拟的BLE设备检测：
```
Simulating BLE device detection: BLE_NL
BLE device detected: BLE_NL, RSSI: -75
Switching to BLE triggered mode - will upload every 2 seconds
Time to upload image (BLE triggered mode, interval: 2000 ms)
```

### 6. 观察模式切换
10秒后应该自动返回正常模式：
```
No BLE device detected for 10 seconds, returning to normal mode
Time to upload image (normal mode, interval: 5000 ms)
```

## 预期行为

### 正常模式（默认）
- 每5秒上传一张图片
- 日志显示 "normal mode, interval: 5000 ms"

### BLE触发模式
- 检测到BLE_NL设备时自动切换
- 每2秒上传一张图片
- 日志显示 "BLE triggered mode, interval: 2000 ms"
- 10秒无检测后自动返回正常模式

### 模拟行为
- 每30秒模拟检测一次BLE_NL设备
- 模拟RSSI值为-75dBm（满足>-80条件）
- 触发快速上传模式持续约10秒

## 故障排除

### 1. 编译失败
```bash
# 清理构建
idf.py clean

# 重新编译
idf.py build
```

### 2. 烧录失败
- 检查COM端口是否正确
- 确认ESP32-S3开发板连接正常
- 尝试按住BOOT按钮再烧录

### 3. WiFi连接失败
- 检查WiFi密码是否正确
- 确认WiFi网络可用
- 重新配置WiFi（长按BOOT按钮重置）

### 4. 图片上传失败
- 检查HTTP服务器是否运行在 http://192.168.3.68:10880/api/upload
- 确认网络连接正常
- 查看服务器日志

## 性能监控

### 内存使用
观察日志中的内存信息，确保没有内存泄漏

### 任务状态
BLE模拟任务应该正常运行，不应该出现任务崩溃

### 网络状态
WiFi连接应该保持稳定，断线时会自动重连

## 下一步测试

1. **真实BLE设备测试**：使用真实的BLE设备进行测试
2. **长时间运行测试**：24小时稳定性测试
3. **网络异常测试**：测试网络断线重连功能
4. **电源管理测试**：测试低功耗模式

## 测试结果记录

请记录以下测试结果：
- [ ] 编译成功
- [ ] 烧录成功
- [ ] WiFi连接成功
- [ ] 摄像头初始化成功
- [ ] BLE扫描器初始化成功
- [ ] 正常模式图片上传正常
- [ ] BLE触发模式切换正常
- [ ] 快速上传模式工作正常
- [ ] 自动返回正常模式正常
- [ ] 长时间运行稳定
