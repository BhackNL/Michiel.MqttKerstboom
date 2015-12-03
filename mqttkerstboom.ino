#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int redPin = 14;
int greenPin = 12;
int bluePin = 13;

const char *wifiSsid = "Bhack";
const char *wifiKey = "!LhackerHacken!";

const char *mqttHost = "10.0.10.103";
const int mqttPort = 1883;

WiFiClient espWifi;
PubSubClient mqtt(espWifi);

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


void handleMqtt(char* cstrTopic, byte* binPayload, unsigned int length) {
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

void setup() {  
  // Setup pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initial color red
  setColor(255, 0, 0);

  // Setup WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiKey);

  while (WiFi.status() != WL_CONNECTED)
    delay(250);

  // We are connected
  setColor(0, 255, 0);

  // Setup MQTT
  mqtt.setServer(mqttHost, mqttPort);
  mqtt.setCallback(handleMqtt);
  mqtt.connect("kerstboom");

  // Subscribe to topics
  mqtt.subscribe("kerstboom/+");
}

void loop() {
  mqtt.loop();
}
