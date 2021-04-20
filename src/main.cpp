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
    setupWifimanager(true);
    checkForUpdate();
    Serial.print("api-key: ");
    Serial.print(apiKey);
    sleep(1 * 60e6);
}

void loop() {
}