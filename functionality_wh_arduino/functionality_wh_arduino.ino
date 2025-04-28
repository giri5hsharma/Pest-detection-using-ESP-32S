#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions
const int ledPin = 2;
const int irSensorPin = 4;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(irSensorPin, INPUT);

  Serial.begin(9600);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("System Ready");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void loop() {
  int sensorState = digitalRead(irSensorPin); // LOW = object detected

  if (sensorState == LOW) {
    digitalWrite(ledPin, HIGH);

    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Object Detected!");
    display.display();

    Serial.println("Object Detected");
  } else {
    digitalWrite(ledPin, LOW);

    display.clearDisplay();
    display.display(); // Clearing the screen
    Serial.println("No Object");
  }

  delay(200);
}
