#include <Arduino.h>
#include "ota.h"

#define SLEEP_TIME 3 * 60e6

void sleep(unsigned long sleeptime = SLEEP_TIME) {
    Serial.println("sleep");
    Serial.flush();
    ESP.deepSleep(SLEEP_TIME);
    delay(10);
}

void setup() {
    Serial.begin(115200);
    setupWifimanager(false);
    checkForUpdate();
    Serial.print("apiKey in setup: ");
    Serial.println(apiKey);
    sleep(1 * 60e6);
}

void loop() {
}