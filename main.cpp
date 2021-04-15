#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WS2812FX.h>

#define LED_PIN 5

WS2812FX pixels = WS2812FX(3, LED_PIN, NEO_RGBW + NEO_KHZ800);

void red() {
    pixels.clear();
    pixels.setPixelColor(2, pixels.Color(0, 255, 0, 0));
    pixels.show();
}

void green() {
    pixels.clear();
    pixels.setPixelColor(2, pixels.Color(255, 0, 0, 0));
    pixels.show();
}

void sleep() {
  Serial.println("sleep");
  Serial.flush();
  ESP.deepSleep(5e6);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while(!Serial) { }
  Serial.println("setup...");
  pixels.init();
}

void loop() {
  Serial.print("loop: ");
  green();
  delay(5000);
  red();
  sleep();
}