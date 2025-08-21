#pragma once

#include "esp_err.h"
#include "esp_event.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi manager wrapper events
 */
typedef enum {
    WIFI_MANAGER_EVENT_STA_CONNECTED,
    WIFI_MANAGER_EVENT_STA_DISCONNECTED,
    WIFI_MANAGER_EVENT_AP_STARTED,
    WIFI_MANAGER_EVENT_CONFIG_PORTAL_STARTED,
    WIFI_MANAGER_EVENT_CONFIG_PORTAL_STOPPED
} wifi_manager_event_t;

/**
 * @brief WiFi manager callback function type
 */
typedef void (*wifi_manager_callback_t)(wifi_manager_event_t event);

/**
 * @brief Initialize WiFi manager with esp32-wifi-manager
 *
 * @param callback Callback function for WiFi events
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_manager_wrapper_init(wifi_manager_callback_t callback);

/**
 * @brief Start WiFi manager (will start captive portal if no saved credentials)
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_manager_wrapper_start(void);

/**
 * @brief Check if WiFi is connected
 *
 * @return true if connected, false otherwise
 */
bool wifi_manager_wrapper_is_connected(void);

/**
 * @brief Stop WiFi manager
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_manager_wrapper_stop(void);

/**
 * @brief Get the IP address if connected
 *
 * @param ip_str Buffer to store IP string (minimum 16 bytes)
 * @return esp_err_t ESP_OK if connected and IP retrieved
 */
esp_err_t wifi_manager_wrapper_get_ip(char *ip_str);

#ifdef __cplusplus
}
#endif
