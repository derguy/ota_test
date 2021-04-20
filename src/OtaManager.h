#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include "Arduino.h"

class OtaManager {
    public:
        OtaManager();
        void wifiReset();
        void setupWifimanager(bool startConfigPortal);
        void checkForUpdate();
        char* getApiKey();
    private:
        char _apiKey[40];
        void readConfiguration();
        void writeConfiguration();
};
#endif