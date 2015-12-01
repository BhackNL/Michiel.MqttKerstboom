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

void setColor(int red, int green, int blue) {
  // Clamp values
  red = red > 255 ? 255 : (red < 0 ? 0 : red);
  green = green > 255 ? 255 : (green < 0 ? 0 : green);
  blue = blue > 255 ? 255 : (blue < 0 ? 0 : blue);

  // Set color values
  analogWrite(redPin, red * 4);
  analogWrite(greenPin, green * 4);
  analogWrite(bluePin, blue * 4);
}

void handleMqtt(char* topic, byte* binPayload, unsigned int length) {
  // Create C string from payload
  char payload[length + 1];
  memcpy(payload, binPayload, length);

  // Parse string into RGB
  char *parts[3];
  parts[0] = strtok(payload, ",");
  parts[1] = strtok(NULL, ",");
  parts[2] = strtok(NULL, ",");

  setColor(String(parts[0]).toInt(), String(parts[1]).toInt(), String(parts[2]).toInt());
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

  // Subscribe and publish
  mqtt.subscribe("kerstboom/rgb");
}

void loop() {
  mqtt.loop();
}
