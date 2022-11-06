#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

class LedManager
{

public:
    LedManager();
    void setState(StaticJsonDocument<256> payload);
    StaticJsonDocument<256> getState();
    void loop();

private:
    void setColor(int inR, int inG, int inB, int inW);
};

#endif