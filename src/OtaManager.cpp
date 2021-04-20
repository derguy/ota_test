#include "Arduino.h"
#include "OtaManager.h"
#include <FS.h>
#include <CertStoreBearSSL.h>
#include <ESP_OTA_GitHub.h>
#include <WiFiManager.h>
#include "version.h"

#define WIFI_RESET_BUTTON 4

BearSSL::CertStore certStore;

OtaManager::OtaManager() {};

char githubUser[40] = "";
char githubRepo[40] = "";
char githubReleaseFilename[40] = "firmware.bin";
char apiKey[40] = "";

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

void readConfiguration() {
    Serial.println("mounting FS...");
    if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json")) {
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
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
                        strcpy(apiKey, json["apiKey"]);
                    }

                    Serial.print("githubUser: ");
                    Serial.println(githubUser);
                    Serial.print("githubRepo: ");
                    Serial.println(githubRepo);
                    Serial.print("githubReleaseFilename: ");
                    Serial.println(githubReleaseFilename);
                    Serial.print("apiKey: ");
                    Serial.println(apiKey);
                } else {
                    Serial.println("failed to load json config");
                }
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }
}

void writeConfiguration() {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["githubUser"] = githubUser;
    json["githubRepo"] = githubRepo;
    json["githubReleaseFilename"] = githubReleaseFilename;
    json["apiKey"] = apiKey;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("failed to open config file for writing");
    }

    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
}

void OtaManager::setupWifimanager(bool startConfigPortal = false) {
    WiFi.mode(WIFI_STA);
    WiFiManager wifiManager;

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    readConfiguration();

    WiFiManagerParameter custom_githubUser("githubUser", "githubUser", githubUser, 40);
    WiFiManagerParameter custom_githubRepo("githubRepo", "githubRepo", githubRepo, 40);
    WiFiManagerParameter custom_githubReleaseFilename("githubReleaseFilename", "githubReleaseFilename", githubReleaseFilename, 40);
    WiFiManagerParameter custom_apiKey("apiKey", "apiKey", apiKey, 40);

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
        strcpy(apiKey, custom_apiKey.getValue());
    }
    if (shouldSaveConfig) {
        writeConfiguration();
        ESP.reset();
    }
}

void OtaManager::checkForUpdate() {
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