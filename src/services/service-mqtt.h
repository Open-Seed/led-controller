#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

class MQTTService
{

public:
    MQTTService();
    void tick();

private:
    bool enabled;
    void reportStatusChanges();
};

#endif