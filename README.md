# led-controller

Control RGB led strips using home assistant. This is a simple project with the aim to only control RGB strip lights.
You will be able to set the brightness and color of 5050, 3535 and other variant types that uses PWM signals.

## Setup

### Settings

1. Open the `settings.h` file to update the settings as per your setup
2. For the `CONFIG_STRIP` option, choose one of `BRIGHTNESS`, `RGB`, or `RGBW`.
3. Update the `CONFIG_PIN_*` with the pins your are planning to use.
4. Update the `MQTT Topics` if required, for instance running more than one device and you do not want them grouped.

### Loading the firmware

1. This project was created in Visual Studio Code using the PlatformIO plugin.
2. Add the `led-controller` project to your list of project.
3. Use `Upload` or `Upload and Monitor` to upload your sketch to the ESP8266 board.

### Network

1. Connect to the `Open LED Controller` network on your device
2. Send the following json payload to the device on `http://192.168.68.104/api/system/preference

```json
{
  "board": {
    "name": "Open LED Controller",
    "dns": "openled"
  },
  "wifi": {
    "name": "Open LED Controller",
    "ssid": "ssid name",
    "password": "ssid password"
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

3. The device should now restarted and start trying to connect to your network and mqtt server.
4. Configure your mqtt server to communicate with the device.

### Home Assistant

To set this system up, you need to configure the [MQTT light component](https://www.home-assistant.io/integrations/light.mqtt#json-schema) in Home Assistant and set up a light to control. This guide assumes that you already have Home Assistant set up and running. If not, see the installation guides [here](https://home-assistant.io/getting-started/).

1. In your configuration.yaml, add the following, depending on the supported features of the light:

```yaml
# Only one color:
light:
  - platform: mqtt
    schema: json
    name: mqtt_json_light_1
    state_topic: "open_led/state"
    command_topic: "open_led/state/set"
    brightness: true
    effect: true
    effect_list: [flash]
    optimistic: false
    qos: 0

# RGB:
light:
  - platform: mqtt
    schema: json
    name: mqtt_json_light_2
    state_topic: "open_led/state"
    command_topic: "open_led/state/set"
    brightness: true
    rgb: true
    effect: true
    effect_list: [color_fade_slow, color_fade_fast, flash]
    optimistic: false
    qos: 0

# RGBW:
light:
  - platform: mqtt
    schema: json
    name: mqtt_json_light_3
    state_topic: "open_led/state"
    command_topic: "open_led/state/set"
    brightness: true
    rgb: true
    white_value: true
    effect: true
    effect_list: [color_fade_slow, color_fade_fast, flash]
    optimistic: false
    qos: 0
```

2. Set the name, state_topic, and command_topic to values that make sense for you.
3. Restart Home Assistant.

## Hardware

This project was built and tested on an ESP8266.

Components list:

- ESP8266
- Linear Voltage Regulator, 7805 5V (Require on 12v, the built in LM1117 3.3v regulator will run too hot on 12v)
- Capacitor 100nf
- Capacitor 10uf
- IRLB8721, IRL520N, ect mosfet able to run the LED strips and turn on with low voltage.
- LED strip of your choice, 3535, 5050
- Power source (5v/12v) dependent on your requirements

![ESP8266 Circuit for RGB LED Strip](/docs/assets/esp8266-circuit.png "ESP8266 Circuit")

## API

We use swagger to classify the API, you can view the api reference here [Online Api Reference](https://editor.swagger.io/?url=https://raw.githubusercontent.com/Open-Seed/led-controller/main/docs/assets/swagger.yaml)
The api is definition file is located at /docs/assets/swagger.yaml file.
