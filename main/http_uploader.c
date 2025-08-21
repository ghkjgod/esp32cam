#include "http_uploader.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include <string.h>
#include <stdio.h>
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

    printf("Uploading image: %s, size: %zu bytes\n", filename, image_size);

    // Reset response buffer
    memset(s_response_buffer, 0, sizeof(s_response_buffer));
    s_response_len = 0;

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
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;

    // Create multipart form data boundary
    const char *boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    char content_type[128];
    snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s", boundary);

    // Calculate content length
    char header_part[512];
    char footer_part[128];
    
    snprintf(header_part, sizeof(header_part),
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
             "Content-Type: image/jpeg\r\n\r\n",
             boundary, filename);
    
    snprintf(footer_part, sizeof(footer_part), "\r\n--%s--\r\n", boundary);
    
    size_t total_len = strlen(header_part) + image_size + strlen(footer_part);

    // Set HTTP method and headers
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", content_type);
    esp_http_client_set_post_field(client, NULL, total_len);

    // Open connection
    err = esp_http_client_open(client, total_len);
    if (err != ESP_OK) {
        printf("Failed to open HTTP connection: %s\n", esp_err_to_name(err));
        goto cleanup;
    }

    // Send header part
    int written = esp_http_client_write(client, header_part, strlen(header_part));
    if (written < 0) {
        printf("Failed to write header part\n");
        err = ESP_FAIL;
        goto cleanup;
    }

    // Send image data
    written = esp_http_client_write(client, (const char *)image_data, image_size);
    if (written < 0) {
        printf("Failed to write image data\n");
        err = ESP_FAIL;
        goto cleanup;
    }

    // Send footer part
    written = esp_http_client_write(client, footer_part, strlen(footer_part));
    if (written < 0) {
        printf("Failed to write footer part\n");
        err = ESP_FAIL;
        goto cleanup;
    }

    // Fetch response
    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);

    printf("HTTP Status: %d, Content-Length: %d\n", status_code, content_length);

    if (content_length > 0) {
        int data_read = esp_http_client_read_response(client, s_response_buffer, 
                                                     sizeof(s_response_buffer) - 1);
        if (data_read >= 0) {
            s_response_buffer[data_read] = '\0';
            s_response_len = data_read;
        }
    }

    // Fill response structure
    if (response != NULL) {
        response->status_code = status_code;
        response->response_len = s_response_len;
        memcpy(response->response_data, s_response_buffer, 
               MIN(s_response_len + 1, sizeof(response->response_data)));
    }

    if (status_code >= 200 && status_code < 300) {
        printf("Image uploaded successfully\n");
        err = ESP_OK;
    } else {
        printf("Upload failed with status: %d\n", status_code);
        err = ESP_FAIL;
    }

cleanup:
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

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