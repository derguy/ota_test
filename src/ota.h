#ifndef OTA_H
#define OTA_H

extern char apiKey[40];

void wifiReset();

void setupWifimanager(bool startConfigPortal);

void checkForUpdate();

#endif