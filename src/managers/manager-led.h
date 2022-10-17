#include <Arduino.h>

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

class LedManager
{

public:
    LedManager();
    void processJson(String &payload);
    void setState();
    String getState();
    void tick();

private:
};

#endif