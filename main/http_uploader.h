#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP upload configuration
 */
typedef struct {
    char url[256];
    int timeout_ms;
    char user_agent[64];
} http_upload_config_t;

/**
 * @brief HTTP upload response
 */
typedef struct {
    int status_code;
    char response_data[512];
    size_t response_len;
} http_upload_response_t;

/**
 * @brief Initialize HTTP uploader
 * 
 * @param config Upload configuration
 * @return esp_err_t ESP_OK on success
 */
esp_err_t http_uploader_init(http_upload_config_t *config);

/**
 * @brief Upload image data via HTTP POST
 * 
 * @param image_data Image data buffer
 * @param image_size Image data size
 * @param filename Filename for the upload
 * @param response Response structure to store result
 * @return esp_err_t ESP_OK on success
 */
esp_err_t http_uploader_upload_image(const uint8_t *image_data, size_t image_size, 
                                   const char *filename, http_upload_response_t *response);

/**
 * @brief Upload camera frame buffer via HTTP POST
 * 
 * @param fb Camera frame buffer
 * @param filename Filename for the upload
 * @param response Response structure to store result
 * @return esp_err_t ESP_OK on success
 */
esp_err_t http_uploader_upload_fb(camera_fb_t *fb, const char *filename, 
                                http_upload_response_t *response);

/**
 * @brief Check if HTTP uploader is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool http_uploader_is_initialized(void);

/**
 * @brief Deinitialize HTTP uploader
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t http_uploader_deinit(void);

#ifdef __cplusplus
}
#endif
