
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "main.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include <esp_timer.h>
#include "esp_heap_caps.h"

// Include our custom modules
#include "wifi_manager_wrapper.h"
#include "camera_manager.h"
#include "http_uploader.h"
#include "ble_scanner.h"

static const char *TAG = "espcam_main";
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

// Configuration constants
#define UPLOAD_URL "http://120.25.165.65:23186/api/upload"
#define BLE_TARGET_DEVICE "BLE_NL"
#define BLE_RSSI_THRESHOLD -80
#define BLE_TRIGGERED_UPLOAD_INTERVAL_MS 2000  // 2 seconds when BLE device detected

// Application state
typedef enum {
    APP_STATE_INIT,
    APP_STATE_WIFI_CONFIG,
    APP_STATE_WIFI_CONNECTED,
    APP_STATE_CAMERA_INIT,
    APP_STATE_BLE_INIT,
    APP_STATE_READY,
    APP_STATE_BLE_TRIGGERED,
    APP_STATE_ERROR
} app_state_t;

static app_state_t s_app_state = APP_STATE_INIT;
static bool s_wifi_connected = false;
static bool s_camera_ready = false;
static bool s_ble_ready = false;
static bool s_ble_device_detected = false;
static int64_t s_last_upload_time = 0;
static int64_t s_ble_detection_time = 0;

// BLE scan result callback
static void ble_scan_callback(ble_scan_result_t *result)
{
    if (result && result->found) {
        printf("BLE device detected: %s, RSSI: %d\n", result->device_name, result->rssi);
        s_ble_device_detected = true;
        s_ble_detection_time = esp_timer_get_time() / 1000;  // Convert to milliseconds

        // Switch to BLE triggered mode for faster uploads
        if (s_app_state == APP_STATE_READY) {
            s_app_state = APP_STATE_BLE_TRIGGERED;
            printf("Switching to BLE triggered mode - will upload every %d seconds\n",
                   BLE_TRIGGERED_UPLOAD_INTERVAL_MS / 1000);
        }
    }
}

// WiFi event callback
static void wifi_event_callback(wifi_manager_event_t event)
{
    switch (event) {
        case WIFI_MANAGER_EVENT_STA_CONNECTED:
            printf("WiFi connected successfully\n");
            s_wifi_connected = true;
            s_app_state = APP_STATE_WIFI_CONNECTED;

            // Display IP address
            char ip_str[16];
            if (wifi_manager_wrapper_get_ip(ip_str) == ESP_OK) {
                printf("IP Address: %s\n", ip_str);
            }
            break;
        case WIFI_MANAGER_EVENT_STA_DISCONNECTED:
            printf("WiFi disconnected\n");
            s_wifi_connected = false;
            s_app_state = APP_STATE_WIFI_CONFIG;
            break;
        case WIFI_MANAGER_EVENT_CONFIG_PORTAL_STARTED:
            printf("WiFi config portal started\n");
            printf("Connect to 'ESP32-Camera' AP and open http://192.168.4.1 to configure WiFi\n");
            s_app_state = APP_STATE_WIFI_CONFIG;
            break;
        default:
            break;
    }
}

// Function to take and upload a picture
static void take_and_upload_picture(void)
{
    // Take a picture
    camera_fb_t *fb = NULL;
    esp_err_t err = camera_manager_take_picture(&fb);
    if (err != ESP_OK) {
        printf("Failed to take picture: %s\n", esp_err_to_name(err));
        return;
    }

    // Generate filename with timestamp
    char filename[64];
    snprintf(filename, sizeof(filename), "capture_%lld.jpg", esp_timer_get_time() / 1000);

    // Upload the image
    http_upload_response_t response;
    err = http_uploader_upload_fb(fb, filename, &response);

    if (err == ESP_OK) {
        printf("Image uploaded successfully. Status: %d\n", response.status_code);
        if (response.response_len > 0) {
            printf("Server response: %s\n", response.response_data);
        }
    } else {
        printf("Failed to upload image: %s\n", esp_err_to_name(err));
    }

    // Return frame buffer
    camera_manager_return_fb(fb);
}

