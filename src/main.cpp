#include <CertStoreBearSSL.h>
#include <ESP_OTA_GitHub.h>
#include <FS.h>
#include <WiFiManager.h>
#include <version.h>
BearSSL::CertStore certStore;

WiFiManagerParameter thingspeakApiKey("Thingspeak Key", "Thingspeak Key", "", 40);
WiFiManagerParameter githubUser("Github user", "Github user", "derguy", 40);
WiFiManagerParameter githubRepo("Github repo", "Github repo", "ota_test", 40);
WiFiManagerParameter githubFileName("Github filename", "Github filename", "firmware.bin", 40);

void setupWifimanager() {
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    // wm.resetSettings(); //reset settings - wipe credentials for testing
 	wm.addParameter(&thingspeakApiKey);
 	wm.addParameter(&githubUser);
 	wm.addParameter(&githubRepo);
 	wm.addParameter(&githubFileName);

    bool res;
    res = wm.autoConnect("esp", "111");  // password protected ap
	
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
    setupWifimanager();
    checkForUpdate();
    Serial.print("Current GITHUB_RELEASE_VERSION: ");
    Serial.println(GITHUB_RELEASE_VERSION);
}

void loop() {
}