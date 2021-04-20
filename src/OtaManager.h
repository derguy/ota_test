#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include "Arduino.h"

class OtaManager {
    public:
        OtaManager();
        char apiKey[40];
        void wifiReset();
        void setupWifimanager(bool startConfigPortal);
        void checkForUpdate();
};
#endif