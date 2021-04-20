#ifndef OTA_H
#define OTA_H

void sleep(unsigned long sleeptime);

void wifiReset();

void setupWifimanager(bool startConfigPortal);

void checkForUpdate();

#endif