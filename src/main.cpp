#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>
#include <ezButton.h>

// VARS
//-- WIFI
bool __WIFI_CONNECTED = false;

//-- SCREEN
unsigned long screenIsInitiated = 0;

//-- CONFIG
bool fetching_config = false;
int __CONFIG_ENABLED_FROM = 0;
int __CONFIG_ENABLED_TO = 0;
int __CONFIG_STAND_UP_PERIOD_MIN = 0;

//-- STAND_UP
bool __CURRENT_IS_STANDING = 0;
bool __CURRENT_IS_SITTING = 0;
int __CURRENT_START_PERIOD_TIME_MIN = 0;
int __CURRENT_FROM_TIME_SEC = 0;
int __CURRENT_TO_TIME_SEC = 0;
int __CHANGE_POSITION_PREVIOUS = 0;
int __CHANGE_POSITION_NEXT = 0;
int __CHANGE_POSITION_NEXT_EPOCH = 0;
int __TARGET_DAY = 0;
int __CURRENT_POSITION = 0; // See lightStates

//-- TIME
int __THIS_TIME = 0;
int __TIME_HH = 0;
int __TIME_MM = 0;
int __TIME_SS = 0;
int __TIME_EPOCH = 0;
String __TIME_FORMATTED = "";

//-- BUTTONS
ezButton button(2); // GPIO2 = D4
unsigned long lastCount = 0;
unsigned long count = 0;
unsigned long buttonIsPressed = 0;

unsigned long lastButtonPressTargetMs = 4000;
unsigned long lastButtonPressMillis = millis();
unsigned long lastButtonPressWasLongPress = 0;

// OLED SCREEN
#include <U8g2lib.h>

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
char lightStates[3][12] = {"Ukendt", "Standup", "Sit"};

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

// WIFI - WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wm;

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

int getStandUpStartPeriod()
{
  // If used has defined another period by using the button
  if (__CURRENT_START_PERIOD_TIME_MIN)
  {
    return __CURRENT_START_PERIOD_TIME_MIN;
  }

  // Else, default to first minute in hour
  return 0;
}

void setNextPeriodTime()
{
  int __START_PERIOD_MM = int(getStandUpStartPeriod());
  __CURRENT_FROM_TIME_SEC = (__CONFIG_ENABLED_FROM * 60 * 60);                        // HH ie. 17 * 60 * 60
  __CURRENT_TO_TIME_SEC = (__CONFIG_ENABLED_TO * 60 * 60) + (__START_PERIOD_MM * 60); // HH ie. 17 * 60 * 60 + MM

  Serial.println("");
  Serial.println("__START_PERIOD_MIN " + String(__START_PERIOD_MM));
  Serial.println("__CURRENT_FROM_TIME_SEC " + String(__CURRENT_FROM_TIME_SEC));
  Serial.println("__CURRENT_TO_TIME_SEC " + String(__CURRENT_TO_TIME_SEC));
  Serial.println("__CHANGE_POSITION_PREVIOUS " + String(__CHANGE_POSITION_PREVIOUS));

  __CHANGE_POSITION_PREVIOUS = __CHANGE_POSITION_NEXT;

  // If target MM is already passed
  // If same minute, add one hour... Else we'll stick to the same minute, in 60 sec
  // if (__TIME_MM >= __START_PERIOD_MM)
  // {
  //   __CHANGE_POSITION_NEXT = ((__TIME_HH + 1) * 60 * 60) + (__START_PERIOD_MM * 60) + (__TIME_SS);
  //   Serial.println("Bob1:" + String(__CHANGE_POSITION_NEXT));
  // }
  // else
  // {
  //   __CHANGE_POSITION_NEXT = (__TIME_HH * 60 * 60) + (__START_PERIOD_MM * 60) + (__TIME_SS);
  //   Serial.println("Bob2");
  // }

  __CHANGE_POSITION_NEXT = (__TIME_HH * 60 * 60) + (__START_PERIOD_MM * 60) + (__CONFIG_STAND_UP_PERIOD_MIN * 60);
  Serial.println("CPN:" + String(__CHANGE_POSITION_NEXT));

  // If next position change is out of allowed period...
  if (__CHANGE_POSITION_NEXT > __CURRENT_TO_TIME_SEC)
  {
    __TARGET_DAY = 
    Serial.println("Bob3 CPN: " + String(__CHANGE_POSITION_NEXT));
    __CHANGE_POSITION_NEXT = (__CONFIG_ENABLED_FROM * 60 * 60) + (__CONFIG_STAND_UP_PERIOD_MIN * 60);
    __CHANGE_POSITION_NEXT_EPOCH = __TIME_EPOCH + (60*60*24 -__THIS_TIME) + __CHANGE_POSITION_NEXT; // Current EPOCH + (length-of-day - current time) + new position, the next day
    Serial.println("Bob3 CPN After: " + String(__CHANGE_POSITION_NEXT));
    Serial.println("Bob3 CEF: " + String(__CONFIG_ENABLED_FROM));
    Serial.println("Bob3 SPM: " + String(__CONFIG_STAND_UP_PERIOD_MIN));
  }

  Serial.println("__CHANGE_POSITION_PREVIOUS " + String(__CHANGE_POSITION_PREVIOUS));
  Serial.println("__CHANGE_POSITION_NEXT " + String(__CHANGE_POSITION_NEXT));
}

