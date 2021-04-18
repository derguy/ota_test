# OTA example

## generate certs
https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi/examples/BearSSL_CertStore
python certs-from-mozilla.py
copy to project /data/certs.ar
## Upload certs.ar to arduino filesystem
pio run -t uploadfs

# Creating new release by tag
git tag -a 0.0.2 -m "test 0.0.2" && git push origin --tags

