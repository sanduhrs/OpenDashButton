/**
 *  OpenDashButton on ESP8266, Arduino (IoT Wi-Fi Button)
 */

#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>

#include "credentials.h"

// System details.
const char* name = "OpenDashButton";
const char* version = "0.0.0-dev";
const char* description = "The Open Dash Button on ESP8266, Arduino";

// Unique identifier.
char uuid[] = UUID;

// Wi-Fi credentials.
char ssid[] = SSID;
char password[] = PASS;

// Webhook details.
char server[] = SERVER;
int port = PORT;
char secret[] = SECRET;
char signature[] = SIGNATURE;
char resource[] = RESOURCE;

// GPIO2 of ESP8266.
int ledPin = 2;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);

  initWifi();
  makeRequest();

  digitalWrite(ledPin, HIGH);

  // Deep sleep mode until RESET pin is connected to a LOW signal (pushbutton is pressed).
  ESP.deepSleep(0);
}

void loop() {
  // Sleeping so wont get here.
}

// Establish a Wi-Fi connection.
void initWifi() {
  Serial.print("Connecting to: ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  int timeout = 10 * 4; // 10 seconds
  while (WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
  blink(2);
}

// Make an HTTP request to the web service
void makeRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient wifi;
  HttpClient client = HttpClient(wifi, server, port);

  int retries = 5;
  while (!!!wifi.connect(server, port) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!wifi.connected()) {
    Serial.println("Failed to connect, going back to sleep");
    blink(5);
    return;
  }

  Serial.print("Request resource: ");
  Serial.println(resource);

  JSONVar payload;
  payload["name"] = name;
  payload["version"] = version;
  payload["description"] = description;
  payload["uuid"] = uuid;
  String json = JSON.stringify(payload);

  client.beginRequest();
  client.post(resource);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", json.length());
  client.sendHeader("X-Hub-Signature", signature);
  client.beginBody();
  client.print(json);
  client.endRequest();

  int timeout = 5 * 10; // 5 seconds
  while (!!!wifi.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!wifi.available()) {
    Serial.println("No response, going back to sleep");
    blink(5);
    return;
  }
  while (wifi.available()) {
    Serial.write(wifi.read());
  }

  Serial.println("\nclosing connection");
  client.stop();
  blink(2);
}

void blink(int count) {
  pinMode(ledPin, OUTPUT);
  while (count-- > 0) {
    digitalWrite(ledPin, HIGH);
    delay(250);
    digitalWrite(ledPin, LOW);
    delay(250);
  }
}
