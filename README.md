# led-controller

Control RGB led strips using home assistant

## Setup

To enable to controller to communicate with your wifi & mqqt server (Home Assistant), you will need to call the api with the following payload:

```
{
  "board": {
    "name": "openled",
    "dns": "openled"
  },
  "wifi": {
    "name": "openled",
    "ssid": "ssid name",
    "password": "ssid password"
  },
  "alarm": {
    "code": "alarmCode"
  },
  "mqtt": {
    "enabled": true,
    "name": "openled",
    "server": "server ip",
    "port": "server  port",
    "username": "mqtt username",
    "password": "mqtt password"
  }
}

```

Enter the detail as required

## API

We use swagger to classify the API, you can view the api reference here [Online Api Reference](https://editor.swagger.io/?url=https://raw.githubusercontent.com/Open-Seed/led-controller/main/docs/assets/swagger.yaml)
The api is definition file is located at /docs/assets/swagger.yaml file.
