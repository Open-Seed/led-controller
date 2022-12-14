# led-controller

Control RGB led strips using home assistant. This is a simple project with the aim to only control RGB strip lights.
You will be able to set the brightness and color of 5050, 3535 and other variant types that uses PWM signals.

## Setup

### Settings

1. Open the `settings.h` file to update the settings as per your setup
2. For the `CONFIG_STRIP` option, choose one of `BRIGHTNESS`, `RGB`, or `RGBW`.
3. Update the `CONFIG_PIN_*` with the pins your are planning to use.
4. Integration configuration:

   - For Home Assistant Device integration set `ENABLED_HOME_ASSISTANT` true
   - For MQTT Entity integration set `ENABLED_MQTT` true and update the `MQTT Topics` if required,for instance running more than one device and you do not want them grouped.

### Loading the firmware

1. This project was created in Visual Studio Code using the PlatformIO plugin.
2. Add the `led-controller` project to your list of project.
3. Use `Upload` or `Upload and Monitor` to upload your sketch to the ESP8266 board.

### Network

1. Connect to the `Open LED Controller` network on your device, The device IP address should be 192.168.4.1
2. Send the following json payload to the device on `http://192.168.4.1/api/system/preference mqtt config is shared between HA integration and mqtt integration

- [board][dns] is a easy way to grab the board on the network by navigating to http://openled.local. Note, this name should be unique as well on your network.
- [mqtt][name] is the unique identifier for the device on the ha/mqtt server

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

### Home Assistant (Device)

The integration will allow for auto discovery. The underlying library used to achieve this is [Arduino Home Assistant Integration](https://github.com/dawidchyrzynski/arduino-home-assistant) with implementation of the Light device.

Note, if you are struggling with authentication against your HA mosquito mqtt broker, add a custom `login` under `logins`

```yaml
logins:
  - username: user
    password: pass
```

### Home Assistant (MQTT)

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
mqtt:
    light:
      - name: aquarium_led
        unique_id: light.aquarium_led_1
        schema: json
        state_topic: "open_led/state"
        command_topic: "open_led/state/set"
        availability_topic: "open_led/availability"
        payload_available: "online"
        payload_not_available: "offline"
        brightness: true
        color_mode: true
        supported_color_modes: ["rgb"]
        effect: true
        effect_list: [color_fade_slow, color_fade_fast, flash]
        optimistic: false
        qos: 1

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
