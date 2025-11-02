#include "mqtt_client_wrapper.hpp"

#ifdef ESP32

MqttClientWrapper* MqttClientWrapper::s_instance = nullptr;

MqttClientWrapper::MqttClientWrapper(const char* ssid, const char* password,
                                     const char* broker, uint16_t port,
                                     const char* clientID)
    : m_ssid(ssid)
    , m_password(password)
    , m_broker(broker)
    , m_port(port)
    , m_clientID(clientID)
    , m_mqttClient(m_wifiClient)
    , m_userCallback(nullptr) {
  
  s_instance = this;
}

void MqttClientWrapper::begin() {
  m_mqttClient.setServer(m_broker, m_port);
  m_mqttClient.setCallback(mqttCallbackStatic);
}

bool MqttClientWrapper::connectWiFi() {
  if (isWiFiConnected()) {
    return true;
  }
  
  Serial.print("[WiFi] Connecting to ");
  Serial.print(m_ssid);
  Serial.println("...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(m_ssid, m_password);
  
  unsigned long startTime = millis();
  while (!isWiFiConnected()) {
    if (millis() - startTime > 10000) {  // 10s timeout
      Serial.println("[WiFi] Connection timeout!");
      return false;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("[WiFi] Connected!");
  Serial.print("[WiFi] IP Address: ");
  Serial.println(getIPAddress());
  Serial.print("[WiFi] RSSI: ");
  Serial.print(getWiFiRSSI());
  Serial.println(" dBm");
  
  return true;
}

bool MqttClientWrapper::connectMQTT(const char* user, const char* password) {
  if (isMQTTConnected()) {
    return true;
  }
  
  Serial.print("[MQTT] Connecting to ");
  Serial.print(m_broker);
  Serial.print(":");
  Serial.print(m_port);
  Serial.println("...");
  
  bool connected = false;
  
  if (user != nullptr && strlen(user) > 0) {
    connected = m_mqttClient.connect(m_clientID, user, password);
  } else {
    connected = m_mqttClient.connect(m_clientID);
  }
  
  if (connected) {
    Serial.println("[MQTT] Connected!");
    return true;
  } else {
    Serial.print("[MQTT] Connection failed! State: ");
    Serial.println(m_mqttClient.state());
    return false;
  }
}

void MqttClientWrapper::loop() {
  if (!isWiFiConnected()) {
    connectWiFi();
  }
  
  if (isWiFiConnected() && !isMQTTConnected()) {
    // Attempt reconnection
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) {
      lastAttempt = millis();
      connectMQTT();
    }
  }
  
  m_mqttClient.loop();
}

bool MqttClientWrapper::publish(const char* topic, const uint8_t* payload, 
                                 unsigned int length, bool retained) {
  if (!isMQTTConnected()) {
    Serial.println("[MQTT] Cannot publish - not connected");
    return false;
  }
  
  return m_mqttClient.publish(topic, payload, length, retained);
}

bool MqttClientWrapper::publish(const char* topic, const char* message, bool retained) {
  return publish(topic, (const uint8_t*)message, strlen(message), retained);
}

bool MqttClientWrapper::subscribe(const char* topic) {
  if (!isMQTTConnected()) {
    Serial.println("[MQTT] Cannot subscribe - not connected");
    return false;
  }
  
  Serial.print("[MQTT] Subscribing to: ");
  Serial.println(topic);
  
  return m_mqttClient.subscribe(topic);
}

bool MqttClientWrapper::unsubscribe(const char* topic) {
  return m_mqttClient.unsubscribe(topic);
}

void MqttClientWrapper::setCallback(MqttMessageCallback callback) {
  m_userCallback = callback;
}

void MqttClientWrapper::mqttCallbackStatic(char* topic, uint8_t* payload, unsigned int length) {
  if (s_instance != nullptr && s_instance->m_userCallback != nullptr) {
    s_instance->m_userCallback(topic, payload, length);
  }
}

#endif // ESP32
#ifndef MQTT_CLIENT_WRAPPER_HPP
#define MQTT_CLIENT_WRAPPER_HPP

#ifdef ESP32

#include <WiFi.h>
#include <PubSubClient.h>
#include <stdint.h>

/**
 * @file mqtt_client_wrapper.hpp
 * @brief MQTT Client Wrapper for ESP32
 * 
 * Simplified MQTT interface with:
 * - WiFi connection management
 * - MQTT broker connection
 * - Publish/Subscribe functionality
 * - Callback handling
 */

/**
 * @brief MQTT message callback signature
 * @param topic Topic name
 * @param payload Message payload
 * @param length Payload length
 */
typedef void (*MqttMessageCallback)(const char* topic, const uint8_t* payload, unsigned int length);

/**
 * @class MqttClientWrapper
 * @brief MQTT client wrapper with WiFi management
 */
class MqttClientWrapper {
public:
  /**
   * @brief Constructor
   * @param ssid WiFi SSID
   * @param password WiFi password
   * @param broker MQTT broker address
   * @param port MQTT broker port
   * @param clientID MQTT client ID
   */
  MqttClientWrapper(const char* ssid, const char* password,
                    const char* broker, uint16_t port,
                    const char* clientID);
  
  /**
   * @brief Initialize WiFi and MQTT
   */
  void begin();
  
  /**
   * @brief Connect to WiFi
   * @return true if successful
   */
  bool connectWiFi();
  
  /**
   * @brief Connect to MQTT broker
   * @param user Username (optional, can be nullptr)
   * @param password Password (optional, can be nullptr)
   * @return true if successful
   */
  bool connectMQTT(const char* user = nullptr, const char* password = nullptr);
  
  /**
   * @brief Main loop - must be called regularly
   */
  void loop();
  
  /**
   * @brief Publish message to topic
   * @param topic Topic name
   * @param payload Payload data
   * @param length Payload length
   * @param retained Retained message flag
   * @return true if successful
   */
  bool publish(const char* topic, const uint8_t* payload, unsigned int length, bool retained = false);
  
  /**
   * @brief Publish string message
   * @param topic Topic name
   * @param message String message
   * @param retained Retained message flag
   * @return true if successful
   */
  bool publish(const char* topic, const char* message, bool retained = false);
  
  /**
   * @brief Subscribe to topic
   * @param topic Topic name
   * @return true if successful
   */
  bool subscribe(const char* topic);
  
  /**
   * @brief Unsubscribe from topic
   * @param topic Topic name
   * @return true if successful
   */
  bool unsubscribe(const char* topic);
  
  /**
   * @brief Set message callback
   * @param callback Callback function
   */
  void setCallback(MqttMessageCallback callback);
  
  /**
   * @brief Check if WiFi is connected
   * @return true if connected
   */
  bool isWiFiConnected() const { return WiFi.status() == WL_CONNECTED; }
  
  /**
   * @brief Check if MQTT is connected
   * @return true if connected
   */
  bool isMQTTConnected() const { return m_mqttClient.connected(); }
  
  /**
   * @brief Get WiFi RSSI
   * @return RSSI value
   */
  int32_t getWiFiRSSI() const { return WiFi.RSSI(); }
  
  /**
   * @brief Get IP address
   * @return IP address string
   */
  String getIPAddress() const { return WiFi.localIP().toString(); }

private:
  const char* m_ssid;
  const char* m_password;
  const char* m_broker;
  uint16_t m_port;
  const char* m_clientID;
  
  WiFiClient m_wifiClient;
  PubSubClient m_mqttClient;
  MqttMessageCallback m_userCallback;
  
  static void mqttCallbackStatic(char* topic, uint8_t* payload, unsigned int length);
  static MqttClientWrapper* s_instance;
};

#endif // ESP32

#endif // MQTT_CLIENT_WRAPPER_HPP

