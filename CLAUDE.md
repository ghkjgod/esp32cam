# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-S3 camera project using ESP-IDF v5.5. The project implements a basic camera capture application that takes pictures periodically using the ESP32-CAM component.

## Build System

The project uses ESP-IDF's CMake build system. Key files:
- `CMakeLists.txt` - Main project configuration
- `main/CMakeLists.txt` - Main component configuration  
- `main/idf_component.yml` - Component dependencies (includes espressif/esp32-camera)

## Common Commands

### Environment Setup
```cmd
# Set up development environment (run this first)
setup_env.bat

# Verify configuration
config.bat verify
```

### Build Commands
```cmd
# Build the project
idf.py build

# Build with single thread (for memory constrained systems)
idf.py build -j1

# Full build test with diagnostics
build_test.bat
```

### Flash and Monitor
```cmd
# Flash firmware to device
idf.py flash

# Monitor serial output
idf.py monitor

# Flash and monitor in one command
idf.py flash monitor
```

### Configuration
```cmd
# Open configuration menu
idf.py menuconfig

# Reconfigure project
idf.py reconfigure
```

## Project Structure

- `main/main.c` - Main application with camera initialization and capture loop
- `main/main.h` - GPIO pin definitions for GOOUU ESP32-S3 board
- `managed_components/` - ESP-IDF managed components (esp32-camera driver)
- `build/` - Build output directory

## Hardware Configuration

The project is configured for GOOUU ESP32-S3 board with camera pins defined in `main/main.h:3-28`. The camera is configured for JPEG output at QVGA resolution.

## Development Tools

### VS Code Setup
```cmd
# Fix IntelliSense and header file detection
fix_vscode.bat

# Diagnose header file issues
diagnose_headers.bat
```

### Configuration Files
- `config.bat` - Paths for ESP-IDF, tools, and Python (edit this for your environment)
- `sdkconfig` - ESP-IDF project configuration

## Important Notes

- The project uses PSRAM for camera frame buffers (`fb_location=CAMERA_FB_IN_PSRAM`)
- Camera captures JPEG images every 5 seconds in the main loop
- Frame buffers are managed with `esp_camera_fb_get()` and `esp_camera_fb_return()`
- The project requires ESP-IDF v5.5 and Python 3.11+

## Troubleshooting

If experiencing build issues:
1. Run `build_test.bat` for comprehensive diagnostics
2. Check `config.bat` paths are correct for your environment
3. Run `fix_vscode.bat` for IDE issues
4. Ensure adequate memory (4GB+) for compilation