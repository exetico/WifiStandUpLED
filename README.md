# How to

## Libs

Most libs are added in "platformio.ini", but WifiManager is (as we speak) only released on 0.16.

Therefore WifiManger is added manually in `libs` from:
https://github.com/tzapu/WiFiManager

Cause we're using the `wm.getWebPortalActive()` function.

## Sources / Links

### Reference

https://lastminuteengineers.com/oled-display-esp8266-tutorial/
https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
https://iotforgeeks.com/interface-i2c-oled-display-with-nodemcu/
https://github.com/olikraus/u8g2/wiki/setup_tutorial
https://www.aranacorp.com/en/using-an-oled-display-with-arduino/
https://github.com/olikraus/U8glib_Arduino/blob/master/examples/HelloWorld/HelloWorld.ino
https://efcomputer.net.au/blog/checkout/order-received/24145/?key=wc_order_qhEn46C4K2HWF + https://efcomputer.net.au/blog/product/esp8266-led-strip-wifi-source-code/ + https://www.instructables.com/WiFi-Controlled-RGB-LED-Strip-With-ESP8266/
https://github.com/tzapu/WiFiManager/issues/1426
https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/HTTPSRequest/HTTPSRequest.ino
https://buger.dread.cz/simple-esp8266-https-client-without-verification-of-certificate-fingerprint.html
https://arduino-esp8266.readthedocs.io/en/2.4.1/esp8266wifi/client-secure-examples.html
https://www.theamplituhedron.com/articles/How-to-connect-to-an-SSL-protected-server-with-ESP8266(WiFiClient)/
https://maakbaas.com/esp8266-iot-framework/logs/https-requests/
https://github.com/WestfW/Duino-hacks/blob/master/delay_without_delay/delay_without_delay.ino
https://forum.arduino.cc/t/multiple-buttons-counting-their-presses/401180/5
https://forum.arduino.cc/t/pushing-button-once-twice-three-times-hold-do-action/677512/6
https://www.instructables.com/NodeMCU-ProjectButton-Control-LED/
https://www.calculator.net/resistor-calculator.html?bandnum=5&band1=brown&band2=black&band3=black&multiplier=red&tolerance=brown&temperatureCoefficient=brown&type=c&x=37&y=16
https://arduinogetstarted.com/tutorials/arduino-button-library
https://e-radionica.com/en/blog/what-is-pull-up-pull-down-resistor/
Button wirering: https://i.imgur.com/kr8ZFfU.png
// https://github.com/PaulStoffregen/Time

HTTPS is currently insecure, but that's fine for our config.. But check this, if you need to fix it.. at some point
https://maakbaas.com/esp8266-iot-framework/logs/https-requests/

### Guides

https://arduinojson.org/ (Json Arduino)
https://iotexpert.com/debugging-ssd1306-display-problems/

### Inspiration

https://github.com/tzapu/WiFiManager/issues/1426
https://github.com/tzapu/WiFiManager/issues/1363
https://github.com/tzapu/WiFiManager/issues/1362
https://github.com/tzapu/WiFiManager/issues/1320
https://stackoverflow.com/questions/67702822/error-call-to-httpclientbegin-declared-with-attribute-error-obsolete-api

### Screens

https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
https://github.com/olikraus/u8g2/wiki/u8x8reference
SEE https://github.com/olikraus/u8g2/issues/149
https://forum.arduino.cc/t/random-pixels-showing-on-oled/1041400/13
