#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "service-mqtt.h"
#include "../managers/manager-led.h"
#include "../system/system-preference.h"
#include "settings.h"

extern SystemPreference systemPreference;
extern LedManager ledManager;
extern WiFiClient espClient;

PubSubClient mqttClient(espClient);

// Handles messages received in the mqttSubscribeTopic
void messageReceived(char *topic, byte *payload, unsigned int length)
{

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (topic == CONFIG_MQTT_TOPIC_SET)
    {
        String payloadString = String((char *)payload);

        ledManager.processJson(payloadString);
        ledManager.setState();
    }

    String state = ledManager.getState();
}

MQTTService::MQTTService()
{
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    this->enabled = config["mqtt"]["enabled"];
    if (this->enabled)
    {

        const char *mqttClientName = config["mqtt"]["name"] != nullptr ? config["mqtt"]["name"].as<const char *>() : BOARD_NAME;
        const char *mqttUsername = config["mqtt"]["username"].as<const char *>();
        const char *mqttPassword = config["mqtt"]["password"].as<const char *>();
        const char *mqttServer = config["mqtt"]["server"].as<const char *>();

        mqttClient.setServer(mqttServer, 1883);
        mqttClient.setCallback(messageReceived);

        Serial.print(F(" - MQTT...."));
        if (mqttClient.connect(mqttClientName, mqttUsername, mqttPassword))
        {
            Serial.print(F("connected: "));
            Serial.println(mqttServer);
        }
        else
        {
            Serial.print(F("connection error: "));
            Serial.println(mqttServer);
        }

        mqttClient.subscribe(CONFIG_MQTT_TOPIC_SET);
        mqttClient.subscribe(CONFIG_MQTT_TOPIC_STATE);
    }
}

void MQTTService::tick()
{
    if (this->enabled)
    {
        mqttClient.loop();
    }
}
