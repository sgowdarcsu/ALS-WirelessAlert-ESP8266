// BIOM 300 ALS FSR Wireless Alert System
// Project 5 Group 3
// Authors: Nathan Waanders, Sainandan Gowdar, Eddie Kithara, Sarah Varderame

#include <ESP8266WiFi.h>
#include <ESP8266SMTP.h>
#include <esp8266-google-home-notifier.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int fsrPin = 0;     // the FSR and 2.2K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider
unsigned long startTime;
unsigned long endTime;
unsigned long duration;
byte timerRunning;
GoogleHomeNotifier ghn;

const char* ssid = "stayoutofmywifi";                // WIFI network name
const char* password = "cookiesandcream";           // WIFI network password
const char displayName[] = "Family Room";
uint8_t connection_state = 0;           // Connected to WIFI or not
uint16_t reconnect_interval = 10000;    // If not connected wait time to try again

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);

  connection_state = WiFiConnect(ssid, password);
  if (!connection_state) {        // if not connected to WIFI
    Awaits(reconnect_interval);         // constantly trying to connect
  }

  SMTP.setEmail("alshelpwireless@gmail.com")
  .setPassword("biom300project5")
  .Subject("James-Wheelchair: HELP!")
  .setFrom("ALS Help Alerts")
  .setForGmail();

  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  fsrReading = analogRead(fsrPin);
  Serial.print("Analog reading = ");
  Serial.println(fsrReading);

  if (timerRunning == 0 && fsrReading >= 900) { // button pressed & timer not running already
    startTime = millis();
    timerRunning = 1;
  }
  if (timerRunning == 1 && fsrReading < 900) { // timer running, button released
    endTime = millis();
    timerRunning = 0;
    duration = endTime - startTime;
    if (duration > 1000 && duration <= 3000) {
      Serial.println("Small Amarm");
      tone(5, 999, 1000);

      if (SMTP.Send("sverde@rams.colostate.edu,edkit98@gmail.com,chooseyouradd@gmail.com,waanders@rams.colostate.edu", "NOT URGENT")) {
        Serial.println(F("Message sent"));
      } else {
        Serial.print(F("Error sending message: "));
        Serial.println(SMTP.getError());
      }

      Serial.println("connecting to Google Home...");
      if (ghn.device(displayName, "en") != true) {
        Serial.println(ghn.getLastError());
        return;
      }
      Serial.print("found Google Home(");
      Serial.print(ghn.getIPAddress());
      Serial.print(":");
      Serial.print(ghn.getPort());
      Serial.println(")");

      if (ghn.notify("James needs non-urgent assistance!") != true) {
        Serial.println(ghn.getLastError());
        return;
      }

      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (duration > 3000) {
      Serial.println("Big Alarm");
      tone(5, 999, 3000);
      if (SMTP.Send("sverde@rams.colostate.edu,edkit98@gmail.com,chooseyouradd@gmail.com,waanders@rams.colostate.edu", "EMERGENCY")) {
        Serial.println(F("Message sent"));
      } else {
        Serial.print(F("Error sending message: "));
        Serial.println(SMTP.getError());

      }

      Serial.println("connecting to Google Home...");
      if (ghn.device(displayName, "en") != true) {
        Serial.println(ghn.getLastError());
        return;
      }
      Serial.print("found Google Home(");
      Serial.print(ghn.getIPAddress());
      Serial.print(":");
      Serial.print(ghn.getPort());
      Serial.println(")");

      if (ghn.notify("James needs assistance immediately!") != true) {
        Serial.println(ghn.getLastError());
        return;
      }

      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);

    }
  }

  delay(500);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t WiFiConnect(const char* ssID, const char* nPassword)
{
  static uint16_t attempt = 0;
  Serial.print("Connecting to ");
  Serial.println(ssID);
  WiFi.begin(ssID, nPassword);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 50) {
    delay(200);
    Serial.print(".");
  }
  ++attempt;
  Serial.println("");
  if (i == 51) {
    Serial.print(F("Connection: TIMEOUT on attempt: "));
    Serial.println(attempt);
    if (attempt % 2 == 0)
      Serial.println(F("Check if access point available or SSID and Password are correct\r\n"));
    return false;
  }
  Serial.println(F("Connection: ESTABLISHED"));
  Serial.print(F("Got IP address: "));
  Serial.println(WiFi.localIP());
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awaits(uint16_t interval)
{
  uint32_t ts = millis();
  while (!connection_state) {
    delay(50);
    if (!connection_state && millis() > (ts + interval)) {
      connection_state = WiFiConnect(ssid, password);
      ts = millis();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
