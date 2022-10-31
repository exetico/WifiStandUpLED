// Reference
// https://lastminuteengineers.com/oled-display-esp8266-tutorial/
// https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// https://iotforgeeks.com/interface-i2c-oled-display-with-nodemcu/
// https://github.com/olikraus/u8g2/wiki/setup_tutorial
// https://www.aranacorp.com/en/using-an-oled-display-with-arduino/
// https://github.com/olikraus/U8glib_Arduino/blob/master/examples/HelloWorld/HelloWorld.ino
// https://efcomputer.net.au/blog/checkout/order-received/24145/?key=wc_order_qhEn46C4K2HWF + https://efcomputer.net.au/blog/product/esp8266-led-strip-wifi-source-code/ + https://www.instructables.com/WiFi-Controlled-RGB-LED-Strip-With-ESP8266/
// https://github.com/tzapu/WiFiManager/issues/1426
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/HTTPSRequest/HTTPSRequest.ino
// https://buger.dread.cz/simple-esp8266-https-client-without-verification-of-certificate-fingerprint.html
// https://arduino-esp8266.readthedocs.io/en/2.4.1/esp8266wifi/client-secure-examples.html
// https://www.theamplituhedron.com/articles/How-to-connect-to-an-SSL-protected-server-with-ESP8266(WiFiClient)/
// https://maakbaas.com/esp8266-iot-framework/logs/https-requests/
// https://github.com/WestfW/Duino-hacks/blob/master/delay_without_delay/delay_without_delay.ino
// https://forum.arduino.cc/t/multiple-buttons-counting-their-presses/401180/5
// https://forum.arduino.cc/t/pushing-button-once-twice-three-times-hold-do-action/677512/6
// https://www.instructables.com/NodeMCU-ProjectButton-Control-LED/
// https://www.calculator.net/resistor-calculator.html?bandnum=5&band1=brown&band2=black&band3=black&multiplier=red&tolerance=brown&temperatureCoefficient=brown&type=c&x=37&y=16
// https://arduinogetstarted.com/tutorials/arduino-button-library
// https://e-radionica.com/en/blog/what-is-pull-up-pull-down-resistor/
// Button wirering: https://i.imgur.com/kr8ZFfU.png

// HTTPS is currently insecure, but that's fine for our config.. But check this, if you need to fix it.. at some point
// https://maakbaas.com/esp8266-iot-framework/logs/https-requests/

// Guides
// https://arduinojson.org/ (Json Arduino)
// https://iotexpert.com/debugging-ssd1306-display-problems/

// Inspiration
// https://github.com/tzapu/WiFiManager/issues/1426
// https://github.com/tzapu/WiFiManager/issues/1363
// https://github.com/tzapu/WiFiManager/issues/1362
// https://github.com/tzapu/WiFiManager/issues/1320
// https://stackoverflow.com/questions/67702822/error-call-to-httpclientbegin-declared-with-attribute-error-obsolete-api

// Screens
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://github.com/olikraus/u8g2/wiki/u8x8reference
// SEE https://github.com/olikraus/u8g2/issues/149
// https://forum.arduino.cc/t/random-pixels-showing-on-oled/1041400/13

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>
#include <ezButton.h>

// VARS
bool __WIFI_CONNECTED = false;
bool fetching_config = false;
int __CONFIG_ENABLED_FROM = 0;
int __CONFIG_ENABLED_TO = 0;
int __CONFIG_STAND_UP_PERIOD_MIN = 0;

// BUTTON VARS
ezButton button(2); // create ezButton object; // // GPIO2 = D4
unsigned long lastCount = 0;
unsigned long count = 0;
unsigned long buttonIsPressed = 0;

unsigned long lastButtonPressTargetMs = 4000;
unsigned long lastButtonPressMillis = millis();
unsigned long lastButtonPressWasLongPress = 0;

// OLED SCREEN
#include <U8g2lib.h>
// Parameters
char cstr[16]; // To convert int to char

// Objects
//  VIRKER 128x64: U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
// FORSØG MED 128x32.... VIRKER IKKE: U8X8_SSD1306_128X32_UNIVISION_2ND_HW_I2C u8x8(U8X8_PIN_NONE);
// FORSØG MED 128x32.... VIRKER IKKE: U8X8_SSD1306_128X32_WINSTAR_2ND_HW_I2C u8x8(U8X8_PIN_NONE);

