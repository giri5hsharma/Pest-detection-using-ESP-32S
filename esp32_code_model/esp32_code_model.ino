#include <esp_camera.h>
#include <TFLiteMicro/tensorflow/lite/micro/micro_interpreter.h>
#include <TFLiteMicro/tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <TFLiteMicro/tensorflow/lite/schema/schema_generated.h>

#include "pest_model.h" // the trained model

#define BUZZER_PIN 4
#define DETECTION_THRESHOLD 0.7

// Camera configuration
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_96X96; // Match  model input size
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (1);
  }
}

// TFLite setup
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 10 * 1024; // Adjust based on model size
uint8_t tensor_arena[kTensorArenaSize];

void setupTFLite() {
  model = tflite::GetModel(pest_detection_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch");
    while (1);
  }

  static tflite::MicroMutableOpResolver<5> resolver;
  resolver.AddAveragePool2D();
  resolver.AddConv2D();
  resolver.AddDepthwiseConv2D();
  resolver.AddFullyConnected();
  resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Tensor allocation failed");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("TFLite model initialized");
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  initCamera();
  setupTFLite();
}

void loop() {
  camera_fb_t *fb = esp_camera_frame_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  uint8_t* input_buffer = input->data.uint8;
  for (int i = 0; i < 96 * 96; i++) {
    input_buffer[i] = fb->buf[i * 2]; // Simple RGB565 to uint8 conversion
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Inference failed");
    esp_camera_fb_return(fb);
    return;
  }

  float pest_score = output->data.f[0]; // Assuming binary classification
  Serial.print("Pest score: ");
  Serial.println(pest_score);

  if (pest_score > DETECTION_THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Pest detected! Buzzer on");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("No pest detected");
  }

  esp_camera_fb_return(fb);
  delay(1000);
}