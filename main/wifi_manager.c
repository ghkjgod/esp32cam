#include "wifi_manager_wrapper.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include <string.h>

// Include the esp32-wifi-manager library header
#include "wifi_manager.h"

static const char *TAG = "wifi_manager_wrapper";

static wifi_manager_callback_t s_callback = NULL;
static bool s_wifi_connected = false;

/**
 * @brief WiFi manager event callback for esp32-wifi-manager
 * This function is called by esp32-wifi-manager when WiFi events occur
 */
static void cb_connection_ok(void *pvParameter)
{
    ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI(TAG, "WiFi connected! IP: %s", str_ip);
    s_wifi_connected = true;

    if (s_callback) {
        s_callback(WIFI_MANAGER_EVENT_STA_CONNECTED);
    }
}

/**
 * @brief WiFi manager disconnection callback
 */
static void cb_connection_lost(void *pvParameter)
{
    ESP_LOGI(TAG, "WiFi connection lost");
    s_wifi_connected = false;

    if (s_callback) {
        s_callback(WIFI_MANAGER_EVENT_STA_DISCONNECTED);
    }
}

esp_err_t wifi_manager_wrapper_init(wifi_manager_callback_t callback)
{
    ESP_LOGI(TAG, "Initializing WiFi manager wrapper...");

    s_callback = callback;
    s_wifi_connected = false;

    // Only initialize NVS here, let esp32-wifi-manager handle the rest
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "WiFi manager wrapper initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_manager_wrapper_start(void)
{
    ESP_LOGI(TAG, "Starting esp32-wifi-manager...");

    // Start the esp32-wifi-manager
    // This will automatically:
    // 1. Try to connect to saved WiFi credentials
    // 2. If no credentials or connection fails, start captive portal
    // 3. Provide web interface for WiFi configuration
    wifi_manager_start();

    // Register callbacks for WiFi events
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);
    wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, &cb_connection_lost);

    if (s_callback) {
        s_callback(WIFI_MANAGER_EVENT_CONFIG_PORTAL_STARTED);
    }

    ESP_LOGI(TAG, "esp32-wifi-manager started. If no saved credentials, captive portal will be available.");
    ESP_LOGI(TAG, "Connect to 'esp32' AP and configure WiFi via web interface.");

    return ESP_OK;
}

bool wifi_manager_wrapper_is_connected(void)
{
    return s_wifi_connected;
}

esp_err_t wifi_manager_wrapper_get_ip(char *ip_str)
{
    if (!s_wifi_connected || ip_str == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Use esp32-wifi-manager's function to get IP
    char* sta_ip = wifi_manager_get_sta_ip_string();
    if (sta_ip != NULL) {
        strncpy(ip_str, sta_ip, 16);
        ip_str[15] = '\0'; // Ensure null termination
        return ESP_OK;
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t wifi_manager_wrapper_stop(void)
{
    ESP_LOGI(TAG, "Stopping WiFi manager");

    // Stop the esp32-wifi-manager
    wifi_manager_destroy();

    if (s_callback) {
        s_callback(WIFI_MANAGER_EVENT_CONFIG_PORTAL_STOPPED);
    }

    return ESP_OK;
}
