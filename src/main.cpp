// https://lastminuteengineers.com/oled-display-esp8266-tutorial/
// https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// https://iotforgeeks.com/interface-i2c-oled-display-with-nodemcu/
// https://github.com/olikraus/u8g2/wiki/setup_tutorial
// https://www.aranacorp.com/en/using-an-oled-display-with-arduino/
// https://github.com/olikraus/U8glib_Arduino/blob/master/examples/HelloWorld/HelloWorld.ino

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h> // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>

// OLED SCREEN
#include <U8g2lib.h>
//Parameters
char cstr [16];//To convert int to char
//Variables
int oledCount  = 0;

//Objects
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://github.com/olikraus/u8g2/wiki/u8x8reference
// SEE https://github.com/olikraus/u8g2/issues/149


// Time
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// SCREEN
//U8X8_SH1106_128X64_NONAME_HW_I2C u8g2(/* reset=*/ U8X8_PIN_NONE);

// SETUP
void setup() {
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","standup"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi 
        Serial.println("Connected...Wuhu :)");

        Serial.println("Calling timeClient");
        timeClient.begin();


        // Screen
        // initialize with the I2C addr 0x3C
        Serial.println("Screen begin");
  // picture loop
  u8x8.begin();

    Serial.println("Screen end");
    }
}

void loop() {
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0,"Hello World!");
  delay(1000);

    // Time
    timeClient.update();

    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());
    //Serial.println(timeClient.getFormattedTime());

    delay(1000);
}

