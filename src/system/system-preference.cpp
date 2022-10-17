#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
#include "system-preference.h"
#include "settings.h"

SystemPreference::SystemPreference()
{

  Serial.println(" - Initializing System Preferences");
  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
}

DynamicJsonDocument SystemPreference::getPreferences(const char *name)
{

  DynamicJsonDocument doc(1024);
  File configFile = LittleFS.open(name, "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    clearPreferences(name);
    configFile = LittleFS.open(name, "r");
  }

  DeserializationError error = deserializeJson(doc, configFile);
  // Test if parsing succeeds.
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  return doc;
}

void SystemPreference::setPreferences(const char *name, DynamicJsonDocument doc)
{
  File configFile = LittleFS.open(name, "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  serializeJson(doc, configFile);
}

void SystemPreference::clearPreferences(const char *name)
{

  File configFile = LittleFS.open(name, "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  StaticJsonDocument<200> doc;
  serializeJson(doc, configFile);
}