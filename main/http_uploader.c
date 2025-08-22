#include "http_uploader.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

static const char *TAG = "http_uploader";

static http_upload_config_t s_config = {0};
static bool s_initialized = false;

// HTTP response buffer
static char s_response_buffer[512];
static int s_response_len = 0;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_ON_CONNECTED:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_HEADER_SENT:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_ON_HEADER:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_ON_DATA:
            // ESP_LOGD removed - too verbose for printf
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Copy response data
                int copy_len = MIN(evt->data_len, sizeof(s_response_buffer) - s_response_len - 1);
                if (copy_len > 0) {
                    memcpy(s_response_buffer + s_response_len, evt->data, copy_len);
                    s_response_len += copy_len;
                    s_response_buffer[s_response_len] = '\0';
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_DISCONNECTED:
            // ESP_LOGD removed - too verbose for printf
            break;
        case HTTP_EVENT_REDIRECT:
            // ESP_LOGD removed - too verbose for printf
            break;
    }
    return ESP_OK;
}

esp_err_t http_uploader_init(http_upload_config_t *config)
{
    if (config == NULL) {
        printf("Invalid config parameter\n");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(config->url) == 0) {
        printf("URL cannot be empty\n");
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&s_config, config, sizeof(http_upload_config_t));

    // Set default values if not provided
    if (s_config.timeout_ms <= 0) {
        s_config.timeout_ms = 10000; // 10 seconds default
    }

    if (strlen(s_config.user_agent) == 0) {
        strcpy(s_config.user_agent, "ESP32-Camera/1.0");
    }

    s_initialized = true;
    printf("HTTP uploader initialized with URL: %s\n", s_config.url);

    return ESP_OK;
}

esp_err_t http_uploader_upload_image(const uint8_t *image_data, size_t image_size,
                                   const char *filename, http_upload_response_t *response)
{
    if (!s_initialized) {
        printf("HTTP uploader not initialized\n");
        return ESP_ERR_INVALID_STATE;
    }

    if (image_data == NULL || image_size == 0 || filename == NULL) {
        printf("Invalid parameters\n");
        return ESP_ERR_INVALID_ARG;
    }

    printf("Uploading image: %s, size: %zu bytes to URL: %s\n", filename, image_size, s_config.url);

    // Reset response buffer
    memset(s_response_buffer, 0, sizeof(s_response_buffer));
    s_response_len = 0;

    // Create multipart form data boundary
    const char *boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    char content_type[128];
    snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s", boundary);

    // Build multipart form data parts
    char header_part[512];
    char footer_part[128];

    snprintf(header_part, sizeof(header_part),
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
             "Content-Type: image/jpeg\r\n\r\n",
             boundary, filename);

    snprintf(footer_part, sizeof(footer_part), "\r\n--%s--\r\n", boundary);

    size_t header_len = strlen(header_part);
    size_t footer_len = strlen(footer_part);
    size_t total_len = header_len + image_size + footer_len;

    // Check for reasonable size limits (e.g., 2MB max)
    if (total_len > 2 * 1024 * 1024) {
        printf("Image too large for upload: %zu bytes (max 2MB)\n", total_len);
        return ESP_ERR_INVALID_SIZE;
    }

    // Check available memory
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    printf("Memory status before allocation:\n");
    printf("  - Free heap (default): %zu bytes\n", free_heap);
    printf("  - Free PSRAM: %zu bytes\n", free_psram);
    printf("  - Free internal: %zu bytes\n", free_internal);
    printf("  - Requested: %zu bytes\n", total_len);

    // Try to allocate from PSRAM first (for large allocations), then fallback to internal memory
    uint8_t *post_data = NULL;
    if (total_len > 16384) {  // Use PSRAM for large allocations
        post_data = heap_caps_malloc(total_len, MALLOC_CAP_SPIRAM);
        if (post_data != NULL) {
            printf("Successfully allocated %zu bytes in PSRAM\n", total_len);
        }
    }

    if (post_data == NULL) {
        printf("PSRAM allocation failed or not attempted, trying internal memory...\n");
        post_data = heap_caps_malloc(total_len, MALLOC_CAP_INTERNAL);
        if (post_data != NULL) {
            printf("Successfully allocated %zu bytes in internal RAM\n", total_len);
        }
    }

    if (post_data == NULL) {
        printf("Failed to allocate memory for POST data (%zu bytes)\n", total_len);
        printf("Available memory: heap=%zu, psram=%zu, internal=%zu\n",
               free_heap, free_psram, free_internal);
        return ESP_ERR_NO_MEM;
    }

    // Build complete multipart data
    memcpy(post_data, header_part, header_len);
    memcpy(post_data + header_len, image_data, image_size);
    memcpy(post_data + header_len + image_size, footer_part, footer_len);

    printf("Multipart data prepared: header_len=%zu, image_size=%zu, footer_len=%zu, total_len=%zu\n",
           header_len, image_size, footer_len, total_len);

    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = s_config.url,
        .event_handler = http_event_handler,
        .timeout_ms = s_config.timeout_ms,
        .user_agent = s_config.user_agent,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        printf("Failed to initialize HTTP client\n");
        heap_caps_free(post_data);
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;

    // Set HTTP method and headers
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", content_type);
    esp_http_client_set_post_field(client, (const char *)post_data, total_len);

    // Perform the request
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);

        printf("HTTP Status: %d, Content-Length: %d\n", status_code, content_length);

        // Fill response structure
        if (response != NULL) {
            response->status_code = status_code;
            response->response_len = s_response_len;
            memcpy(response->response_data, s_response_buffer,
                   MIN(s_response_len + 1, sizeof(response->response_data)));
        }

        if (status_code >= 200 && status_code < 300) {
            printf("Image uploaded successfully\n");
            if (s_response_len > 0) {
                printf("Server response: %s\n", s_response_buffer);
            }
            err = ESP_OK;
        } else {
            printf("Upload failed with status: %d\n", status_code);
            if (s_response_len > 0) {
                printf("Server error response: %s\n", s_response_buffer);
            }
            err = ESP_FAIL;
        }
    } else {
        printf("HTTP request failed: %s\n", esp_err_to_name(err));
    }

    // Cleanup
    esp_http_client_cleanup(client);
    heap_caps_free(post_data);

    return err;
}

esp_err_t http_uploader_upload_fb(camera_fb_t *fb, const char *filename,
                                http_upload_response_t *response)
{
    if (fb == NULL) {
        printf("Invalid frame buffer\n");
        return ESP_ERR_INVALID_ARG;
    }

    return http_uploader_upload_image(fb->buf, fb->len, filename, response);
}

bool http_uploader_is_initialized(void)
{
    return s_initialized;
}

esp_err_t http_uploader_deinit(void)
{
    if (!s_initialized) {
        printf("HTTP uploader not initialized\n");
        return ESP_OK;
    }

    memset(&s_config, 0, sizeof(s_config));
    s_initialized = false;

    printf("HTTP uploader deinitialized\n");
    return ESP_OK;
}