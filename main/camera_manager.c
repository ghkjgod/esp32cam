#include "camera_manager.h"
#include "main.h"
#include "esp_log.h"
#include "esp_camera.h"

static const char *TAG = "camera_manager";

static bool s_camera_initialized = false;

// Default camera configuration based on main.h
static camera_config_t s_camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .fb_location = CAMERA_FB_IN_PSRAM,
};

esp_err_t camera_manager_init(void)
{
    if (s_camera_initialized) {
        printf("Camera already initialized\n");
        return ESP_OK;
    }

    printf("Initializing camera...\n");

    // Initialize the camera
    esp_err_t err = esp_camera_init(&s_camera_config);
    if (err != ESP_OK) {
        printf("Camera init failed with error 0x%x\n", err);
        return err;
    }

    s_camera_initialized = true;
    printf("Camera initialized successfully\n");

    return ESP_OK;
}

esp_err_t camera_manager_init_with_config(camera_manager_config_t *config)
{
    if (s_camera_initialized) {
        printf("Camera already initialized\n");
        return ESP_OK;
    }

    if (config == NULL) {
        printf("Invalid config parameter\n");
        return ESP_ERR_INVALID_ARG;
    }

    printf("Initializing camera with custom config...\n");

    // Update configuration
    s_camera_config.pixel_format = config->pixel_format;
    s_camera_config.frame_size = config->frame_size;
    s_camera_config.jpeg_quality = config->jpeg_quality;
    s_camera_config.fb_count = config->fb_count;

    // Initialize the camera
    esp_err_t err = esp_camera_init(&s_camera_config);
    if (err != ESP_OK) {
        printf("Camera init failed with error 0x%x\n", err);
        return err;
    }

    s_camera_initialized = true;
    printf("Camera initialized successfully with custom config\n");

    return ESP_OK;
}

esp_err_t camera_manager_take_picture(camera_fb_t **fb)
{
    if (!s_camera_initialized) {
        printf("Camera not initialized\n");
        return ESP_ERR_INVALID_STATE;
    }

    if (fb == NULL) {
        printf("Invalid frame buffer pointer\n");
        return ESP_ERR_INVALID_ARG;
    }

    printf("Taking picture...\n");

    *fb = esp_camera_fb_get();
    if (*fb == NULL) {
        printf("Camera capture failed\n");
        return ESP_FAIL;
    }

    printf("Picture taken successfully, size: %zu bytes\n", (*fb)->len);

    return ESP_OK;
}

void camera_manager_return_fb(camera_fb_t *fb)
{
    if (fb == NULL) {
        printf("Trying to return NULL frame buffer\n");
        return;
    }

    esp_camera_fb_return(fb);
    // ESP_LOGD removed - too verbose for printf
}

bool camera_manager_is_initialized(void)
{
    return s_camera_initialized;
}

esp_err_t camera_manager_deinit(void)
{
    if (!s_camera_initialized) {
        printf("Camera not initialized\n");
        return ESP_OK;
    }

    printf("Deinitializing camera...\n");

    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        printf("Camera deinit failed with error 0x%x\n", err);
        return err;
    }

    s_camera_initialized = false;
    printf("Camera deinitialized successfully\n");

    return ESP_OK;
}
