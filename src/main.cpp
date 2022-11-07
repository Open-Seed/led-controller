#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoHA.h>

#include "settings.h"

// ---- Managers ----
#include "managers/manager-led.h"

// ---- Services ----
#include "services/service-home-assistant.h"
#include "services/service-mqtt.h"
#include "services/service-server.h"

// ---- System ----
#include "system/system-preference.h"
#include "system/system-wifi.h"

WiFiClient espClient;
SystemWifi systemWifi;

SystemPreference *systemPreference;

ServerService *serverService;
HomeAssistantService *homeAssistantService;
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
  analogWriteRange(255);

  systemPreference = new SystemPreference();
  ledManager = new LedManager();

  if (systemWifi.init())
  {
    // Both use mqtt protocol, so can only enable one at a stage
    if (ENABLED_HOME_ASSISTANT)
    {
      homeAssistantService = new HomeAssistantService();
    }
    else if (ENABLED_MQTT)
    {
      mqttService = new MQTTService();
    }

    serverService = new ServerService();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println(F("Open LED Controller Started"));
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
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
    return;
  }

  if (ENABLED_HOME_ASSISTANT)
  {
    homeAssistantService->loop();
  }
  else if (ENABLED_MQTT)
  {
    mqttService->loop();
  }

  ledManager->loop();
  serverService->loop();
}