static esp_err_t init_modules(void)
{
    esp_err_t err;

    // Initialize WiFi manager
    printf("Initializing WiFi manager...\n");
    err = wifi_manager_wrapper_init(wifi_event_callback);
    if (err != ESP_OK) {
        printf("Failed to initialize WiFi manager: %s\n", esp_err_to_name(err));
        return err;
    }

    // Start WiFi manager
    err = wifi_manager_wrapper_start();
    if (err != ESP_OK) {
        printf("Failed to start WiFi manager: %s\n", esp_err_to_name(err));
        return err;
    }

    // Initialize HTTP uploader
    printf("Initializing HTTP uploader...\n");
    http_upload_config_t upload_config = {
        .timeout_ms = 15000,
    };
    strncpy(upload_config.url, UPLOAD_URL, sizeof(upload_config.url) - 1);
    strncpy(upload_config.user_agent, "ESP32-Camera/1.0", sizeof(upload_config.user_agent) - 1);

    err = http_uploader_init(&upload_config);
    if (err != ESP_OK) {
        printf("Failed to initialize HTTP uploader: %s\n", esp_err_to_name(err));
        return err;
    }

    // Initialize BLE scanner
    printf("Initializing BLE scanner...\n");
    err = ble_scanner_init(BLE_TARGET_DEVICE, BLE_RSSI_THRESHOLD, ble_scan_callback);
    if (err != ESP_OK) {
        printf("Failed to initialize BLE scanner: %s\n", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
void app_main(void)
{
    printf("ESP32 Camera with WiFi Manager and BLE Scanner starting...\n");

    // Initialize modules
    if (init_modules() != ESP_OK) {
        printf("Failed to initialize modules\n");
        s_app_state = APP_STATE_ERROR;
        return;
    }

    s_app_state = APP_STATE_WIFI_CONFIG;

    // Main application loop
    while (1) {
        switch (s_app_state) {
            case APP_STATE_WIFI_CONFIG:
                printf("Waiting for WiFi connection...\n");
                // WiFi manager is running, just wait
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                break;

            case APP_STATE_WIFI_CONNECTED:
                printf("WiFi connected, initializing camera...\n");

                // Initialize camera
                if (camera_manager_init() == ESP_OK) {
                    s_camera_ready = true;
                    s_app_state = APP_STATE_CAMERA_INIT;
                    printf("Camera initialized successfully\n");
                } else {
                    printf("Failed to initialize camera\n");
                    s_app_state = APP_STATE_ERROR;
                }
                break;

            case APP_STATE_CAMERA_INIT:
                printf("Camera ready, initializing BLE scanner...\n");
                s_app_state = APP_STATE_BLE_INIT;
                break;

            case APP_STATE_BLE_INIT:
                printf("Starting BLE scanner...\n");

                // Start BLE scanning (continuous scan)
                if (ble_scanner_start(0) == ESP_OK) {
                    s_ble_ready = true;
                    s_app_state = APP_STATE_READY;
                    s_last_upload_time = esp_timer_get_time() / 1000;  // Initialize upload timer
                    printf("System ready - BLE scanner active, looking for '%s' devices\n", BLE_TARGET_DEVICE);
                    printf("Normal mode: no automatic uploads, only when BLE device detected\n");
                    printf("BLE triggered mode: will upload images every %d seconds when device detected\n",
                           BLE_TRIGGERED_UPLOAD_INTERVAL_MS / 1000);
                } else {
                    printf("Failed to start BLE scanner\n");
                    s_app_state = APP_STATE_ERROR;
                }
                break;

            case APP_STATE_READY:
                // System is fully operational - normal mode (no automatic uploads)
                // Only upload when BLE device is detected

                // Check if WiFi is still connected
                if (!wifi_manager_wrapper_is_connected()) {
                    printf("WiFi disconnected, returning to config mode\n");
                    s_app_state = APP_STATE_WIFI_CONFIG;
                    s_wifi_connected = false;
                }

                vTaskDelay(1000 / portTICK_PERIOD_MS);  // Check every 1 second
                break;

            case APP_STATE_BLE_TRIGGERED: {
                // BLE device detected - faster upload mode
                int64_t current_time = esp_timer_get_time() / 1000;  // Convert to milliseconds

                // Upload images more frequently when BLE device is detected
                if (current_time - s_last_upload_time >= BLE_TRIGGERED_UPLOAD_INTERVAL_MS) {
                    printf("Time to upload image (BLE triggered mode, interval: %d ms)\n",
                           BLE_TRIGGERED_UPLOAD_INTERVAL_MS);
                    take_and_upload_picture();
                    s_last_upload_time = current_time;
                }

                // Check if we should return to normal mode (no BLE device detected for 10 seconds)
                if (current_time - s_ble_detection_time > 10000) {
                    printf("No BLE device detected for 10 seconds, returning to normal mode\n");
                    s_ble_device_detected = false;
                    s_app_state = APP_STATE_READY;
                }

                // Check if WiFi is still connected
                if (!wifi_manager_wrapper_is_connected()) {
                    printf("WiFi disconnected, returning to config mode\n");
                    s_app_state = APP_STATE_WIFI_CONFIG;
                    s_wifi_connected = false;
                }

                vTaskDelay(500 / portTICK_PERIOD_MS);  // Check more frequently in BLE mode
                break;
            }

            case APP_STATE_ERROR:
                printf("Application in error state\n");
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                break;

            default:
                printf("Unknown application state: %d\n", s_app_state);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
        }
    }
}