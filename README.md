# ESP32 JSN-SR04T Sensor

This repository contains code for an ESP32-based water level sensor using the JSN-SR04T ultrasonic sensor. The sensor measures the distance to the water surface and publishes the data to an MQTT broker. Additionally, it provides an HTTP endpoint to query the current water level and supports over-the-air (OTA) updates for firmware upgrades.

## Hardware Components

- ESP32 development board
- JSN-SR04T ultrasonic sensor
- WiFi network connection

## Prerequisites

Before using the code in this repository, make sure you have the following:

- Arduino IDE installed ([Download Arduino IDE](https://www.arduino.cc/en/Main/Software))
- Necessary libraries installed in the Arduino IDE:
  - EspMQTTClient ([Installation instructions](https://github.com/plapointe6/EspMQTTClient))
  - ESPAsyncWebServer ([Installation instructions](https://github.com/me-no-dev/ESPAsyncWebServer))
  - ArduinoJson ([Installation instructions](https://arduinojson.org/))
  - Update ([Installation instructions](https://github.com/esp8266/Arduino/blob/master/libraries/ArduinoOTA/README.md))

## Installation and Setup

1. Clone or download this repository to your local machine.
2. Open the Arduino IDE and navigate to `File > Open` to open the `JSN-SR04T.ino` file.
3. Connect your ESP32 board to your computer via USB.
4. Select the appropriate board and port from the Arduino IDE's `Tools` menu.
5. Upload the code to your ESP32 board.

## Configuration

Before uploading the code, make sure to modify the following variables in the `JSN-SR04T.ino` file to match your specific configuration:

```cpp
#define WIFI_SSID        "your_wifi_ssid"
#define WIFI_PASS        "your_wifi_password"
#define BROKER_IP        "mqtt_broker_ip"
#define BROKER_USERNAME  "mqtt_username"
#define BROKER_PASSWORD  "mqtt_password"
#define CLIENT_NAME      "esp32waterlevel"
#define BROKER_PORT      1883
#define lastwill_topic   "waterlevel/lastwill_topic_here"
#define lastwill_text    "offline"
#define OTA_HOSTNAME     "esp32-water-level" // Hostname for OTA
#define OTA_PASSWORD     "1234asdf" // Password for OTA (optional)
