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

void sendLedState()
{
    StaticJsonDocument<256> state = ledManager.getState();

    String stateJson;
    serializeJson(state, stateJson);
    char payload[stateJson.length() + 1];
    stateJson.toCharArray(payload, stateJson.length() + 1);
    Serial.print("Payload : ");
    Serial.println(payload);
    mqttClient.publish(CONFIG_MQTT_TOPIC_STATE, payload, true);
}

// Handles messages received in the mqttSubscribeTopic
void messageReceived(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.println("] ");

    if (strcmp(topic, CONFIG_MQTT_TOPIC_SET) == 0)
    {

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }

        String print;
        serializeJson(doc, print);
        Serial.println(print);

        ledManager.setState(doc);
    }

    sendLedState();
}

void connectClient()
{
    DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

    const char *mqttClientName = config["mqtt"]["name"] != nullptr ? config["mqtt"]["name"].as<const char *>() : BOARD_NAME;
    const char *mqttUsername = config["mqtt"]["username"].as<const char *>();
    const char *mqttPassword = config["mqtt"]["password"].as<const char *>();
    const char *mqttServer = config["mqtt"]["server"].as<const char *>();

    mqttClient.setServer(mqttServer, 1883);

    Serial.print(F(" - MQTT...."));
    if (mqttClient.connect(mqttClientName, mqttUsername, mqttPassword, CONFIG_MQTT_TOPIC_AVAILABILITY, 1, false, CONFIG_MQTT_PAYLOAD_ONLINE))
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
    sendLedState();
}

MQTTService::MQTTService()
{

    mqttClient.setCallback(messageReceived);
    connectClient();
}

void MQTTService::loop()
{

    if (!mqttClient.connected())
    {
        connectClient();
    }

    mqttClient.loop();
}
