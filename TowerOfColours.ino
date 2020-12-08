#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#include "NeoPatterns.h"
#include "arduino_secrets.h"

#define PIN        7
#define NUMPIXELS 60

// 0 (min) to 255 (max)
#define BRIGHTNESS_DAY 12
#define BRIGHTNESS_NIGHT 15

#define RAINBOW_SPEED 400

void onPatternComplete();
NeoPatterns strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800, &onPatternComplete);

///////please enter your sensitive data in arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
const unsigned long getIntervalNight = 2L * 1000L;
const unsigned long getIntervalDay = 60L * 60L * 1000L;

char server[] = SERVER; // Server address

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server);

struct RGB {
  byte r;
  byte g;
  byte b;
};

RGB towerColours[10];
bool isDay = false;
bool isBusy = false;

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS_NIGHT);
  strip.RainbowCycle(RAINBOW_SPEED);

  Serial.begin(9600);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }

  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
}

void loop() {
  const unsigned long interval = millis() - lastConnectionTime;

  if (isDay) {
    strip.setBrightness(BRIGHTNESS_DAY);
    strip.Update();
    if (interval > getIntervalDay) {
      getColours();
    }
  } else {
    if (interval > getIntervalNight) {
      strip.setBrightness(BRIGHTNESS_NIGHT);
      getColours();
      setStripColors();
    }
  }
}

void getColours() {
  client.get("/cbn-live/getColours");

  if (client.responseStatusCode() == 200) {
    String response = client.responseBody();
    Serial.print("Response: ");
    Serial.println(response);

    deserializeJSON(response.c_str());
  }

  lastConnectionTime = millis();
}

void setStripColors() {
  for (int i = 0; i < strip.numPixels(); i++) {
    auto color = towerColours[i/6];
    strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
  }

  // Delimiters between floors are dark
  for (int i = 1; i < strip.numPixels(); i += 6) {
    strip.setPixelColor(i-1, strip.Color(0, 0, 0));
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }

  if (isBusy) {
    strip.setPixelColor(0, strip.Color(50, 0, 0));
  } else {
    strip.setPixelColor(0, strip.Color(0, 50, 0));
  }

  strip.show();
}

void onPatternComplete() {
    
}

void deserializeJSON(const char* json) {
  const size_t capacity = 10*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(10) + 90;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, json);

  int status_code = doc["status"]["code"]; // 1
  const char* status_description = doc["status"]["description"]; // "ok"

  JsonObject colours = doc["colours"];

  JsonArray colours_0 = colours["0"];
  towerColours[0].r = colours_0[0];
  towerColours[0].g = colours_0[1];
  towerColours[0].b = colours_0[2];

  JsonArray colours_1 = colours["1"];
  towerColours[1].r = colours_1[0];
  towerColours[1].g = colours_1[1];
  towerColours[1].b = colours_1[2];

  JsonArray colours_2 = colours["2"];
  towerColours[2].r = colours_2[0];
  towerColours[2].g = colours_2[1];
  towerColours[2].b = colours_2[2];

  JsonArray colours_3 = colours["3"];
  towerColours[3].r = colours_3[0];
  towerColours[3].g = colours_3[1];
  towerColours[3].b = colours_3[2];

  JsonArray colours_4 = colours["4"];
  towerColours[4].r = colours_4[0];
  towerColours[4].g = colours_4[1];
  towerColours[4].b = colours_4[2];

  JsonArray colours_5 = colours["5"];
  towerColours[5].r = colours_5[0];
  towerColours[5].g = colours_5[1];
  towerColours[5].b = colours_5[2];

  JsonArray colours_6 = colours["6"];
  towerColours[6].r = colours_6[0];
  towerColours[6].g = colours_6[1];
  towerColours[6].b = colours_6[2];

  JsonArray colours_7 = colours["7"];
  towerColours[7].r = colours_7[0];
  towerColours[7].g = colours_7[1];
  towerColours[7].b = colours_7[2];

  JsonArray colours_8 = colours["8"];
  towerColours[8].r = colours_8[0];
  towerColours[8].g = colours_8[1];
  towerColours[8].b = colours_8[2];

  JsonArray colours_9 = colours["9"];
  towerColours[9].r = colours_9[0];
  towerColours[9].g = colours_9[1];
  towerColours[9].b = colours_9[2];

  isDay = doc["isDay"];
  isBusy = doc["isBusy"];
  bool inCall = doc["inCall"];
}

void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
}

void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);
}
