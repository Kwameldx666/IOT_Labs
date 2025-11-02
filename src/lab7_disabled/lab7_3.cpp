/**
 * @file lab7_3.cpp
 * @brief Lab 7.3 - MQTT Internet Communication (Unified)
 * 
 * Complete IoT application using ESP32:
 * - WiFi connectivity
 * - MQTT protocol (ThingsBoard or HiveMQ)
 * - Sensor data publishing (temperature, humidity, distance)
 * - Remote actuator control (LED, relay)
 * - Dashboard integration
 * 
 * Architecture:
 * - MqttClientWrapper: MQTT communication
 * - DHT sensor: Temperature/Humidity (or reuse ultrasonic)
 * - RelayDriver: Actuator control
 * - JSON for telemetry formatting
 * 
 * ThingsBoard Setup:
 * 1. Create device on demo.thingsboard.io
 * 2. Copy access token to LAB73_MQTT_USER
 * 3. Dashboard will show telemetry automatically
 * 
 * Commands from dashboard:
 * - {"method":"setLED","params":true}  -> Turn LED ON
 * - {"method":"setLED","params":false} -> Turn LED OFF
 * - {"method":"setRelay","params":1}   -> Turn relay ON
 */

#ifdef ESP32

#include "lab7_3.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "mqtt_client_wrapper.hpp"

// Optional: Include DHT library if available
// #include <DHT.h>

// ============================================================================
// Hardware & State
// ============================================================================

namespace {
  MqttClientWrapper mqttClient(LAB73_WIFI_SSID, 
                                LAB73_WIFI_PASSWORD,
                                LAB73_MQTT_BROKER,
                                LAB73_MQTT_PORT,
                                LAB73_MQTT_CLIENT_ID);
  
  // State
  unsigned long lastTelemetry = 0;
  bool ledState = false;
  bool relayState = false;
  
  // Sensor simulation (replace with real sensor)
  float temperature = 25.0f;
  float humidity = 50.0f;
  uint16_t distance = 100;
}

// ============================================================================
// Sensor Reading (Simulated - replace with real sensor)
// ============================================================================

void readSensors() {
  // Simulate sensor data with small variations
  temperature = 25.0f + (random(-50, 50) / 10.0f);
  humidity = 50.0f + (random(-100, 100) / 10.0f);
  distance = 100 + random(-20, 20);
  
  // If using real DHT sensor:
  // DHT dht(LAB73_DHT_PIN, LAB73_DHT_TYPE);
  // temperature = dht.readTemperature();
  // humidity = dht.readHumidity();
  
  // If using ultrasonic (reuse from previous labs):
  // UltrasonicHCSR04 ultrasonic(...);
  // distance = ultrasonic.measureDistanceCm();
}

// ============================================================================
// Actuator Control
// ============================================================================

void setLED(bool state) {
  ledState = state;
  digitalWrite(LAB73_LED_PIN, state ? HIGH : LOW);
  Serial.print("[ACTUATOR] LED set to: ");
  Serial.println(state ? "ON" : "OFF");
}

void setRelay(bool state) {
  relayState = state;
  digitalWrite(LAB73_RELAY_PIN, state ? HIGH : LOW);
  Serial.print("[ACTUATOR] Relay set to: ");
  Serial.println(state ? "ON" : "OFF");
}

// ============================================================================
// MQTT Publishing
// ============================================================================

