#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_system.h"
#include <math.h>
#include <stdlib.h>

#define I2S_WS_PIN   15
#define I2S_SCK_PIN  14
#define I2S_SD_PIN   32
#define I2S_PORT     I2S_NUM_0

static const char *TAG = "VOICE_RECOGNITION";

const int i2s_buffer_size = 1024;

void voice_recognition_task(void *pvParameters)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT, &pin_config));
    ESP_LOGI(TAG, "I2S driver installed.");

    int16_t* i2s_read_buffer = (int16_t*) malloc(i2s_buffer_size * sizeof(int16_t));

    while (1) {
        size_t bytes_read = 0;
        esp_err_t result = i2s_read(I2S_PORT, i2s_read_buffer, i2s_buffer_size * sizeof(int16_t), &bytes_read, portMAX_DELAY);

        if (result == ESP_OK && bytes_read > 0) {
            int32_t sum_sq = 0;
            for (int i = 0; i < bytes_read / sizeof(int16_t); i++) {
                sum_sq += i2s_read_buffer[i] * i2s_read_buffer[i];
            }
            float rms = sqrt(sum_sq / (bytes_read / sizeof(int16_t)));
            ESP_LOGI(TAG, "RMS: %.2f", rms);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    free(i2s_read_buffer);
}

extern "C" void app_main_voice_recognition(void)
{
    xTaskCreate(&voice_recognition_task, "voice_recognition_task", 4096, NULL, 5, NULL);
}