void updatePositionInfo()
{
  // Update screen
  screenPrintText("Update position", 0, 1, 0);

  // Update light
  createColor.off();

  if (__CURRENT_IS_STANDING)
  {
    Serial.println("Standing...  Sitting");
    createColor.whiteRed();
    __CURRENT_IS_STANDING = 0;
    __CURRENT_IS_SITTING = 1;
  }
  else if (__CURRENT_IS_SITTING)
  {
    Serial.println("Sitting... Standing");
    createColor.whiteGreen();
    __CURRENT_IS_STANDING = 1;
    __CURRENT_IS_SITTING = 0;
  }
  else if (!__CURRENT_IS_SITTING && !__CURRENT_IS_STANDING)
  {
    Serial.println("NOTHING... Standing");
    createColor.whiteGreen();
    __CURRENT_IS_STANDING = 1;
    __CURRENT_IS_SITTING = 0;
  }
  Serial.println("Nope...  nah");
}

void updateTimeAndPosition()
{
  setNextPeriodTime();
  updatePositionInfo();
}

void checkPositionGuidance()
{
  // __CONFIG_ENABLED_FROM = 0;
  // __CONFIG_ENABLED_TO = 0;
  // __CONFIG_STAND_UP_PERIOD_MIN = 0;
  // __CURRENT_IS_STANDING = 0;
  // __CURRENT_IS_SITTING = 0;
  // __CURRENT_START_PERIOD_TIME_MIN = 0;

  __THIS_TIME = (__TIME_HH * 60 * 60) + (__TIME_MM * 60) + (__TIME_SS);
  Serial.println("This time" + String(__THIS_TIME) + " .. " + String(__CHANGE_POSITION_NEXT) + " .. " + String(__TIME_EPOCH) + " .. " + String(__CHANGE_POSITION_NEXT_EPOCH));

  // Fresh run
  if (int(__CHANGE_POSITION_NEXT) == (0))
  {
    updateTimeAndPosition();
    Serial.println("NPIX " + String(__THIS_TIME) + " ... " + String(__CHANGE_POSITION_NEXT));
  }
  else if ( // Check if a change is needed
      int(__THIS_TIME) > int(__CURRENT_FROM_TIME_SEC) &&
      int(__THIS_TIME) <= int(__CURRENT_TO_TIME_SEC) &&
      int(__THIS_TIME) > int(__CHANGE_POSITION_NEXT))
  {
    updateTimeAndPosition();
    Serial.println("NPIY " + String(__THIS_TIME) + " ... " + String(__CHANGE_POSITION_NEXT));
  }
  else if ( // Time is passed, but period is out of scope
      int(__THIS_TIME) > int(__CURRENT_FROM_TIME_SEC) &&
      int(__THIS_TIME) > int(__CURRENT_TO_TIME_SEC) &&
      int(__THIS_TIME) > int(__CHANGE_POSITION_NEXT) &&
      int(__TIME_EPOCH) >= int(__CHANGE_POSITION_NEXT_EPOCH)
      )
  {
    updateTimeAndPosition();
    Serial.println("NPIZ " + String(__THIS_TIME) + " ... " + String(__CHANGE_POSITION_NEXT));
  }
  else if(
      int(__THIS_TIME) > int(__CHANGE_POSITION_NEXT)
      &&
       (int(__CHANGE_POSITION_NEXT) != int(__CURRENT_FROM_TIME_SEC + (__CONFIG_STAND_UP_PERIOD_MIN * 60)))
  ){
    Serial.println("!PIZ " + String(__THIS_TIME) + " ... " + String(__CHANGE_POSITION_NEXT));
  updateTimeAndPosition();
  }

  // if (__THIS_TIME > __CHANGE_POSITION_NEXT)
  // {
  //   Serial.println("...1 __THIS_TIME > __CHANGE_POSITION_NEXT");
  // }

  // if (int(__THIS_TIME) > int(__CHANGE_POSITION_NEXT))
  // {
  //   Serial.println("...2 __THIS_TIME > __CHANGE_POSITION_NEXT");
  // }
}

