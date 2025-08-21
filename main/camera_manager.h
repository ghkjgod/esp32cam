#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Camera manager configuration
 */
typedef struct {
    pixformat_t pixel_format;
    framesize_t frame_size;
    uint8_t jpeg_quality;
    uint8_t fb_count;
} camera_manager_config_t;

/**
 * @brief Initialize camera manager with default configuration
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t camera_manager_init(void);

/**
 * @brief Initialize camera manager with custom configuration
 * 
 * @param config Camera configuration
 * @return esp_err_t ESP_OK on success
 */
esp_err_t camera_manager_init_with_config(camera_manager_config_t *config);

/**
 * @brief Take a picture
 * 
 * @param fb Pointer to store the frame buffer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t camera_manager_take_picture(camera_fb_t **fb);

/**
 * @brief Return frame buffer
 * 
 * @param fb Frame buffer to return
 */
void camera_manager_return_fb(camera_fb_t *fb);

/**
 * @brief Check if camera is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool camera_manager_is_initialized(void);

/**
 * @brief Deinitialize camera manager
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t camera_manager_deinit(void);

#ifdef __cplusplus
}
#endif
