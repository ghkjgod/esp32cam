#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for BLE types
#define ESP_BD_ADDR_LEN 6
typedef uint8_t esp_bd_addr_t[ESP_BD_ADDR_LEN];

/**
 * @brief BLE scan result structure
 */
typedef struct {
    char device_name[32];
    esp_bd_addr_t bda;
    int rssi;
    bool found;
} ble_scan_result_t;

/**
 * @brief BLE scanner callback function type
 * 
 * @param result Scan result containing device information
 */
typedef void (*ble_scanner_callback_t)(ble_scan_result_t *result);

/**
 * @brief Initialize BLE scanner
 * 
 * @param target_device_name Target device name to search for (e.g., "car_1234")
 * @param rssi_threshold RSSI threshold for proximity detection (e.g., -50 dBm)
 * @param callback Callback function when target device is found
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ble_scanner_init(const char *target_device_name, int rssi_threshold, ble_scanner_callback_t callback);

/**
 * @brief Start BLE scanning
 * 
 * @param scan_duration_sec Scan duration in seconds (0 for continuous)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ble_scanner_start(uint32_t scan_duration_sec);

/**
 * @brief Stop BLE scanning
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ble_scanner_stop(void);

/**
 * @brief Check if BLE scanner is running
 * 
 * @return true if scanning, false otherwise
 */
bool ble_scanner_is_scanning(void);

/**
 * @brief Deinitialize BLE scanner
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ble_scanner_deinit(void);

#ifdef __cplusplus
}
#endif
