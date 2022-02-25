#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#include "ota.h"
#include "credentials.h"

AsyncWebServer server(80);

// -----------------------------------------------------------------------------

#define FIRST_CONTROLLER_PIN_UP 15
#define FIRST_CONTROLLER_PIN_STOP 13
#define FIRST_CONTROLLER_PIN_DOWN 12

#define SECOND_CONTROLLER_PIN_UP 0
#define SECOND_CONTROLLER_PIN_STOP 4
#define SECOND_CONTROLLER_PIN_DOWN 5

#define SERIAL_BAUDRATE 115200

IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char *PARAM_MESSAGE = "window";

int window = -1; // FIRST == 1; SECOND == 2
int action = -1; // UP == 1; DOWN == 2; STOP == 3

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup()
{

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  WiFi.config(ip, gateway, subnet);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

  // Setup Firmware update over the air (OTA)
  setup_OTA();
}

void serverSetup()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hello, world"); });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_MESSAGE)) {
                message = request->getParam(PARAM_MESSAGE)->value();
                if (message == "FIRST") {
                  Serial.println("FIRST");
                  window = 1;
                } else if (message == "SECOND") {
                  Serial.println("SECOND");
                  window = 2;
                } else {
                  message = "Unknown device";
                  window = 0;
                }

                action = 1;
              } else {
                message = "No message sent";
                window = 0;
              }
              request->send(200, "text/plain", "Hello, GET: " + message); });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_MESSAGE)) {
                message = request->getParam(PARAM_MESSAGE)->value();
                if (message == "FIRST") {
                  Serial.println("FIRST");
                  window = 1;
                } else if (message == "SECOND") {
                  Serial.println("SECOND");
                  window = 2;
                } else {
                  message = "Unknown device";
                  window = 0;
                }

                action = 2;
              } else {
                message = "No message sent";
                window = 0;
              }
              request->send(200, "text/plain", "Hello, GET: " + message); });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message;
              if (request->hasParam(PARAM_MESSAGE)) {
                message = request->getParam(PARAM_MESSAGE)->value();
                if (message == "FIRST") {
                  Serial.println("FIRST");
                  window = 1;
                } else if (message == "SECOND") {
                  Serial.println("SECOND");
                  window = 2;
                } else {
                  message = "Unknown device";
                  window = 0;
                }

                action = 3;
              } else {
                message = "No message sent";
                window = 0;
              }
              request->send(200, "text/plain", "Hello, GET: " + message); });
              
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

  // Start the server
  server.begin();
}

void initVeluxPins()
{
  digitalWrite(FIRST_CONTROLLER_PIN_UP, LOW);
  digitalWrite(FIRST_CONTROLLER_PIN_STOP, LOW);
  digitalWrite(FIRST_CONTROLLER_PIN_DOWN, LOW);

  digitalWrite(SECOND_CONTROLLER_PIN_UP, LOW);
  digitalWrite(SECOND_CONTROLLER_PIN_STOP, LOW);
  digitalWrite(SECOND_CONTROLLER_PIN_DOWN, LOW);

  // SETUP BUILT IN LED
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  Serial.println();

  // SETUP CONTROLLER PINS
  pinMode(FIRST_CONTROLLER_PIN_UP, OUTPUT);
  pinMode(FIRST_CONTROLLER_PIN_STOP, OUTPUT);
  pinMode(FIRST_CONTROLLER_PIN_DOWN, OUTPUT);

  pinMode(SECOND_CONTROLLER_PIN_UP, OUTPUT);
  pinMode(SECOND_CONTROLLER_PIN_STOP, OUTPUT);
  pinMode(SECOND_CONTROLLER_PIN_DOWN, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // INIT CONTROLLER PINS
  initVeluxPins();

  // Wifi
  wifiSetup();

  // Web server
  serverSetup();
}

void processRequest()
{
  if (window == 0)
  {
    Serial.println("INIT");
    window = -1;
    action = -1;
    initVeluxPins();
  }

  if (window == 1)
  {
    Serial.print("FIRST --> ");
    Serial.println(action);
    window = 0;

    switch (action)
    {
    case 1:
      digitalWrite(FIRST_CONTROLLER_PIN_UP, HIGH);
      break;

    case 2:
      digitalWrite(FIRST_CONTROLLER_PIN_DOWN, HIGH);
      break;

    default:
      digitalWrite(FIRST_CONTROLLER_PIN_STOP, HIGH);
      break;
    }

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }

  if (window == 2)
  {
    Serial.print("SECOND --> ");
    Serial.println(action);
    window = 0;

    switch (action)
    {
    case 1:
      digitalWrite(SECOND_CONTROLLER_PIN_UP, HIGH);
      break;

    case 2:
      digitalWrite(SECOND_CONTROLLER_PIN_DOWN, HIGH);
      break;

    default:
      digitalWrite(SECOND_CONTROLLER_PIN_STOP, HIGH);
      break;
    }

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

void loop()
{
  // Check for OTA updates
  ArduinoOTA.handle();

  processRequest();
}