// Time
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// WHITE Strips
#include "Color.h"
const int redLED = 14;   // D3 GPIO0
const int greenLED = 12; // D2 GPIO2
Color createColor(redLED, greenLED);

// Download files
#include <WiFiClient.h>
WiFiClientSecure client;
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
/* Import: See https://randomnerdtutorials.com/esp32-http-get-post-arduino/#comment-722499*/
#include <ArduinoJson.h>

char *HTTPS_PATH = "/playground/stand-up-led.json";
char *HTTPS_START = "https://";
char *CONFIG_HOST = "tobiasnordahl.dk";

String CONFIG_URL = String(HTTPS_START) + String(CONFIG_HOST) + String(HTTPS_PATH);
const char *URL_COMPLETE = CONFIG_URL.c_str();

void log_i(const String &txt, int configInt)
{
  Serial.println(" ");
  Serial.print(txt + " ");
  Serial.print(String(configInt));
  Serial.println(" ");
}

bool fetch_json_config(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    String url = CONFIG_URL;

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    if (https.begin(*client, CONFIG_URL))
    { // HTTPS
      Serial.println("[HTTPS] GET...");
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server?
        if (httpCode == HTTP_CODE_OK)
        {
          String payload = https.getString();
          Serial.println(String("[HTTPS] Received payload: ") + payload);

          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, payload);
          JsonObject obj = doc.as<JsonObject>();

          if (error)
          {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return 1;
          }

          __CONFIG_ENABLED_FROM = int(obj["config"]["enabledFrom"]);
          __CONFIG_ENABLED_TO = int(obj["config"]["enabledTo"]);
          __CONFIG_STAND_UP_PERIOD_MIN = int(obj["config"]["standUpPeriodeMin"]);

          log_i("__CONFIG_ENABLED_FROM", __CONFIG_ENABLED_FROM);
          log_i("__CONFIG_ENABLED_TO", __CONFIG_ENABLED_TO);
          log_i("__CONFIG_STAND_UP_PERIOD_MIN", __CONFIG_STAND_UP_PERIOD_MIN);

          return 0;
        }
      }
      else
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());

        return 0;
      }

      https.end();
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n\r");
    }

    return 0;
  }
  else
  {
    Serial.println("Wifi Err");
    return 1;
  }
}

void get_json_config(void)
{
  fetching_config = true;
  while (fetching_config)
  {
    fetching_config = fetch_json_config();
  }
}

