#include "ble_scanner.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <sys/param.h>

#ifdef CONFIG_BT_ENABLED
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "nvs_flash.h"
#endif


static char s_target_device_name[32] = {0};
static int s_rssi_threshold = -50;
static ble_scanner_callback_t s_callback = NULL;
static bool s_scanning = false;
static uint32_t s_scan_duration = 0;

#ifdef CONFIG_BT_ENABLED
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};
#endif

#ifdef CONFIG_BT_ENABLED
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            printf("BLE scan parameters set complete\n");
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
            if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                printf("BLE scan start failed, error status = %x\n", param->scan_start_cmpl.status);
                s_scanning = false;
            } else {
                printf("BLE scan started successfully\n");
                s_scanning = true;
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
            switch (scan_result->scan_rst.search_evt) {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    // Parse advertisement data to get device name
                    uint8_t *adv_name = NULL;
                    uint8_t adv_name_len = 0;
                    adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                       ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
                    
                    if (adv_name == NULL) {
                        adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                           ESP_BLE_AD_TYPE_NAME_SHORT, &adv_name_len);
                    }
                    
                    if (adv_name != NULL && adv_name_len > 0) {
                        char device_name[32] = {0};
                        memcpy(device_name, adv_name, MIN(adv_name_len, sizeof(device_name) - 1));
                        
                        printf("Found device: %s, RSSI: %d\n", device_name, scan_result->scan_rst.rssi);

                        // Check if this is our target device
                        if (strstr(device_name, s_target_device_name) != NULL) {
                            printf("Target device found: %s, RSSI: %d\n", device_name, scan_result->scan_rst.rssi);

                            // Check RSSI threshold
                            if (scan_result->scan_rst.rssi >= s_rssi_threshold) {
                                printf("Device is close enough (RSSI: %d >= %d)\n",
                                        scan_result->scan_rst.rssi, s_rssi_threshold);

                                // Prepare result
                                ble_scan_result_t result = {0};
                                strncpy(result.device_name, device_name, sizeof(result.device_name) - 1);
                                memcpy(result.bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
                                result.rssi = scan_result->scan_rst.rssi;
                                result.found = true;

                                // Call callback
                                if (s_callback) {
                                    s_callback(&result);
                                }
                            } else {
                                printf("Device too far (RSSI: %d < %d)\n",
                                        scan_result->scan_rst.rssi, s_rssi_threshold);
                            }
                        }
                    }
                    break;
                }
                case ESP_GAP_SEARCH_INQ_CMPL_EVT:
                    printf("BLE scan complete\n");
                    s_scanning = false;
                    break;
                default:
                    break;
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
            if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                printf("BLE scan stop failed, error status = %x\n", param->scan_stop_cmpl.status);
            } else {
                printf("BLE scan stopped successfully\n");
            }
            s_scanning = false;
            break;
        }
        default:
            break;
    }
}
#endif // CONFIG_BT_ENABLED

esp_err_t ble_scanner_init(const char *target_device_name, int rssi_threshold, ble_scanner_callback_t callback)
{
    if (target_device_name == NULL || callback == NULL) {
        printf("Invalid parameters\n");
        return ESP_ERR_INVALID_ARG;
    }

    strncpy(s_target_device_name, target_device_name, sizeof(s_target_device_name) - 1);
    s_rssi_threshold = rssi_threshold;
    s_callback = callback;
    s_scanning = false;

    printf("Initializing BLE scanner for device: %s, RSSI threshold: %d\n",
             s_target_device_name, s_rssi_threshold);

#ifdef CONFIG_BT_ENABLED
    printf("BLE is enabled in configuration - but temporarily disabled due to linking issues\n");
    printf("BLE scanner initialized successfully (simulation mode)\n");
    return ESP_OK;
#else
    printf("BLE not enabled in configuration - BLE scanner disabled\n");
    return ESP_OK;
#endif
}

esp_err_t ble_scanner_start(uint32_t scan_duration_sec)
{
    if (s_scanning) {
        printf("BLE scanner already running\n");
        return ESP_OK;
    }

    s_scan_duration = scan_duration_sec;

    printf("Starting BLE scan for %lu seconds\n", scan_duration_sec);

#ifdef CONFIG_BT_ENABLED
    printf("BLE scan started (simulation mode)\n");
    s_scanning = true;
    return ESP_OK;
#else
    printf("BLE not enabled - simulating scan start\n");
    s_scanning = true;
    return ESP_OK;
#endif
}

esp_err_t ble_scanner_stop(void)
{
    if (!s_scanning) {
        printf("BLE scanner not running\n");
        return ESP_OK;
    }

    printf("Stopping BLE scan\n");

#ifdef CONFIG_BT_ENABLED
    printf("BLE scan stopped (simulation mode)\n");
    s_scanning = false;
    return ESP_OK;
#else
    printf("BLE not enabled - simulating scan stop\n");
    s_scanning = false;
    return ESP_OK;
#endif
}

bool ble_scanner_is_scanning(void)
{
    return s_scanning;
}

esp_err_t ble_scanner_deinit(void)
{
    // Stop scanning if running
    if (s_scanning) {
        ble_scanner_stop();
    }

#ifdef CONFIG_BT_ENABLED
    esp_err_t ret;

    // Disable and deinitialize Bluedroid
    ret = esp_bluedroid_disable();
    if (ret) {
        printf("Disable bluedroid failed: %s\n", esp_err_to_name(ret));
    }

    ret = esp_bluedroid_deinit();
    if (ret) {
        printf("Deinitialize bluedroid failed: %s\n", esp_err_to_name(ret));
    }

    // Disable and deinitialize controller
    ret = esp_bt_controller_disable();
    if (ret) {
        printf("Disable controller failed: %s\n", esp_err_to_name(ret));
    }

    ret = esp_bt_controller_deinit();
    if (ret) {
        printf("Deinitialize controller failed: %s\n", esp_err_to_name(ret));
    }
#endif

    printf("BLE scanner deinitialized\n");
    return ESP_OK;
}
