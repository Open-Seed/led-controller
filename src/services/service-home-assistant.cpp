
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

HADevice device(HA_UNIQUE_NAME);
HAMqtt haMqtt(espClient, device);
HALight light(HA_UNIQUE_LIGHT_NAME, HALight::BrightnessFeature | HALight::RGBFeature);

void onStateCommand(bool state, HALight *sender)
{
    StaticJsonDocument<256> doc;
    doc["state"] = state ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
    ledManager.setState(doc);

    sender->setState(state);
}

void onBrightnessCommand(uint8_t brightness, HALight *sender)
{
    StaticJsonDocument<256> doc;
    doc["brightness"] = brightness;
    ledManager.setState(doc);
    sender->setBrightness(brightness);
}

void onRGBColorCommand(HALight::RGBColor color, HALight *sender)
{
    StaticJsonDocument<256> doc;
    doc["color"]["r"] = color.red;
    doc["color"]["g"] = color.green;
    doc["color"]["b"] = color.blue;
    ledManager.setState(doc);

    sender->setRGBColor(color);
}

HomeAssistantService::HomeAssistantService()
{
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    this->mqttClientName = config["mqtt"]["name"] != nullptr ? config["mqtt"]["name"].as<String>() : HA_UNIQUE_NAME;
    this->mqttUsername = config["mqtt"]["username"].as<String>();
    this->mqttPassword = config["mqtt"]["password"].as<String>();
    this->mqttServer = config["mqtt"]["server"].as<String>();

    // set device's details (optional)
    device.setName(this->mqttClientName.c_str());
    device.setSoftwareVersion(FIRMWARE_VERSION);
    device.setManufacturer("Open Seed");
    device.setModel("Led Controller");
    device.enableSharedAvailability();
    device.enableLastWill();

    // configure light (optional)
    light.setName("Light");

    // handle light states
    light.onStateCommand(onStateCommand);
    light.onBrightnessCommand(onBrightnessCommand);
    light.onRGBColorCommand(onRGBColorCommand);

    haMqtt.begin(this->mqttServer.c_str(), this->mqttUsername.c_str(), this->mqttPassword.c_str());

    if (haMqtt.isConnected())
    {
        Serial.print("Connected to home assistant : ");
        Serial.println(mqttServer);
    }
    else
    {
        Serial.println("Failed to connected to home assistant");
    }
}

void HomeAssistantService::loop()
{
    haMqtt.loop();
}
