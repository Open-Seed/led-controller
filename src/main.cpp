#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "settings.h"

// ---- Managers ----
#include "managers/manager-led.h"

// ---- Services ----
#include "services/service-mqtt.h"
#include "services/service-server.h"

// ---- System ----
#include "system/system-preference.h"
#include "system/system-wifi.h"

WiFiClient espClient;
SystemWifi systemWifi;
ServerService *serverService;
SystemPreference *systemPreference;
MQTTService *mqttService;
LedManager *ledManager;

bool shouldReboot = false;

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.print(F("Running version "));
  Serial.println(FIRMWARE_VERSION);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  analogWriteRange(255);

  if (systemWifi.init())
  {
    ledManager = new LedManager();
    mqttService = new MQTTService();
    serverService = new ServerService();
    Serial.println(F("Open LED Controller Started"));
  }
  else
  {
    Serial.println(F("Open LED Controller failed to start"));
  }
}

void loop()
{

  if (shouldReboot)
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }

  ledManager->tick();

  mqttService->tick();
  serverService->tick();
}