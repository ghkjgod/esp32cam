idf_component_register(SRCS "main.c"
                           "wifi_manager.c"
                           "ble_scanner.c"
                           "camera_manager.c"
                           "http_uploader.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_http_client esp_wifi esp_netif nvs_flash esp_timer bt esp32-wifi-manager)