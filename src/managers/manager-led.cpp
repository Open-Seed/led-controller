#include "manager-led.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "settings.h"
#include "util/util-step.h"
#include "../system/system-preference.h"

extern SystemPreference systemPreference;

const bool rgb = (CONFIG_STRIP == RGB) || (CONFIG_STRIP == RGBW);
const bool includeWhite = (CONFIG_STRIP == BRIGHTNESS) || (CONFIG_STRIP == RGBW);

// Maintained state for reporting to HA
byte red = 255;
byte green = 255;
byte blue = 255;
byte white = 255;
byte brightness = 255;

// Real values to write to the LEDs (ex. including brightness and state)
byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;
byte realWhite = 0;

bool stateOn = false;

// Globals for fade/transitions
bool startFade = false;
unsigned long lastLoop = 0;
unsigned long transitionTime = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB, stepW;
int redVal, grnVal, bluVal, whtVal;

// Globals for flash
bool flash = false;
bool startFlash = false;
unsigned long flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = red;
byte flashGreen = green;
byte flashBlue = blue;
byte flashWhite = white;
byte flashBrightness = brightness;

// Globals for colorFade
bool colorFade = false;
int currentColor = 0;

// {red, grn, blu, wht}
const byte colors[][4] = {
    {255, 0, 0, 0},
    {0, 255, 0, 0},
    {0, 0, 255, 0},
    {255, 80, 0, 0},
    {163, 0, 255, 0},
    {0, 255, 255, 0},
    {255, 255, 0, 0}};

const int numColors = 7;

LedManager::LedManager()
{

    if (rgb)
    {
        pinMode(CONFIG_PIN_RED, OUTPUT);
        pinMode(CONFIG_PIN_GREEN, OUTPUT);
        pinMode(CONFIG_PIN_BLUE, OUTPUT);
    }
    if (includeWhite)
    {
        pinMode(CONFIG_PIN_WHITE, OUTPUT);
    }

    DynamicJsonDocument systemState = systemPreference.getPreferences(PREFERENCE_SYSTEM_STATE);
    this->setState(systemState);
}

