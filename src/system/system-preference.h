#ifndef SYSTEM_PREFERENCE_H
#define SYSTEM_PREFERENCE_H

#include <ArduinoJson.h>

class SystemPreference
{

public:
    SystemPreference();
    static DynamicJsonDocument getPreferences(const char *name);
    static void setPreferences(const char *name, DynamicJsonDocument doc);
    static void clearPreferences(const char *name);

private:
};

#endif