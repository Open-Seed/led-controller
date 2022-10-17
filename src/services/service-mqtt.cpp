#include <Arduino.h>
#include <ArduinoJson.h>
#include "settings.h"

#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include "service-mqtt.h"
#include "../managers/manager-led.h"
#include "../system/system-preference.h"

extern SystemPreference systemPreference;
extern LedManager ledManager;
extern WiFiClient espClient;

MQTTClient mqtt;

bool mqttConnect()
{
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    const char *mqttClientName = config["mqtt"]["name"] != nullptr ? config["mqtt"]["name"].as<const char *>() : BOARD_NAME;
    const char *mqttUsername = config["mqtt"]["username"].as<const char *>();
    const char *mqttPassword = config["mqtt"]["password"].as<const char *>();
    const char *mqttServer = config["mqtt"]["server"].as<const char *>();

    Serial.print(F(" - MQTT...."));
    if (mqtt.connect(mqttClientName, mqttUsername, mqttPassword))
    {
        Serial.print(F("connected: "));
        Serial.println(mqttServer);
    }
    else
    {
        Serial.print(F("connection error: "));
        Serial.println(mqttServer);
    }
    return mqtt.connected();
}

// Handles messages received in the mqttSubscribeTopic
void messageReceived(String &topic, String &payload)
{

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (topic == CONFIG_MQTT_TOPIC_SET)
    {
        ledManager.processJson(payload);
        ledManager.setState();
    }

    String state = ledManager.getState();
    mqtt.publish(CONFIG_MQTT_TOPIC_STATE, state, true, 0);
}

MQTTService::MQTTService()
{
    Serial.println(" - Initializing MQTT Service");
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    this->enabled = config["mqtt"]["enabled"];
    if (this->enabled)
    {
        const char *mqttServer = config["mqtt"]["server"].as<const char *>();
        int mqttPort = config["mqtt"]["port"].as<int>();

        mqtt.begin(mqttServer, mqttPort, espClient);
        mqtt.onMessage(messageReceived);
        mqtt.subscribe(CONFIG_MQTT_TOPIC_SET);
        mqtt.subscribe(CONFIG_MQTT_TOPIC_STATE);
    }
}

void MQTTService::tick()
{
    if (this->enabled)
    {
        mqtt.loop();
    }
}
