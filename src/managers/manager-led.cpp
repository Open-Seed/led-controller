#include "manager-led.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "settings.h"
#include "util/util-step.h"

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

const bool rgb = (CONFIG_STRIP == RGB) || (CONFIG_STRIP == RGBW);
const bool includeWhite = (CONFIG_STRIP == BRIGHTNESS) || (CONFIG_STRIP == RGBW);

bool stateOn = false;

// Globals for fade/transitions
bool startFade = false;
unsigned long lastLoop = 0;
int transitionTime = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB, stepW;
int redVal, grnVal, bluVal, whtVal;

// Globals for flash
bool flash = false;
bool startFlash = false;
int flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = red;
byte flashGreen = green;
byte flashBlue = blue;
byte flashWhite = white;
byte flashBrightness = brightness;

// Globals for colorfade
bool colorfade = false;
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
}

void LedManager::processJson(String &payload)
{
    StaticJsonDocument<256> doc;
    serializeJson(doc, payload);

    if (doc.containsKey("state"))
    {
        if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_ON) == 0)
        {
            stateOn = true;
        }
        else if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0)
        {
            stateOn = false;
        }
    }

    // If "flash" is included, treat RGB and brightness differently
    if (doc.containsKey("flash") ||
        (doc.containsKey("effect") && strcmp(doc["effect"], "flash") == 0))
    {

        if (doc.containsKey("flash"))
        {
            flashLength = (int)doc["flash"] * 1000;
        }
        else
        {
            flashLength = CONFIG_DEFAULT_FLASH_LENGTH * 1000;
        }

        if (doc.containsKey("brightness"))
        {
            flashBrightness = doc["brightness"];
        }
        else
        {
            flashBrightness = brightness;
        }

        if (rgb && doc.containsKey("color"))
        {
            flashRed = doc["color"]["r"];
            flashGreen = doc["color"]["g"];
            flashBlue = doc["color"]["b"];
        }
        else
        {
            flashRed = red;
            flashGreen = green;
            flashBlue = blue;
        }

        if (includeWhite && doc.containsKey("white_value"))
        {
            flashWhite = doc["white_value"];
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
    else if (rgb && doc.containsKey("effect") &&
             (strcmp(doc["effect"], "colorfade_slow") == 0 || strcmp(doc["effect"], "colorfade_fast") == 0))
    {
        flash = false;
        colorfade = true;
        currentColor = 0;
        if (strcmp(doc["effect"], "colorfade_slow") == 0)
        {
            transitionTime = CONFIG_COLORFADE_TIME_SLOW;
        }
        else
        {
            transitionTime = CONFIG_COLORFADE_TIME_FAST;
        }
    }
    else if (colorfade && !doc.containsKey("color") && doc.containsKey("brightness"))
    {
        // Adjust brightness during colorfade
        // (will be applied when fading to the next color)
        brightness = doc["brightness"];
    }
    else
    { // No effect
        flash = false;
        colorfade = false;

        if (rgb && doc.containsKey("color"))
        {
            red = doc["color"]["r"];
            green = doc["color"]["g"];
            blue = doc["color"]["b"];
        }

        if (includeWhite && doc.containsKey("white_value"))
        {
            white = doc["white_value"];
        }

        if (doc.containsKey("brightness"))
        {
            brightness = doc["brightness"];
        }

        if (doc.containsKey("transition"))
        {
            transitionTime = doc["transition"];
        }
        else
        {
            transitionTime = CONFIG_DEFAULT_TRANSITION_TIME;
        }
    }
}

void setColor(int inR, int inG, int inB, int inW)
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

    Serial.print("Setting LEDs: {");
    if (rgb)
    {
        Serial.print("r: ");
        Serial.print(inR);
        Serial.print(" , g: ");
        Serial.print(inG);
        Serial.print(" , b: ");
        Serial.print(inB);
    }

    if (includeWhite)
    {
        if (rgb)
        {
            Serial.print(", ");
        }
        Serial.print("w: ");
        Serial.print(inW);
    }

    Serial.println("}");
}

void LedManager::setState()
{
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
}

String LedManager::getState()
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

    if (includeWhite)
    {
        doc["white_value"] = white;
    }

    if (rgb && colorfade)
    {
        if (transitionTime == CONFIG_COLORFADE_TIME_SLOW)
        {
            doc["effect"] = "colorfade_slow";
        }
        else
        {
            doc["effect"] = "colorfade_fast";
        }
    }
    else
    {
        doc["effect"] = "null";
    }

    String json;
    serializeJson(doc, json);

    return json;
}

void LedManager::tick()
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
                setColor(flashRed, flashGreen, flashBlue, flashWhite);
            }
            else
            {
                setColor(0, 0, 0, 0);
            }
        }
        else
        {
            flash = false;
            setColor(realRed, realGreen, realBlue, realWhite);
        }
    }
    else if (rgb && colorfade && !inFade)
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
            setColor(realRed, realGreen, realBlue, realWhite);

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

                setColor(redVal, grnVal, bluVal, whtVal); // Write current values to LED pins
                loopCount++;
            }
            else
            {
                inFade = false;
            }
        }
    }
}