void publishTelemetry() {
  unsigned long now = millis();
  if (now - lastTelemetry < LAB73_TELEMETRY_INTERVAL_MS) {
    return;
  }
  lastTelemetry = now;
  
  // Read sensors
  readSensors();
  
  // Create JSON telemetry
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["distance"] = distance;
  doc["ledState"] = ledState;
  doc["relayState"] = relayState;
  doc["rssi"] = mqttClient.getWiFiRSSI();
  doc["uptime"] = millis() / 1000;
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════════╗");
  Serial.println("║     Publishing Telemetry                          ║");
  Serial.println("╚════════════════════════════════════════════════════╝");
  Serial.println();
  Serial.println("Data:");
  Serial.print("  Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("  Humidity:    ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("  Distance:    ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("  LED State:   ");
  Serial.println(ledState ? "ON" : "OFF");
  Serial.print("  Relay State: ");
  Serial.println(relayState ? "ON" : "OFF");
  Serial.println();
  Serial.print("JSON: ");
  Serial.println(buffer);
  Serial.println();
  
  if (mqttClient.publish(LAB73_TOPIC_TELEMETRY, buffer)) {
    Serial.println("[MQTT] ✓ Telemetry published successfully");
  } else {
    Serial.println("[MQTT] ✗ Failed to publish telemetry");
  }
  
  Serial.println("════════════════════════════════════════════════════");
}

void publishAttributes() {
  StaticJsonDocument<128> doc;
  doc["deviceType"] = "ESP32";
  doc["firmware"] = "Lab7.3_v1.0";
  doc["ipAddress"] = mqttClient.getIPAddress();
  
  char buffer[128];
  serializeJson(doc, buffer);
  
  mqttClient.publish(LAB73_TOPIC_ATTRIBUTES, buffer);
  Serial.println("[MQTT] Attributes published");
}

// ============================================================================
// MQTT Message Handling
// ============================================================================

void onMqttMessage(const char* topic, const uint8_t* payload, unsigned int length) {
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════════╗");
  Serial.println("║     MQTT Message Received                         ║");
  Serial.println("╚════════════════════════════════════════════════════╝");
  Serial.println();
  Serial.print("Topic:   ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.write(payload, length);
  Serial.println();
  Serial.println();
  
  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  
  if (error) {
    Serial.print("[ERROR] JSON parsing failed: ");
    Serial.println(error.c_str());
    Serial.println("════════════════════════════════════════════════════");
    return;
  }
  
  // Extract method and params
  const char* method = doc["method"];
  
  if (method == nullptr) {
    Serial.println("[ERROR] No method field in JSON");
    Serial.println("════════════════════════════════════════════════════");
    return;
  }
  
  Serial.print("Method:  ");
  Serial.println(method);
  
  // Handle commands
  if (strcmp(method, "setLED") == 0) {
    bool state = doc["params"];
    Serial.print("Command: Set LED to ");
    Serial.println(state ? "ON" : "OFF");
    setLED(state);
    
    // Send RPC response (ThingsBoard)
    if (strstr(topic, "rpc/request") != nullptr) {
      // Extract request ID from topic
      const char* requestId = strrchr(topic, '/');
      if (requestId != nullptr) {
        requestId++;  // Skip '/'
        
        char responseTopic[128];
        snprintf(responseTopic, sizeof(responseTopic), "%s%s", 
                 LAB73_TOPIC_RPC_RESPONSE, requestId);
        
        StaticJsonDocument<64> responseDoc;
        responseDoc["success"] = true;
        responseDoc["ledState"] = ledState;
        
        char responseBuffer[64];
        serializeJson(responseDoc, responseBuffer);
        
        mqttClient.publish(responseTopic, responseBuffer);
        Serial.println("[RPC] Response sent");
      }
    }
  }
  else if (strcmp(method, "setRelay") == 0) {
    bool state = doc["params"];
    Serial.print("Command: Set Relay to ");
    Serial.println(state ? "ON" : "OFF");
    setRelay(state);
    
    // Send RPC response
    if (strstr(topic, "rpc/request") != nullptr) {
      const char* requestId = strrchr(topic, '/');
      if (requestId != nullptr) {
        requestId++;
        
        char responseTopic[128];
        snprintf(responseTopic, sizeof(responseTopic), "%s%s",
                 LAB73_TOPIC_RPC_RESPONSE, requestId);
        
        StaticJsonDocument<64> responseDoc;
        responseDoc["success"] = true;
        responseDoc["relayState"] = relayState;
        
        char responseBuffer[64];
        serializeJson(responseDoc, responseBuffer);
        
        mqttClient.publish(responseTopic, responseBuffer);
        Serial.println("[RPC] Response sent");
      }
    }
  }
  else if (strcmp(method, "getTelemetry") == 0) {
    Serial.println("Command: Get current telemetry");
    publishTelemetry();
  }
  else {
    Serial.print("[WARNING] Unknown method: ");
    Serial.println(method);
  }
  
  Serial.println("════════════════════════════════════════════════════");
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab7_3() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize hardware
  pinMode(LAB73_LED_PIN, OUTPUT);
  pinMode(LAB73_RELAY_PIN, OUTPUT);
  digitalWrite(LAB73_LED_PIN, LOW);
  digitalWrite(LAB73_RELAY_PIN, LOW);
  
  // Print welcome
  Serial.println();
  Serial.println("════════════════════════════════════════════════════");
  Serial.println("   Lab 7.3: MQTT Internet Communication (ESP32)");
  Serial.println("════════════════════════════════════════════════════");
  Serial.println();
  Serial.println("Configuration:");
  Serial.print("  WiFi SSID:      ");
  Serial.println(LAB73_WIFI_SSID);
  Serial.print("  MQTT Broker:    ");
  Serial.print(LAB73_MQTT_BROKER);
  Serial.print(":");
  Serial.println(LAB73_MQTT_PORT);
  Serial.print("  Client ID:      ");
  Serial.println(LAB73_MQTT_CLIENT_ID);
  Serial.println();
  Serial.println("Features:");
  Serial.println("  • WiFi connectivity");
  Serial.println("  • MQTT telemetry publishing");
  Serial.println("  • Remote actuator control");
  Serial.println("  • ThingsBoard/HiveMQ integration");
  Serial.println("  • JSON formatted messages");
  Serial.println();
  Serial.println("Telemetry:");
  Serial.println("  - Temperature (simulated)");
  Serial.println("  - Humidity (simulated)");
  Serial.println("  - Distance (simulated)");
  Serial.println("  - LED state");
  Serial.println("  - Relay state");
  Serial.println("  - WiFi RSSI");
  Serial.println();
  Serial.println("Supported Commands (from dashboard):");
  Serial.println("  {\"method\":\"setLED\",\"params\":true}");
  Serial.println("  {\"method\":\"setRelay\",\"params\":true}");
  Serial.println("  {\"method\":\"getTelemetry\",\"params\":{}}");
  Serial.println();
  Serial.println("════════════════════════════════════════════════════");
  Serial.println();
  
  // Initialize MQTT
  mqttClient.begin();
  mqttClient.setCallback(onMqttMessage);
  
  // Connect to WiFi
  if (!mqttClient.connectWiFi()) {
    Serial.println("[ERROR] WiFi connection failed!");
    Serial.println("Please check your WiFi credentials in config.h");
    Serial.println("Halting...");
    while (1) {
      delay(1000);
    }
  }
  
  // Connect to MQTT
  if (!mqttClient.connectMQTT(LAB73_MQTT_USER, LAB73_MQTT_PASSWORD)) {
    Serial.println("[WARNING] MQTT connection failed!");
    Serial.println("Will retry automatically...");
  } else {
    // Subscribe to RPC requests
    mqttClient.subscribe(LAB73_TOPIC_RPC_REQUEST);
    
    // Publish initial attributes
    publishAttributes();
    
    // Publish initial telemetry
    publishTelemetry();
  }
  
  Serial.println();
  Serial.println("Setup complete! Running...");
  Serial.println();
}

void loop_lab7_3() {
  // MQTT loop (handles WiFi/MQTT reconnection automatically)
  mqttClient.loop();
  
  // Publish telemetry periodically
  publishTelemetry();
  
  // Small delay to prevent watchdog issues
  delay(10);
}

#else

// Stub for non-ESP32 platforms
void setup_lab7_3() {
  Serial.begin(115200);
  Serial.println("[ERROR] Lab 7.3 requires ESP32!");
  Serial.println("Please select ESP32 board in platformio.ini");
}

void loop_lab7_3() {
  delay(1000);
}

#endif // ESP32
#ifndef LAB7_3_HPP
#define LAB7_3_HPP

/**
 * @file lab7_3.hpp
 * @brief Lab 7.3 - MQTT Internet Communication (ESP32)
 * 
 * ESP32 application with MQTT:
 * - Reads sensor data (DHT22 or ultrasonic)
 * - Publishes telemetry to MQTT broker (ThingsBoard/HiveMQ)
 * - Subscribes to RPC commands
 * - Controls actuators (LED, relay) via MQTT
 * - Dashboard visualization and control
 */

void setup_lab7_3();
void loop_lab7_3();

#endif // LAB7_3_HPP

