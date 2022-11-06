
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoHA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "service-home-assistant.h"
#include "../managers/manager-led.h"
#include "../system/system-preference.h"
#include "settings.h"

extern SystemPreference systemPreference;
extern LedManager ledManager;
extern WiFiClient espClient;

HADevice device;
HAMqtt mqtt(espClient, device);

HALight light(HA_UNIQUE_NAME, HALight::BrightnessFeature | HALight::RGBFeature);

void onStateCommand(bool state, HALight *sender)
{
    StaticJsonDocument<256> doc;
    doc["state"] = state ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
    ledManager.setState(doc);

    Serial.print("State: ");
    Serial.println(state);

    sender->setState(state); // report state back to the Home Assistant
}

void onBrightnessCommand(uint8_t brightness, HALight *sender)
{
    Serial.print("Brightness: ");
    Serial.println(brightness);

    StaticJsonDocument<256> doc;
    doc["brightness"] = brightness;
    ledManager.setState(doc);

    sender->setBrightness(brightness); // report brightness back to the Home Assistant
}

void onRGBColorCommand(HALight::RGBColor color, HALight *sender)
{
    StaticJsonDocument<256> doc;
    doc["color"]["r"] = color.red;
    doc["color"]["g"] = color.red;
    doc["color"]["b"] = color.red;
    ledManager.setState(doc);

    Serial.print("Red: ");
    Serial.println(color.red);
    Serial.print("Green: ");
    Serial.println(color.green);
    Serial.print("Blue: ");
    Serial.println(color.blue);

    sender->setRGBColor(color); // report color back to the Home Assistant
}

HomeAssistantService::HomeAssistantService()
{
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    const char *clientName = config["mqtt"]["name"] != nullptr ? config["mqtt"]["name"].as<const char *>() : HA_UNIQUE_NAME;
    const char *username = config["mqtt"]["username"].as<const char *>();
    const char *password = config["mqtt"]["password"].as<const char *>();
    const char *serverIp = config["mqtt"]["server"].as<const char *>();

    String mac = WiFi.macAddress();
    int macSize = sizeof(mac) + 1;
    byte macAddress[macSize];
    mac.getBytes(macAddress, macSize);

    // set device's details (optional)
    device.setName(clientName);
    device.setSoftwareVersion(FIRMWARE_VERSION);
    device.setModel("ESP8266");
    device.setUniqueId(macAddress, macSize);

    // configure light (optional)
    light.setName(clientName);

    // handle light states
    light.onStateCommand(onStateCommand);
    light.onBrightnessCommand(onBrightnessCommand);
    light.onRGBColorCommand(onRGBColorCommand);

    mqtt.begin(serverIp, username, password);
}

void HomeAssistantService::loop()
{
    mqtt.loop();
}
