#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include "Arduino.h"

class OtaManager {
    public:
        OtaManager();
        void wifiReset();
        void setupWifimanager(bool startConfigPortal, bool detectDoubleReset);
        void checkForUpdate();
        char* getApiKey();
    private:
        char githubUser[40];
        char githubRepo[40];
        char githubReleaseFilename[40];
        char _apiKey[40];
        void readConfiguration();
        void writeConfiguration();
};
#endif