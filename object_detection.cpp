#include "esp_camera.h"
#include "fd_forward.h"

// AI-Thinker ESP32-CAM pin configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Face detection configuration
static mtmn_config_t mtmn_config = {0};

void setup() {
  Serial.begin(115200);
  Serial.println("\nInitializing ESP32-CAM Face Detection...");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // 320x240 for face detection
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera initialized.");

  // Initialize face detection model
  mtmn_config = mtmn_init_config();
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix) {
      Serial.println("dl_matrix3du_alloc failed");
      esp_camera_fb_return(fb);
      return;
  }

  if(!fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, image_matrix->item)){
      Serial.println("fmt2rgb888 failed");
      dl_matrix3du_free(image_matrix);
      esp_camera_fb_return(fb);
      return;
  }
  
  box_list_t *boxes = face_detect(image_matrix, &mtmn_config);

  if (boxes) {
    Serial.printf("FACE DETECTED! Found %d faces.\n", boxes->len);
    free(boxes->score);
    free(boxes->box);
    free(boxes->landmark);
    free(boxes);
  } else {
    Serial.println("No face detected.");
  }
  
  dl_matrix3du_free(image_matrix);
  esp_camera_fb_return(fb);

  delay(2000);
}