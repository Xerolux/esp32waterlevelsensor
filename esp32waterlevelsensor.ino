#include "EspMQTTClient.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h> // Library for OTA updates
#include <ArduinoJson.h> // Library for handling JSON

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

String  client_name       = CLIENT_NAME;
String  startup_topic     = "waterlevel/Status";
String  water_level_topic = "waterlevel/Distanz";

const unsigned long event_interval = 15000;
unsigned long previous_time = 0;

EspMQTTClient client(
                  WIFI_SSID,
                  WIFI_PASS,
                  BROKER_IP,
                  BROKER_USERNAME,
                  BROKER_PASSWORD,
                  CLIENT_NAME,
                  BROKER_PORT
                  );

#define TRIG 14
#define ECHO 12

int distance = 0;

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String content = "Distance: " + String(distance) + " cm - Timestamp: " + String(millis());
    if (content.length() > 0) {
        request->send(200, "text/plain", content);
        Serial.println("Response sent successfully");
    } else {
        request->send(500, "text/plain", "Error: No data available");
        Serial.println("Error: No data available");
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

  if(current_time - previous_time >= event_interval) {
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
