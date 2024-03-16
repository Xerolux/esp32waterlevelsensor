#include "EspMQTTClient.h"
extern "C" {
#include "esp_wpa2.h"  // Include the WPA2 library for Enterprise
}
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>       // Library for OTA updates
#include <ArduinoJson.h>  // Library for handling JSON

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

// Define your static IP address and network parameters
IPAddress local_IP(192, 168, 180, 250);  // Example IP, change to fit your network
IPAddress gateway(192, 168, 180, 1);     // Your network gateway
IPAddress subnet(255, 255, 255, 0);      // Your network subnet
IPAddress primaryDNS(192, 168, 180, 1);  // Optional: DNS

String client_name = CLIENT_NAME;
String startup_topic = "waterlevel/Status";
String water_level_topic = "waterlevel/Distanz";

unsigned long event_interval = 60000;  // every minute
unsigned long previous_time = 0;

EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  BROKER_IP,
  BROKER_USERNAME,
  BROKER_PASSWORD,
  CLIENT_NAME,
  BROKER_PORT);

#define TRIG 14
#define ECHO 12

int distance = 0;

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Configure static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  // Initialize WiFi for WPA2 Enterprise
  WiFi.disconnect(true);  // Disconnect any existing connections
  WiFi.mode(WIFI_STA);    // Set to STA mode

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String content = "Distance: " + String(distance) + " cm - Timestamp: " + String(millis());
    request->send(200, "text/plain", content);
    Serial.println("Response sent successfully");
  });

  // Handler to adjust the event_interval
  server.on("/adjust_interval", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("interval")) {
      String intervalStr = request->getParam("interval")->value();
      int newInterval = intervalStr.toInt();
      if (newInterval > 0) {  // Basic validation
        event_interval = newInterval;
        request->send(200, "text/plain", "Interval updated");
        Serial.println("Interval updated to " + String(event_interval));
      } else {
        request->send(400, "text/plain", "Invalid interval");
        Serial.println("Invalid interval received");
      }
    } else {
      request->send(400, "text/plain", "Interval parameter missing");
      Serial.println("Interval parameter missing");
    }
  });

  server.begin();

  client.enableDebuggingMessages();
  client.enableHTTPWebUpdater();
  client.enableLastWillMessage(lastwill_topic, lastwill_text);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT_PULLUP);

  // Initialize OTA update
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  // Optionally, set OTA password
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  // Publish Home Assistant Discovery message
  publishHomeAssistantDiscovery();
}

void onConnectionEstablished() {
  client.publish(startup_topic, String(client_name + " is now online."));
}

void loop() {
  client.loop();

  unsigned long current_time = millis();

  if (current_time - previous_time >= event_interval) {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(20);

    digitalWrite(TRIG, LOW);

    distance = pulseIn(ECHO, HIGH, 26000);

    distance = distance / 58;

    Serial.print("Distance ");
    Serial.print(distance);
    Serial.println(" cm");

    client.publish(water_level_topic, String(distance));

    previous_time = current_time;
  }

  // Handle OTA update
  ArduinoOTA.handle();
}

void publishHomeAssistantDiscovery() {
  // Create a JSON payload for the sensor
  StaticJsonDocument<200> doc;
  doc["name"] = "Water Level Sensor";
  doc["state_topic"] = water_level_topic;
  doc["unit_of_measurement"] = "cm";
  doc["device_class"] = "distance";
  doc["unique_id"] = "water_level_sensor";

  char payload[200];
  serializeJson(doc, payload);

  // Publish the sensor discovery message
  client.publish("homeassistant/sensor/water_level_sensor/config", payload, true);
}
