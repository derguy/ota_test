#include <OtaManager.h>

#define SLEEP_TIME 3 * 60e6

OtaManager otaManager;

void sleep(unsigned long sleeptime = SLEEP_TIME) {
    Serial.println("sleep");
    Serial.flush();
    ESP.deepSleep(SLEEP_TIME);
    delay(10);
}

void setup() {
    Serial.begin(115200);
    otaManager.setupWifimanager(true, 5);
    otaManager.checkForUpdate();
    Serial.print("apiKey in setup: ");
    Serial.println(otaManager.getApiKey());
    sleep(1 * 60e6);
}

void loop() {
}