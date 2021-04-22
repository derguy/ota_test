#include "OtaManager.h"
#include <CertStoreBearSSL.h>
#include <DoubleResetDetector.h>
#include <ESP_OTA_GitHub.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#include "Arduino.h"
#include "version.h"

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 5
// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

#define WIFI_RESET_BUTTON 4

BearSSL::CertStore certStore;

OtaManager::OtaManager() {}

char* OtaManager::getApiKey() {
    return _apiKey;
}

bool shouldSaveConfig = false;

// https://github.com/zhouhan0126/WIFIMANAGER-ESP32/blob/master/examples/AutoConnectWithFSParameters/AutoConnectWithFSParameters.ino
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void wifiReset() {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Serial.println("WifiManager resetted...");
    // ESP.restart();
}

void OtaManager::readConfiguration() {
    Serial.println("mounting FS...");
    if (LittleFS.begin()) {
        Serial.println("mounted file system");
        if (LittleFS.exists("/config.json")) {
            Serial.println("reading config file");
            File configFile = LittleFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("opened config file");
                size_t size = configFile.size();
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonDocument json(1024);
                auto error = deserializeJson(json, buf.get());
                if (!error) {
                    Serial.println("parsed json");

                    if (json["githubUser"]) {
                        strcpy(githubUser, json["githubUser"]);
                    }
                    if (json["githubRepo"]) {
                        strcpy(githubRepo, json["githubRepo"]);
                    }
                    if (json["githubReleaseFilename"]) {
                        strcpy(githubReleaseFilename, json["githubReleaseFilename"]);
                    }
                    if (json["apiKey"]) {
                        strcpy(_apiKey, json["apiKey"]);
                    }

                    Serial.print("githubUser: ");
                    Serial.println(githubUser);
                    Serial.print("githubRepo: ");
                    Serial.println(githubRepo);
                    Serial.print("githubReleaseFilename: ");
                    Serial.println(githubReleaseFilename);
                    Serial.print("apiKey: ");
                    Serial.println(_apiKey);
                } else {
                    Serial.println("failed to load json config");
                }
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }
}

void OtaManager::writeConfiguration() {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["githubUser"] = githubUser;
    json["githubRepo"] = githubRepo;
    json["githubReleaseFilename"] = githubReleaseFilename;
    json["apiKey"] = _apiKey;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("failed to open config file for writing");
    }

    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
}

void OtaManager::setupWifimanager(bool startConfigPortal = false, int configPortalButtonPin = 4) {
    pinMode(configPortalButtonPin, INPUT_PULLUP);
    if (digitalRead(configPortalButtonPin) == LOW) {
        Serial.println("Button for configportal pressed");
        startConfigPortal = true;
    }

    WiFi.mode(WIFI_STA);
    WiFiManager wifiManager;

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    readConfiguration();

    WiFiManagerParameter custom_githubUser("githubUser", "githubUser", githubUser, 40);
    WiFiManagerParameter custom_githubRepo("githubRepo", "githubRepo", githubRepo, 40);
    WiFiManagerParameter custom_githubReleaseFilename("githubReleaseFilename", "githubReleaseFilename", githubReleaseFilename, 40);
    WiFiManagerParameter custom_apiKey("apiKey", "apiKey", _apiKey, 40);

    wifiManager.addParameter(&custom_githubUser);
    wifiManager.addParameter(&custom_githubRepo);
    wifiManager.addParameter(&custom_githubReleaseFilename);
    wifiManager.addParameter(&custom_apiKey);

    bool res;

    if (startConfigPortal) {
        res = wifiManager.startConfigPortal("esp", "12345678");
    } else {
        res = wifiManager.autoConnect("esp", "12345678");
    }

    if (!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } else {
        Serial.println("connected...");
        strcpy(githubUser, custom_githubUser.getValue());
        strcpy(githubRepo, custom_githubRepo.getValue());
        strcpy(githubReleaseFilename, custom_githubReleaseFilename.getValue());
        strcpy(_apiKey, custom_apiKey.getValue());
    }
    if (shouldSaveConfig) {
        writeConfiguration();
        ESP.reset();
    }
}

void OtaManager::checkForUpdate() {
    LittleFS.begin();
    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    Serial.print(F("Number of CA certs read: "));
    Serial.println(numCerts);
    if (numCerts == 0) {
        Serial.println(F("No certs found. Did you run certs-from-mozill.py and upload the LittleFS directory before running?"));
        return;  // Can't connect to anything w/o certs!
    }

    Serial.print("Current GITHUB_RELEASE_VERSION: ");
    Serial.println(GITHUB_RELEASE_VERSION);
    Serial.println("Checking for update...");
    ESPOTAGitHub ESPOTAGitHub(&certStore, githubUser, githubRepo, GITHUB_RELEASE_VERSION, githubReleaseFilename, 1 /* accept prerelease */);
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