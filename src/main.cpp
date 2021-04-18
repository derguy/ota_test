#include <CertStoreBearSSL.h>
#include <ESP_OTA_GitHub.h>
#include <FS.h>
#include <WiFiManager.h>
#include <version.h>
BearSSL::CertStore certStore;

#define SLEEP_TIME 3 * 60e6
#define WIFI_RESET_BUTTON 4

WiFiManagerParameter githubUser("Github user", "Github user", "", 40);
WiFiManagerParameter githubRepo("Github repo", "Github repo", "", 40);
WiFiManagerParameter githubFileName("Github filename", "Github filename", "firmware.bin", 40);

void sleep() {
  Serial.println("sleep");
  Serial.flush();
  ESP.deepSleep(SLEEP_TIME);
  delay(10);
}

void wifiResetOnButtonPressed() {
	// Reset if Button WIFI_RESET_BUTTON pressed on startup
	pinMode(WIFI_RESET_BUTTON, INPUT);
	digitalWrite(WIFI_RESET_BUTTON, HIGH);
	delay(10);
	int buttonState = buttonState = digitalRead(WIFI_RESET_BUTTON);
	if (buttonState == LOW) {
		Serial.println("WIFI_RESET_BUTTON pressed");
		WiFiManager wifiManager;
		wifiManager.resetSettings(); 
		Serial.println("WifiManager resetted...");
		// ESP.restart();
	}
}

void setupWifimanager() {
    WiFi.mode(WIFI_STA);
    WiFiManager wifiManager;
    
 	wifiManager.addParameter(&githubUser);
 	wifiManager.addParameter(&githubRepo);
 	wifiManager.addParameter(&githubFileName);

    bool res;
    res = wifiManager.autoConnect("esp", "111");
	
    if (!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } else {
        Serial.println("connected...");
    }
}

void checkForUpdate() {
    SPIFFS.begin();
    int numCerts = certStore.initCertStore(SPIFFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    Serial.print(F("Number of CA certs read: "));
    Serial.println(numCerts);
    if (numCerts == 0) {
        Serial.println(F("No certs found. Did you run certs-from-mozill.py and upload the SPIFFS directory before running?"));
        return;  // Can't connect to anything w/o certs!
    }

	Serial.print("Current GITHUB_RELEASE_VERSION: ");
    Serial.println(GITHUB_RELEASE_VERSION);
    Serial.println("Checking for update...");
	ESPOTAGitHub ESPOTAGitHub(&certStore, githubUser.getValue(), githubRepo.getValue(), GITHUB_RELEASE_VERSION, githubFileName.getValue(), 1 /* accept prerelease */);
    if (ESPOTAGitHub.checkUpgrade()) {
        Serial.print("Upgrade found at: ");
        Serial.println(ESPOTAGitHub.getUpgradeURL());
        if (ESPOTAGitHub.doUpgrade()) {
            Serial.println("Upgrade complete.");  //This should never be seen as the device should restart on successful upgrade.
        } else {
            Serial.print("Unable to upgrade: ");
            Serial.println(ESPOTAGitHub.getLastError());
        }
    } else {
        Serial.print("Not proceeding to upgrade: ");
        Serial.println(ESPOTAGitHub.getLastError());
    }
}

void setup() {
    Serial.begin(115200);
	wifiResetOnButtonPressed();
    setupWifimanager();
    checkForUpdate();
	sleep();
}

void loop() {
}