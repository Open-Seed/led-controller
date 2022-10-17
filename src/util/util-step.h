#ifndef UTIL_STEP_H
#define UTIL_STEP_H

#include <ArduinoJson.h>

class UtilStep
{

public:
    static int calculateStep(int prevValue, int endValue);
    static int calculateVal(int step, int val, int i);

private:
};

#endif