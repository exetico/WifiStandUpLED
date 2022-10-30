// https://lastminuteengineers.com/oled-display-esp8266-tutorial/
// https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// https://iotforgeeks.com/interface-i2c-oled-display-with-nodemcu/
// https://github.com/olikraus/u8g2/wiki/setup_tutorial
// https://www.aranacorp.com/en/using-an-oled-display-with-arduino/
// https://github.com/olikraus/U8glib_Arduino/blob/master/examples/HelloWorld/HelloWorld.ino
// https://efcomputer.net.au/blog/checkout/order-received/24145/?key=wc_order_qhEn46C4K2HWF + https://efcomputer.net.au/blog/product/esp8266-led-strip-wifi-source-code/ + https://www.instructables.com/WiFi-Controlled-RGB-LED-Strip-With-ESP8266/

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>

// OLED SCREEN
#include <U8g2lib.h>
// Parameters
char cstr[16]; // To convert int to char
// Variables
int oledCount = 0;

// Objects
//  VIRKER 128x64: U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
// FORSØG MED 128x32.... VIRKER IKKE: U8X8_SSD1306_128X32_UNIVISION_2ND_HW_I2C u8x8(U8X8_PIN_NONE);
// FORSØG MED 128x32.... VIRKER IKKE: U8X8_SSD1306_128X32_WINSTAR_2ND_HW_I2C u8x8(U8X8_PIN_NONE);

// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://github.com/olikraus/u8g2/wiki/u8x8reference
// SEE https://github.com/olikraus/u8g2/issues/149
// https://forum.arduino.cc/t/random-pixels-showing-on-oled/1041400/13

// Time
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// RGB Strip
#include "Color.h"
const int redLED = 14;   // D3 GPIO0
const int greenLED = 12; // D2 GPIO2
const int blueLED = 13;  // D4 GPIO4
Color createColor(redLED, greenLED, blueLED);

// SETUP
void setup()
{
  // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // put your setup code here, to run once:
  Serial.begin(115200);

  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
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
  res = wm.autoConnect("AutoConnectAP", "standup"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("Connected...Wuhu :)");

    Serial.println("Calling timeClient");
    timeClient.begin();

    // Screen
    // initialize with the I2C addr 0x3C
    Serial.println("Screen begin");
    // picture loop
    u8x8.begin();
    u8x8.setFont(u8x8_font_victoriabold8_r);

    // Color test
    createColor.off();
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "whiteRed");
    createColor.whiteRed();
    delay(1000);

    createColor.off();
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "whiteGreen");
    createColor.whiteGreen();
    delay(1000);

    createColor.off();
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "Blink animation");
    createColor.blink();

    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "Done, wait 2sec");
    delay(2000);

    createColor.off();

    Serial.println("Screen end");
    u8x8.clearDisplay();
  }
}

void loop()
{
  // u8x8.setFont(u8x8_font_chroma48medium8_r);
  // u8x8.drawString(0,0,"Hello World!");
  // delay(1000);

  // Time
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime();
  // const char * c = formattedTime.c_str();

  u8x8.drawString(0, 0, formattedTime.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done with c_str()
  delay(1000);

  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  // Serial.println(timeClient.getFormattedTime());
}
