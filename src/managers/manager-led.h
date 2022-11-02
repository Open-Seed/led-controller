#include <Arduino.h>

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

class LedManager
{

public:
    LedManager();
    void setState(String &payload);
    String getState();
    void tick();

private:
    void setColor(int inR, int inG, int inB, int inW);
};

#endif