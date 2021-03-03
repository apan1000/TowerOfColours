#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

#include "NeoPatterns.h"
#include "arduino_secrets.h"

#define PIN        7
#define NUMPIXELS 60

// 0 (min) to 255 (max)
#define BRIGHTNESS_DAY 12
#define BRIGHTNESS_NIGHT 15

#define RAINBOW_SPEED 400

#define RESPONSE_STATUS_CODE_ERROR_INDEX 0
#define WIFI_ERROR_INDEX 2
#define START_ERROR_INDEX (NUMPIXELS / 2)

void onPatternComplete();
NeoPatterns strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800, &onPatternComplete);

///////please enter your sensitive data in arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
char server[] = SERVER; // Server address

int status = WL_IDLE_STATUS; // the Wifi radio's status

unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
const unsigned long getIntervalNight = 1000;
const unsigned long getIntervalDay = 20L * 60L * 1000L;

WiFiClient wifiClient;

int responseStatusCode = 200;

String response;
bool hasNewResponse = false;

struct RGB {
  byte r;
  byte g;
  byte b;
};

StaticJsonDocument<1024> doc;
RGB towerColours[10];
bool isDay = false;
bool isBusy = false;
int statusCode = 1;

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS_NIGHT);
  strip.RainbowCycle(RAINBOW_SPEED);

  Serial.begin(9600);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    showErrorColors(START_ERROR_INDEX);
    // don't continue
    while (true);
  }

  connectToWifi();
}

void loop() {
  checkWiFi();

  while (wifiClient.available()) {
    hasNewResponse = true;
    char c = wifiClient.read();
    response += c;
  }

  if (hasNewResponse) {
    handleResponse();
  }

  const unsigned long now = millis();
  unsigned long interval;
  if (now >= lastConnectionTime) {
    interval = now - lastConnectionTime;
  } else {
    interval = now;
  }

  if (isDay) {
    strip.setBrightness(BRIGHTNESS_DAY);
    strip.Update();
    if (interval > getIntervalDay) {
      getColours();
    }
  } else {
    setStripColors();
    if (interval > getIntervalNight) {
      strip.setBrightness(BRIGHTNESS_NIGHT);
      getColours();
    }
  }

  if (responseStatusCode != 200) {
    showErrorColors(RESPONSE_STATUS_CODE_ERROR_INDEX);
  }
}

void getColours() {
  wifiClient.stop();
  response = "";

  if (wifiClient.connect(server, 80)) {
    char hostHeader[40];
    sprintf(hostHeader, "Host: %s", server);

    wifiClient.println("GET /cbn-live/getColours HTTP/1.1");
    wifiClient.println(hostHeader);
    wifiClient.println("Connection: close");
    wifiClient.println();
  }
  lastConnectionTime = millis();
}

void handleResponse() {
  responseStatusCode = getStatusCode(response);
  if (responseStatusCode == 200) {
    auto body = getResponseBody(response);
    if (strlen(body) != 0) {
      deserializeJSON(body);
    }
  }
  hasNewResponse = false;
}

int getStatusCode(const String& response) {
  int statusCode = 0;
  if (response.length() > 12) {
    statusCode = response.substring(9,12).toInt();
  }
  return statusCode;
}

const char* getResponseBody(const String& response) {
  int index = response.indexOf("\r\n\r\n");
  if (index != -1) {
    index += 4;
    auto body = response.substring(index);
    return body.c_str();
  }
  return "";
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

  if (statusCode != 1) {
    strip.setPixelColor(0, strip.Color(200, 160, 0));
  }

  strip.show();
}

void showErrorColors(int startIndex) {
  for (int i = startIndex; i < strip.numPixels(); i += 2) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
}

void onPatternComplete() {
    
}

void checkWiFi() {
  status = WiFi.status();
  if (status == WL_CONNECTION_LOST || status == WL_DISCONNECTED) {
    showErrorColors(WIFI_ERROR_INDEX);
    connectToWifi();
  }
}

void connectToWifi() {
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
}

void deserializeJSON(const char* json) {
  DeserializationError err = deserializeJson(doc, json);

  if (!err) {
    statusCode = doc["status"]["code"]; // 1
    const char* statusDescription = doc["status"]["description"]; // "ok"
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
    statusCode = -1;
  }

  if (statusCode == 1) {
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
}
