/**
 * Open LED Settings
 */
#define FIRMWARE_VERSION "0.0.1"
#define FIRMWARE_UPDATE_URL "http://192.168.1.70:3000/firmware/httpUpdateNew.bin"

enum StripType
{
    BRIGHTNESS, // only one color/only white
    RGB,        // RGB LEDs
    RGBW        // RGB LEDs with an extra white LED per LED
};

#define CONFIG_STRIP RGB // Choose one of the options from above.

// MQTT Topics
#define CONFIG_MQTT_TOPIC_STATE "open_led/ESP_LED"
#define CONFIG_MQTT_TOPIC_SET "open_led/ESP_LED/set"

#define CONFIG_MQTT_PAYLOAD_ON "ON"
#define CONFIG_MQTT_PAYLOAD_OFF "OFF"

/**
 * WIFI Setup
 */
#define BOARD_NAME "openled"
#define BOARD_DNS "openled"
#define WIFI_SSID_NAME "Open LED Controller"
#define HTTP_REST_PORT 8080 // Port 80 clashes with MQTT client
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

/**
 * Preference Setup
 */
#define EEPROM_SIZE 512
#define PREFERENCE_SYSTEM_CONFIG "/system_config.json"

/**
 * Hardware
 */
// In case of BRIGHTNESS: only WHITE is used
// In case of RGB(W): red, green, blue(, white) is used
// All values need to be present, if they are not needed, set to -1,
// it will be ignored.
#define CONFIG_PIN_RED 0    // For RGB(W)
#define CONFIG_PIN_GREEN 2  // For RGB(W)
#define CONFIG_PIN_BLUE 3   // For RGB(W)
#define CONFIG_PIN_WHITE -1 // For BRIGHTNESS and RGBW

// Miscellaneous
#define CONFIG_DEFAULT_TRANSITION_TIME 1
// Default number of flashes if no value was given
#define CONFIG_DEFAULT_FLASH_LENGTH 2
// Number of seconds for one transition in colorfade mode
#define CONFIG_COLORFADE_TIME_SLOW 10
#define CONFIG_COLORFADE_TIME_FAST 3