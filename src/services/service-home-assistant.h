#ifndef HOME_ASSISTANT_SERVICE_H
#define HOME_ASSISTANT_SERVICE_H

class HomeAssistantService
{
public:
    HomeAssistantService();
    void loop();

private:
    String mqttClientName;
    String mqttUsername;
    String mqttPassword;
    String mqttServer;
};

#endif