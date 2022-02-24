#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <ESPAsyncWebServer.h>

#include "credentials.h"

AsyncWebServer server(80);

// -----------------------------------------------------------------------------

#define FIRST_CONTROLLER_PIN_GREEN 15
#define FIRST_CONTROLLER_PIN_RED 13
#define FIRST_CONTROLLER_PIN_BLUE 12

#define SECOND_CONTROLLER_PIN_GREEN 0
#define SECOND_CONTROLLER_PIN_RED 4
#define SECOND_CONTROLLER_PIN_BLUE 5

#define SERIAL_BAUDRATE     115200

IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* PARAM_MESSAGE = "window";

int actionToExecute = 0;

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  WiFi.config(ip, gateway, subnet);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}

void serverSetup() {

  // Custom entry point (not required by the library, here just as an example)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Hello, world");
  });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest * request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
      if (message == "FIRST") {
        Serial.println("FIRST");
        actionToExecute = 1;
      } else if (message == "SECOND") {
        Serial.println("SECOND");
        actionToExecute = 2;
      } else {
        message = "Unknown device";
      }
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, GET: " + message);
  });

  //    // These two callbacks are required for gen1 and gen3 compatibility
  //    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  //        if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
  //        // Handle any other body request here...
  //    });
  //    server.onNotFound([](AsyncWebServerRequest *request) {
  //        String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
  //        if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
  //        // Handle not found request here...
  //    });

  // Start the server
  server.begin();

}

void initVeluxPins() {
  digitalWrite(FIRST_CONTROLLER_PIN_GREEN, LOW);
  digitalWrite(FIRST_CONTROLLER_PIN_RED, LOW);
  digitalWrite(FIRST_CONTROLLER_PIN_BLUE, LOW);

  digitalWrite(SECOND_CONTROLLER_PIN_GREEN, LOW);
  digitalWrite(SECOND_CONTROLLER_PIN_RED, LOW);
  digitalWrite(SECOND_CONTROLLER_PIN_BLUE, LOW);

  // SETUP BUILT IN LED
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  Serial.println();

  // SETUP CONTROLLER PINS
  pinMode(FIRST_CONTROLLER_PIN_GREEN, OUTPUT);
  pinMode(FIRST_CONTROLLER_PIN_RED, OUTPUT);
  pinMode(FIRST_CONTROLLER_PIN_BLUE, OUTPUT);

  pinMode(SECOND_CONTROLLER_PIN_GREEN, OUTPUT);
  pinMode(SECOND_CONTROLLER_PIN_RED, OUTPUT);
  pinMode(SECOND_CONTROLLER_PIN_BLUE, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // INIT CONTROLLER PINS
  initVeluxPins();

  // Wifi
  wifiSetup();

  // Web server
  serverSetup();

}

void loop() {
  // This is a sample code to output free heap every 5 seconds
  // This is a cheap way to detect memory leaks
  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  }
}
