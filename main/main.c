
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

// Include our custom modules
#include "wifi_manager_wrapper.h"
#include "ble_scanner.h"
#include "camera_manager.h"
#include "http_uploader.h"

static const char *TAG = "espcam_main";
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

// Configuration constants
#define TARGET_DEVICE_NAME "car_1234"
#define RSSI_THRESHOLD -50  // dBm, closer devices have higher RSSI
#define UPLOAD_URL "http://192.168.1.21:10880/api/upload"
#define BLE_SCAN_DURATION 30  // seconds

// Application state
typedef enum {
    APP_STATE_INIT,
    APP_STATE_WIFI_CONFIG,
    APP_STATE_WIFI_CONNECTED,
    APP_STATE_CAMERA_INIT,
    APP_STATE_BLE_SCANNING,
    APP_STATE_READY,
    APP_STATE_ERROR
} app_state_t;

static app_state_t s_app_state = APP_STATE_INIT;
static bool s_wifi_connected = false;
static bool s_camera_ready = false;
static bool s_ble_ready = false;


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

// BLE scan result callback
static void ble_scan_callback(ble_scan_result_t *result)
{
    if (result == NULL || !result->found) {
        return;
    }

    printf("Target device found: %s, RSSI: %d\n", result->device_name, result->rssi);

    // Take a picture and upload it
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
                printf("Camera ready, starting BLE scanner...\n");

                // Initialize and start BLE scanner
                if (ble_scanner_init(TARGET_DEVICE_NAME, RSSI_THRESHOLD, ble_scan_callback) == ESP_OK) {
                    if (ble_scanner_start(0) == ESP_OK) {  // 0 = continuous scanning
                        s_ble_ready = true;
                        s_app_state = APP_STATE_BLE_SCANNING;
                        printf("BLE scanner started successfully\n");
                    } else {
                        printf("Failed to start BLE scanner\n");
                        s_app_state = APP_STATE_ERROR;
                    }
                } else {
                    printf("Failed to initialize BLE scanner\n");
                    s_app_state = APP_STATE_ERROR;
                }
                break;

            case APP_STATE_BLE_SCANNING:
                printf("System ready - scanning for target device: %s\n", TARGET_DEVICE_NAME);
                s_app_state = APP_STATE_READY;
                break;

            case APP_STATE_READY:
                // System is fully operational
                // BLE scanner is running in background and will trigger callbacks
                // when target device is found

                // Check if WiFi is still connected
                if (!wifi_manager_wrapper_is_connected()) {
                    printf("WiFi disconnected, returning to config mode\n");
                    s_app_state = APP_STATE_WIFI_CONFIG;
                    s_wifi_connected = false;

                    // Stop BLE scanner
                    if (s_ble_ready) {
                        ble_scanner_stop();
                        s_ble_ready = false;
                    }
                }

                vTaskDelay(10000 / portTICK_PERIOD_MS);  // Check every 10 seconds
                break;

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