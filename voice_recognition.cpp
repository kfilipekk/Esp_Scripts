#include <Arduino.h>
#include <math.h>
#include "driver/i2s.h"

#define I2S_WS_PIN   15
#define I2S_SCK_PIN  14
#define I2S_SD_PIN   32
#define I2S_PORT I2S_NUM_0

const int i2s_buffer_size = 1024;
int16_t i2s_read_buffer[i2s_buffer_size];

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up I2S microphone...");

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

  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  
  Serial.println("I2S driver installed.");
}

void loop() {
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(I2S_PORT, &i2s_read_buffer, sizeof(i2s_read_buffer), &bytes_read, portMAX_DELAY);

  if (result == ESP_OK && bytes_read > 0) {
    int32_t sum_sq = 0;
    for (int i = 0; i < bytes_read / sizeof(int16_t); i++) {
      sum_sq += i2s_read_buffer[i] * i2s_read_buffer[i];
    }
    float rms = sqrt(sum_sq / (bytes_read / sizeof(int16_t)));

    Serial.printf("RMS: %.2f\n", rms);
  }
}