void LedManager::setState(StaticJsonDocument<256> payload)
{
    if (payload["state"] != nullptr)
    {
        if (strcmp(payload["state"], CONFIG_MQTT_PAYLOAD_ON) == 0)
        {
            stateOn = true;
        }
        else if (strcmp(payload["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0)
        {
            stateOn = false;
        }
    }

    // If "flash" is included, treat RGB and brightness differently
    if (payload["flash"] != nullptr || (payload["effect"] != nullptr && strcmp(payload["effect"], "flash") == 0))
    {
        if (payload["flash"] != nullptr)
        {
            flashLength = (long)payload["flash"] * 1000;
        }
        else
        {
            flashLength = CONFIG_DEFAULT_FLASH_LENGTH * 1000;
        }

        if (payload["brightness"] != nullptr)
        {
            flashBrightness = payload["brightness"];
        }
        else
        {
            flashBrightness = brightness;
        }

        if (rgb && payload["color"] != nullptr)
        {
            flashRed = payload["color"]["r"];
            flashGreen = payload["color"]["g"];
            flashBlue = payload["color"]["b"];
        }
        else
        {
            flashRed = red;
            flashGreen = green;
            flashBlue = blue;
        }

        if (includeWhite && payload["white_value"] != nullptr)
        {
            flashWhite = payload["white_value"];
        }
        else
        {
            flashWhite = white;
        }

        flashRed = map(flashRed, 0, 255, 0, flashBrightness);
        flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
        flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);
        flashWhite = map(flashWhite, 0, 255, 0, flashBrightness);

        flash = true;
        startFlash = true;
    }
    else if (rgb && payload["effect"] != nullptr && (strcmp(payload["effect"], "color_fade_slow") == 0 || strcmp(payload["effect"], "color_fade_fast") == 0))
    {
        flash = false;
        colorFade = true;
        currentColor = 0;
        if (strcmp(payload["effect"], "color_fade_slow") == 0)
        {
            transitionTime = CONFIG_COLOR_FADE_TIME_SLOW;
        }
        else
        {
            transitionTime = CONFIG_COLOR_FADE_TIME_FAST;
        }
    }
    else if (colorFade && payload["color"] == nullptr && payload["brightness"] != nullptr)
    {
        // Adjust brightness during colorFade
        // (will be applied when fading to the next color)
        brightness = payload["brightness"];
    }
    else
    {
        // No effect
        flash = false;
        colorFade = false;

        if (rgb && payload["color"] != nullptr)
        {
            red = payload["color"]["r"];
            green = payload["color"]["g"];
            blue = payload["color"]["b"];
        }

        if (includeWhite && payload["white_value"] != nullptr)
        {
            white = payload["white_value"];
        }

        if (payload["brightness"] != nullptr)
        {
            brightness = payload["brightness"];
        }

        if (payload["transition"] != nullptr)
        {
            transitionTime = payload["transition"];
        }
        else
        {
            transitionTime = CONFIG_DEFAULT_TRANSITION_TIME;
        }
    }

    if (stateOn)
    {
        // Update lights
        realRed = map(red, 0, 255, 0, brightness);
        realGreen = map(green, 0, 255, 0, brightness);
        realBlue = map(blue, 0, 255, 0, brightness);
        realWhite = map(white, 0, 255, 0, brightness);
    }
    else
    {
        realRed = 0;
        realGreen = 0;
        realBlue = 0;
        realWhite = 0;
    }

    startFade = true;
    inFade = false; // Kill the current fade

    StaticJsonDocument<256> fullState = this->getState();
    // Persist the LED state to reinitialize on a power cycle
    systemPreference.setPreferences(PREFERENCE_SYSTEM_STATE, fullState);
}

StaticJsonDocument<256> LedManager::getState()
{
    StaticJsonDocument<256> doc;

    doc["state"] = (stateOn) ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
    if (rgb)
    {
        doc["color"]["r"] = red;
        doc["color"]["g"] = green;
        doc["color"]["b"] = blue;
    }

    doc["brightness"] = brightness;
    doc["color_mode"] = rgb ? "rgb" : "brightness";
    if (includeWhite)
    {
        doc["white_value"] = white;
    }

    if (rgb && colorFade)
    {
        if (transitionTime == CONFIG_COLOR_FADE_TIME_SLOW)
        {
            doc["effect"] = "color_fade_slow";
        }
        else
        {
            doc["effect"] = "color_fade_fast";
        }
    }
    else
    {
        doc["effect"] = "null";
    }

    return doc;
}

void LedManager::setColor(int inR, int inG, int inB, int inW)
{
    if (rgb)
    {
        analogWrite(CONFIG_PIN_RED, inR);
        analogWrite(CONFIG_PIN_GREEN, inG);
        analogWrite(CONFIG_PIN_BLUE, inB);
    }

    if (includeWhite)
    {
        analogWrite(CONFIG_PIN_WHITE, inW);
    }
}

void LedManager::loop()
{

    if (flash)
    {
        if (startFlash)
        {
            startFlash = false;
            flashStartTime = millis();
        }

        if ((millis() - flashStartTime) <= flashLength)
        {
            if ((millis() - flashStartTime) % 1000 <= 500)
            {
                this->setColor(flashRed, flashGreen, flashBlue, flashWhite);
            }
            else
            {
                this->setColor(0, 0, 0, 0);
            }
        }
        else
        {
            flash = false;
            this->setColor(realRed, realGreen, realBlue, realWhite);
        }
    }
    else if (rgb && colorFade && !inFade)
    {
        realRed = map(colors[currentColor][0], 0, 255, 0, brightness);
        realGreen = map(colors[currentColor][1], 0, 255, 0, brightness);
        realBlue = map(colors[currentColor][2], 0, 255, 0, brightness);
        realWhite = map(colors[currentColor][3], 0, 255, 0, brightness);
        currentColor = (currentColor + 1) % numColors;
        startFade = true;
    }

    if (startFade)
    {
        // If we don't want to fade, skip it.
        if (transitionTime == 0)
        {
            this->setColor(realRed, realGreen, realBlue, realWhite);

            redVal = realRed;
            grnVal = realGreen;
            bluVal = realBlue;
            whtVal = realWhite;

            startFade = false;
        }
        else
        {
            loopCount = 0;
            stepR = UtilStep::calculateStep(redVal, realRed);
            stepG = UtilStep::calculateStep(grnVal, realGreen);
            stepB = UtilStep::calculateStep(bluVal, realBlue);
            stepW = UtilStep::calculateStep(whtVal, realWhite);

            inFade = true;
        }
    }

    if (inFade)
    {
        startFade = false;
        unsigned long now = millis();
        if (now - lastLoop > transitionTime)
        {
            if (loopCount <= 1020)
            {
                lastLoop = now;

                redVal = UtilStep::calculateVal(stepR, redVal, loopCount);
                grnVal = UtilStep::calculateVal(stepG, grnVal, loopCount);
                bluVal = UtilStep::calculateVal(stepB, bluVal, loopCount);
                whtVal = UtilStep::calculateVal(stepW, whtVal, loopCount);

                this->setColor(redVal, grnVal, bluVal, whtVal); // Write current values to LED pins
                loopCount++;
            }
            else
            {
                inFade = false;
            }
        }
    }
}