void updateScreenAndTime()
{
  // u8x8.setFont(u8x8_font_chroma48medium8_r);
  // u8x8.drawString(0,0,"Hello World!");
  // delay(1000);

  // Time
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime();
  // const char * c = formattedTime.c_str();

  u8x8.setCursor(0, 0);
  u8x8.drawString(0, 0, formattedTime.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done
  u8x8.setCursor(0, 1);
  String configString = "F:" + String(__CONFIG_ENABLED_FROM) + ",T:" + String(__CONFIG_ENABLED_TO) + ",M:" + String(__CONFIG_STAND_UP_PERIOD_MIN);
  u8x8.drawString(0, 1, configString.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done

  u8x8.setCursor(0, 2);
  String countString = "Count:" + String(count) + "   ";
  u8x8.drawString(0, 2, countString.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done

  u8x8.setCursor(0, 3);
  String buttonString = "Knap:" + String(buttonIsPressed ? "Aktiv    " : "Ej i brug") + "  ";
  u8x8.drawString(0, 3, buttonString.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done

  // Print date and time to serial
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  // Serial.println(timeClient.getFormattedTime());
}

unsigned long screenIsInitiated = 0;

void screenPrintText(String text, unsigned long position, unsigned long line, unsigned long cleanScreen)
{
  if (!screenIsInitiated)
  {
    screenIsInitiated = 1;
    // Screen begin
    u8x8.begin();
    u8x8.setFont(u8x8_font_victoriabold8_r);
  }

  if (cleanScreen)
  {
    u8x8.clearDisplay();
  }

  String stringTxt = text;
  u8x8.drawString(position, line, stringTxt.c_str());
}

// SETUP
void setup()
{
  // Txt
  screenPrintText("Starter op...", 0, 0, 1);

  // Buttons
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  button.setCountMode(COUNT_FALLING);

  // put your setup code here, to run once:
  Serial.begin(115200);

  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

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
    screenPrintText("Wifi-fejl...", 0, 0, 1);
    // ESP.restart();
  }

  if (res)
  {
    __WIFI_CONNECTED = true;

    // if you get here you have connected to the WiFi
    Serial.println("Connected...Wuhu :)");

    Serial.println("Calling timeClient");
    timeClient.begin();

    // Download file
    Serial.println("Download begin");
    get_json_config();

    if (__CONFIG_ENABLED_FROM && __CONFIG_ENABLED_TO && __CONFIG_STAND_UP_PERIOD_MIN)
    {
      Serial.println("Download OK");
    }
    Serial.println("Download end");

    // Screen
    // initialize with the I2C addr 0x3C
    Serial.println("Screen begin");

    // Color test
    screenPrintText("whiteRed", 0, 0, 1);
    createColor.off();
    createColor.whiteRed();
    delay(1000);

    screenPrintText("whiteGreen", 0, 0, 1);
    createColor.off();
    createColor.whiteGreen();
    delay(1000);

    u8x8.clearDisplay();
    screenPrintText("Blink animation", 0, 0, 1);
    createColor.off();
    createColor.blink();

    u8x8.clearDisplay();
    screenPrintText("Done wait 1sec", 0, 0, 1);
    delay(1000);

    createColor.off();

    Serial.println("Screen end");
    u8x8.clearDisplay();
  }
  else
  {
    Serial.println("Configuration portal running");

    u8x8.setCursor(0, 0);
    String errorMessage = "Please setup wifi";
    u8x8.drawString(0, 0, errorMessage.c_str()); // Txt needs to be "const char * c "... In String-type, that can be done
  }
}

// Delays
boolean delay_without_delaying(unsigned long time)
{
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  static unsigned long previousmillis = 0;
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= time)
  {
    previousmillis = currentmillis;
    return true;
  }
  return false;
}

boolean delay_without_delaying(unsigned long &since, unsigned long time)
{
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time)
  {
    since = currentmillis;
    return true;
  }
  return false;
}

void loop()
{
  // Button counts
  button.loop(); // MUST call the loop() function first

  if (button.isPressed())
  {
    Serial.println("The button is pressed");
    buttonIsPressed = 1;
    createColor.whiteBoth();
  }

  if (button.isReleased())
  {
    Serial.println("The button is released");
    buttonIsPressed = 0;
    createColor.off();
  }

  if (lastButtonPressWasLongPress)
  {
    // Not long press tigger anymore, and fade light to confirm userinput
    lastButtonPressWasLongPress = !lastButtonPressWasLongPress;
    createColor.off();
    createColor.blink();
    createColor.off();
  }

  count = button.getCount();

  if (count != lastCount)
  {
    Serial.println(count);

    int countIn3 = count % 3 + 1;

    lastButtonPressMillis = millis();

    switch (countIn3)
    {
    case 1:
      Serial.println("Case 1, woop");
      break;

    case 2:
      Serial.println("Case 2, woop");
      break;

    case 3:
      Serial.println("Case 3, woop");
      break;
    }

    lastCount = count;
  }

  // Loop, different timeperiods
  static unsigned long ledtime = 0;
  static unsigned long atime, btime, ctime;

  if (delay_without_delaying(ledtime, 500))
  {
    // Check if button press has been too long time ago
    unsigned long currentButtonmillis = millis();
    if (button.getCount() > 0 && currentButtonmillis - lastButtonPressMillis >= lastButtonPressTargetMs)
    {
      // Reset count
      button.resetCount();
      Serial.print("ResetButtonCount");

      // Set state of longpress button
      if (buttonIsPressed)
        lastButtonPressWasLongPress = 1;
    }
  }
  // Every 1000
  if (delay_without_delaying(atime, 1000))
  {
    updateScreenAndTime();
  }
  if (delay_without_delaying(btime, 5000))
  {
    // Nothing right now - Serial.print("B");
  }
  if (delay_without_delaying(ctime, 500))
  {
    // Nothing right now - Serial.print("C");
  }
}