void updateScreenAndTime()
{
  // Line 1
  screenPrintText(__TIME_FORMATTED.c_str(), 0, 0, 1); // Txt needs to be "const char * c "... In String-type, that can be done

  // Line 2
  String configString = "F:" + String(__CONFIG_ENABLED_FROM) + ",T:" + String(__CONFIG_ENABLED_TO) + ",M:" + String(__CONFIG_STAND_UP_PERIOD_MIN);
  screenPrintText(configString.c_str(), 0, 1, 0);

  // Line 3
  String countString = "Count:" + String(count) + "   ";
  screenPrintText(countString.c_str(), 0, 2, 0);

  // Line 4
  String buttonString = "Knap:" + String(buttonIsPressed ? "Aktiv    " : "Ej i brug") + "  ";
  screenPrintText(buttonString.c_str(), 0, 3, 0);

  // Line 5
  String extraString1 = "_T_T:" + String(__THIS_TIME);
  screenPrintText(extraString1.c_str(), 0, 4, 0);

  // Line 6
  String extraString2 = "_C_P_N:" + String(__CHANGE_POSITION_NEXT);
  screenPrintText(extraString2.c_str(), 0, 5, 0);

  // Line 7
  String extraString3 = "_CS_M:" + String(__CURRENT_START_PERIOD_TIME_MIN);
  screenPrintText(extraString3.c_str(), 0, 6, 0);

  // Print date and time to serial
  Serial.println(daysOfTheWeek[timeClient.getDay()] + __TIME_FORMATTED);

  // Update our table position
  checkPositionGuidance();
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

  Serial.println("Test" + __CURRENT_POSITION);
  Serial.println(lightStates[__CURRENT_POSITION]);
  Serial.println(lightStates[0]);
  Serial.println(lightStates[1]);
  Serial.println(lightStates[2]);
  

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
  res = wm.autoConnect("AutoConnectAP", "standupnow"); // password protected ap

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
    delay(300);

    screenPrintText("whiteGreen", 0, 0, 1);
    createColor.off();
    createColor.whiteGreen();
    delay(300);

    screenPrintText("Blink animation", 0, 0, 1);
    createColor.off();
    createColor.blinkShort();
    screenPrintText("Done wait 0.5sec", 0, 0, 1);
    delay(500);
    createColor.off();

    Serial.println("Screen end");
    u8x8.clearDisplay();

    // Update position first time
    updatePositionInfo();
  }
  else
  {
    Serial.println("Configuration portal running");
    screenPrintText("No WiFi", 0, 0, 1);
    screenPrintText("-Restart", 0, 1, 0);
    screenPrintText("-Change SSID", 0, 2, 0);
    screenPrintText("-Ping me", 0, 3, 0);
    screenPrintText("-Try stand up", 0, 4, 0);
    screenPrintText("", 0, 5, 0);
    screenPrintText("Didn't create hotspot", 0, 6, 0);
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

String command;

void loop()
{


  // Read serial
    if(Serial.available()){
        command = Serial.readStringUntil('\n');

        Serial.printf("Command received %s \n", command.c_str());

        char * commandStr = const_cast<char*> ( command.c_str() );

        // Serial.println(strtok(commandStr, "bob"));
        // Serial.println(command.indexOf("bob"));
        // Serial.println(command.equals("bob"));

        if(command.indexOf("cpn") == (0)){
          Serial.println(strtok(commandStr, "cpn"));
          long number = atol( strtok(commandStr, "cpn") );
          __CHANGE_POSITION_NEXT = number;
        }

        Serial.println(command.indexOf("o"));
        if(command.indexOf("o") == (0)){
             createColor.off();
             Serial.println("o");
        }

        if(command.indexOf("g") == (0)){
            createColor.whiteGreen();
            Serial.println("g");
        }

        if(command.indexOf("r") == (0)){
            createColor.whiteRed();
            Serial.println("r");
        }


        // if(commandStr){
        //     Serial.println("Ok...Bob");
        // }
        // if(command.equals("bob1")){
        //     Serial.println("ok-bob1");
        // } else if(command.equals("bob2")){
        //     Serial.println("ok-bob2");
        // } else if(command.equals("bob3")){
        //     Serial.println("ok-bob3");
        // } else{
        //     Serial.println("Invalid command");
        // }
    }


  if (wm.getConfigPortalActive())
  {
    Serial.println("No wifi1....");
    screenPrintText("No wifi1...", 0, 0, 1);
  }

  if (wm.getWebPortalActive())
  {
    Serial.println("No wifi2....");
    screenPrintText("No wifi2...", 0, 0, 1);
  }

  // Button counts
  button.loop(); // MUST call the loop() function first

  if (button.isPressed())
  {
    Serial.println("The button is pressed");
    buttonIsPressed = 1;
    // createColor.whiteBoth();
  }

  if (button.isReleased())
  {
    Serial.println("The button is released");
    buttonIsPressed = 0;
    // createColor.off();
  }

  // If no wifi, skip the rest...
  if (!__WIFI_CONNECTED)
  {
    return;
  }

  if (lastButtonPressWasLongPress)
  {
    // Not long press tigger anymore, and fade light to confirm userinput
    lastButtonPressWasLongPress = !lastButtonPressWasLongPress;
    screenPrintText("New MM period:", 0, 0, 1);
    screenPrintText(String(__TIME_MM), 0, 1, 0);
    screenPrintText("saved :)...", 0, 2, 0);
    createColor.off();
    createColor.whiteRed();
    delay(200);
    createColor.off();
    createColor.whiteGreen();
    delay(200);
    createColor.off();
    createColor.whiteRed();
    delay(200);
    createColor.off();
    createColor.whiteGreen();
    delay(200);
    createColor.off();
    createColor.whiteRed();
    delay(200);
    createColor.off();
    createColor.whiteGreen();
    delay(200);
    createColor.off();

    // Set the new minute target
    __CURRENT_START_PERIOD_TIME_MIN = __TIME_MM;
    __CHANGE_POSITION_NEXT = 0;
    __CHANGE_POSITION_PREVIOUS = 0;
    // Calculate new targets
    // createColor.off();
    updateTimeAndPosition();
  }

  count = button.getCount();

  if (count != lastCount)
  {
    Serial.println(count);

    int countIn6 = count % 6 + 1;

    lastButtonPressMillis = millis();

    switch (countIn6)
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

    case 4:
      Serial.println("Case 4, woop");
      break;

    case 5:
      Serial.println("Case 5, woop");
      setNextPeriodTime();
      break;

    case 6:
      Serial.println("Case 6, woop");
    createColor.off();
    createColor.whiteGreen();
    delay(300);
    createColor.whiteRed();
    delay(300);
    createColor.whiteGreen();
    delay(300);
    createColor.whiteRed();
    delay(300);
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
    // Update time
    timeClient.update();
    __TIME_HH = timeClient.getHours();
    __TIME_MM = timeClient.getMinutes();
    __TIME_SS = timeClient.getSeconds();
    __TIME_EPOCH = timeClient.getEpochTime();
    __TIME_FORMATTED = timeClient.getFormattedTime();

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
