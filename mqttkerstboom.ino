#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "FS.h"
#include <ESP8266WebServer.h>

int redPin = 14;
int greenPin = 12;
int bluePin = 13;
int apPin = 5;

const char *mqttHost = "10.0.10.103";
const int mqttPort = 1883;

// Default WiFi client settings
String ssidString = "Bhack";
String keyString = "!LhackerHacken!";

bool apMode = false;

WiFiClient espWifi;
PubSubClient mqtt(espWifi);
ESP8266WebServer http(80);

/*
 * Color functions
 */
void setRed(int level) {
  level = level > 255 ? 255 : (level < 0 ? 0 : level);
  analogWrite(redPin, level * 4);
}

void setGreen(int level) {
  level = level > 255 ? 255 : (level < 0 ? 0 : level);
  analogWrite(greenPin, level * 4);
}

void setBlue(int level) {
  level = level > 255 ? 255 : (level < 0 ? 0 : level);
  analogWrite(bluePin, level * 4);
}

void setColor(int red, int green, int blue) {
  setRed(red);
  setGreen(green);
  setBlue(blue);
}

/*
 * STA mode
 */
void handleMqtt(char *cstrTopic, byte *binPayload, unsigned int length) {
  // Create String from topic
  String topic(cstrTopic);
  
  // Create C string from payload
  char payload[length + 1];
  memcpy(payload, binPayload, length);

  // Create String from payload
  String strPayload(payload);
  
  if (topic.endsWith("/rgb")) {
    // Parse string into RGB
    char *parts[3];
    parts[0] = strtok(payload, ",");
    parts[1] = strtok(NULL, ",");
    parts[2] = strtok(NULL, ",");

    setColor(String(parts[0]).toInt(), String(parts[1]).toInt(), String(parts[2]).toInt());
  } else if (topic.endsWith("/r")) {
    setRed(strPayload.toInt());
  } else if (topic.endsWith("/g")) {
    setGreen(strPayload.toInt());
  } else if (topic.endsWith("/b")) {
    setBlue(strPayload.toInt());
  }
}

void setupSta() {
  // Get SSID and password  
  if (SPIFFS.exists("/ssid")) {
    File ssidFile = SPIFFS.open("/ssid", "r");
    ssidString = ssidFile.readString();
    ssidFile.close();

    File keyFile = SPIFFS.open("/key", "r");
    keyString = keyFile.readString();
    keyFile.close();
  }
  
  // Setup WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidString.c_str(), keyString.c_str());

  while (WiFi.status() != WL_CONNECTED)
    delay(250);

  // Setup MQTT
  mqtt.setServer(mqttHost, mqttPort);
  mqtt.setCallback(handleMqtt);
  mqtt.connect("kerstboom");

  // Subscribe to topics
  mqtt.subscribe("kerstboom/+");

  // We are ready
  setColor(0, 255, 0);
}

/*
 * AP mode
 */
void saveWifiCredentials(String ssid, String key) {
  SPIFFS.format();

  File ssidFile = SPIFFS.open("/ssid", "w");
  ssidFile.print(ssid);
  ssidFile.flush();
  ssidFile.close();

  File keyFile = SPIFFS.open("/key", "w");
  keyFile.print(key);
  keyFile.flush();
  keyFile.close();
}

void handleGet() {
  if (!http.hasArg("ssid") || !http.hasArg("key")) {
    http.send(400);
    return;
  }

  String ssid = http.arg("ssid");
  String key = http.arg("key");

  saveWifiCredentials(ssid, key);
  http.send(200);

  ESP.restart();
}

void setupAp() {
  apMode = true;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Kerstboom");

  http.on("/", handleGet);
  http.begin();
  
  // Set yellow to indicate ready to configure
  setColor(255, 255, 0);
}

/*
 * Setup and loop
 */
void setup() {  
  // Setup pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(apPin, INPUT_PULLUP);
  digitalWrite(apPin, HIGH);

  // Initial color red
  setColor(255, 0, 0);

  // Enable SPIFFS
  SPIFFS.begin();

  if (digitalRead(apPin) == LOW)
    setupAp();
  else
    setupSta();
}

void loop() {
  if (apMode)
    http.handleClient();
  else
    mqtt.loop();
}
