#include "camera_manager.h"
#include "main.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"

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
    .frame_size = FRAMESIZE_SVGA,    //SVGA: 800x600, JPEG mode supports higher resolutions

    .jpeg_quality = 8, //0-63, for OV series camera sensors, lower number means higher quality (improved from 12 to 8)
    .fb_count = 2,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .fb_location = CAMERA_FB_IN_PSRAM,  //Using PSRAM for frame buffers (now enabled)
};

esp_err_t camera_manager_init(void)
{
    if (s_camera_initialized) {
        printf("Camera already initialized\n");
        return ESP_OK;
    }

    printf("Initializing camera...\n");
    printf("Camera config: frame_size=%d, fb_count=%d, fb_location=%d\n",
           s_camera_config.frame_size, s_camera_config.fb_count, s_camera_config.fb_location);

    // Check PSRAM availability and memory status
    printf("=== Memory Status Before Camera Init ===\n");
    printf("Total heap size: %d bytes\n", heap_caps_get_total_size(MALLOC_CAP_8BIT));
    printf("Free heap size: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("Largest free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    // Check PSRAM specifically
    printf("PSRAM total size: %d bytes\n", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    printf("PSRAM free size: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("PSRAM largest free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // Check internal RAM
    printf("Internal RAM total: %d bytes\n", heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
    printf("Internal RAM free: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    printf("Internal RAM largest free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    printf("========================================\n");

    // If PSRAM is not available or has insufficient memory, try using internal RAM
    if (heap_caps_get_free_size(MALLOC_CAP_SPIRAM) < 128000) {  // Need at least 128KB for SVGA camera buffers
        printf("WARNING: PSRAM has insufficient memory (%d bytes), switching to internal RAM\n",
               heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        s_camera_config.fb_location = CAMERA_FB_IN_DRAM;
        s_camera_config.fb_count = 1;  // Reduce buffer count to save memory
        printf("Updated config: fb_location=%d, fb_count=%d\n",
               s_camera_config.fb_location, s_camera_config.fb_count);
    }

    // Initialize the camera
    esp_err_t err = esp_camera_init(&s_camera_config);
    if (err != ESP_OK) {
        printf("Camera init failed with error 0x%x (%s)\n", err, esp_err_to_name(err));

        // If PSRAM init failed, try with internal RAM as fallback
        if (s_camera_config.fb_location == CAMERA_FB_IN_PSRAM) {
            printf("Retrying with internal RAM...\n");
            s_camera_config.fb_location = CAMERA_FB_IN_DRAM;
            s_camera_config.fb_count = 1;  // Use single buffer to save memory
            err = esp_camera_init(&s_camera_config);
            if (err != ESP_OK) {
                printf("Camera init failed again with internal RAM: 0x%x (%s)\n", err, esp_err_to_name(err));
                return err;
            }
        } else {
            return err;
        }
    }

    s_camera_initialized = true;
    printf("Camera initialized successfully\n");

    // Print final memory status
    printf("=== Memory Status After Camera Init ===\n");
    printf("Free heap size: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("PSRAM free size: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("Internal RAM free: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    printf("=======================================\n");

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
