#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "settings.h"
#include "system-wifi.h"
#include "system-preference.h"

extern SystemPreference systemPreference;

bool SystemWifi::init()
{

  DynamicJsonDocument config = systemPreference.getPreferences(PREFERENCE_SYSTEM_CONFIG);

  Serial.println(" - Initializing Wifi");

  if (config["wifi"]["ssid"] == nullptr)
  {

    Serial.println(" - WiFi Mode = AP");

    WiFi.mode(WIFI_AP);
    WiFi.hostname(config["board"]["name"] != nullptr ? config["board"]["name"].as<char *>() : BOARD_NAME);
    WiFi.softAP(config["wifi"]["name"] != nullptr ? config["wifi"]["name"].as<char *>() : WIFI_SSID_NAME);

    IPAddress IP = WiFi.softAPIP();
    Serial.print(" - AP IP address: ");
    Serial.println(IP);

    return true;
  }
  else
  {

    Serial.println(" - WiFi Mode = STA");

    WiFi.mode(WIFI_STA);
    WiFi.hostname(config["board"]["name"] != nullptr ? config["board"]["name"].as<char *>() : BOARD_NAME);
    WiFi.begin(config["wifi"]["ssid"].as<char *>(), config["wifi"]["password"].as<char *>());

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.printf("WiFi Failed!\n");
      return false;
    }

    MDNS.begin(config["board"]["dns"] != nullptr ? config["board"]["dns"].as<char *>() : BOARD_DNS);

    Serial.print(" - ");
    Serial.println(WiFi.localIP());
    return WiFi.status() == WL_CONNECTED;
  }
}

bool SystemWifi::isAP()
{
  return WiFi.status() == WL_CONNECTED; // return the WiFi connection status
}

bool SystemWifi::isConnected()
{
  return WiFi.status() == WL_CONNECTED; // return the WiFi connection status
}