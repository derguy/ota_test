# OTA example

## generate certs
https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/BearSSL_CertStore
python certs-from-mozilla.py
copy to project /data/certs.ar
## Upload certs.ar to arduino filesystem
pio run -t uploadfs

