#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "settings.h"
#include "service-server.h"
#include "../managers/manager-led.h"
#include "../system/system-preference.h"

extern SystemPreference systemPreference;
extern LedManager ledManager;
extern bool shouldReboot;

ESP8266WebServer server(HTTP_REST_PORT);

void api_get_root()
{
  server.send(200, "text/plain", "Welcome to Open LED");
}

void api_get_system_info()
{
  StaticJsonDocument<200> doc;
  doc["version"] = FIRMWARE_VERSION;

  // Board Info
  doc["board"]["ChipId"] = ESP.getChipId();
  doc["board"]["CoreVersion"] = ESP.getCoreVersion();
  doc["board"]["CpuFreqMHz"] = ESP.getCpuFreqMHz();
  doc["board"]["CycleCount"] = ESP.getCycleCount();
  doc["board"]["Vcc"] = ESP.getVcc();
  doc["board"]["FreeHeap"] = ESP.getFreeHeap();
  doc["sketch"]["md5"] = ESP.getSketchMD5();
  doc["sketch"]["size"] = ESP.getSketchSize();
  doc["sketch"]["freeSpace"] = ESP.getFreeSketchSpace();

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void api_get_preferences()
{
  StaticJsonDocument<1000> doc = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void api_post_preferences()
{

  String _body = server.arg(0);
  if (_body == NULL)
  {                                                         // If the POST request doesn't have data
    server.send(400, "text/plain", "400: Invalid Request"); // The request is invalid, so send HTTP status 400
    return;
  }
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, _body);
  // Test if parsing succeeds.
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    server.send(500, "text/plain", "Could not deserialize JSON");
    return;
  }

  systemPreference.setPreferences(PREFERENCE_SYSTEM_CONFIG, doc);

  server.send(200, "text/plain", "Restarting..");

  // Restart Esp for changes to take affect
  shouldReboot = true;
}

void api_get_status()
{
  String status = ledManager.getState();
  server.send(200, "application/json", status);
}

void api_not_found()
{
  server.send(404, "text/plain", "API Not Found");
}

ServerService::ServerService()
{
  Serial.println(" - Initializing Server Service");
  server.on("/", HTTP_GET, api_get_root);
  server.on("/api/status", HTTP_GET, api_get_status);
  server.on("/api/system/preference", HTTP_GET, api_get_preferences);
  server.on("/api/system/preference", HTTP_POST, api_post_preferences);
  server.on("/api/system/info", HTTP_GET, api_get_system_info);
  server.onNotFound(api_not_found);
  server.begin();
}

void ServerService::tick()
{
  server.handleClient();
}
