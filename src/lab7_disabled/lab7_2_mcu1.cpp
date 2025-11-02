/**
 * @file lab7_2_mcu1.cpp
 * @brief Lab 7.2 MCU1 - Serial Protocol Sender (Unified)
 * 
 * Functionality:
 * - Reads HC-SR04 ultrasonic sensor
 * - Packages data into Serial protocol packets
 * - Sends packets via Serial/Serial1
 * - Responds to commands:
 *   • REQUEST_DATA: Send sensor reading
 *   • SET_LED: Control built-in LED
 * 
 * Architecture:
 * - SerialProtocol: Packet encoding
 * - UltrasonicHCSR04: Sensor reading
 * - SerialProtocolParser: Command parsing
 */

#include "lab7_2_mcu1.hpp"

#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "dd_serial.hpp"
#include "ultrasonic_hcsr04.hpp"
#include "serial_protocol.hpp"

// ============================================================================
// Hardware & State
// ============================================================================

namespace {
  UltrasonicHCSR04 ultrasonic(LAB72_ULTRASONIC_TRIG_PIN,
                               LAB72_ULTRASONIC_ECHO_PIN,
                               400);
  
  SerialProtocolParser parser;
  uint8_t packetCounter = 0;
  uint8_t ledState = LOW;
  
  unsigned long lastAutoSend = 0;
  uint32_t packetsSent = 0;
  uint32_t commandsReceived = 0;
}

// Use Serial1 if available (Mega has Serial1), otherwise use Serial
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  #define COMM_SERIAL Serial1
#else
  #define COMM_SERIAL Serial
#endif

// ============================================================================
// Packet Sending
// ============================================================================

void sendPacket(const SerialPacket& packet) {
  uint8_t buffer[128];
  uint8_t size = packet.serialize(buffer);
  
  COMM_SERIAL.write(buffer, size);
  COMM_SERIAL.flush();
  
  packetsSent++;
  
  printf("[TX] Packet #%u sent (Type: 0x%02X, Size: %u bytes)\n",
         packet.packetNumber, packet.packetType, size);
}

void sendSensorData() {
  uint16_t distance = ultrasonic.measureDistanceCm();
  
  SerialPacket packet(packetCounter++, LAB72_MCU1_ID, LAB72_MCU2_ID, LAB72_CMD_RESPONSE_DATA);
  
  // Payload: [distance_high, distance_low]
  uint8_t payload[2];
  payload[0] = (distance >> 8) & 0xFF;
  payload[1] = distance & 0xFF;
  
  packet.setPayload(payload, 2);
  sendPacket(packet);
  
  printf("[SENSOR] Distance: %u cm\n", distance);
}

void sendAck(uint8_t originalPktNum, uint8_t dstID) {
  SerialPacket packet(packetCounter++, LAB72_MCU1_ID, dstID, LAB72_CMD_ACK);
  
  // Payload: original packet number
  uint8_t payload[1] = {originalPktNum};
  packet.setPayload(payload, 1);
  
  sendPacket(packet);
}

// ============================================================================
// Command Processing
// ============================================================================

void processCommand(const SerialPacket& packet) {
  commandsReceived++;
  
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║     Command Received #%-5lu                       ║\n",
         (unsigned long)commandsReceived);
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Packet Details:\n");
  printf("  Packet Number:   %u\n", packet.packetNumber);
  printf("  Source ID:       0x%02X\n", packet.sourceID);
  printf("  Destination ID:  0x%02X\n", packet.destinationID);
  printf("  Type:            0x%02X\n", packet.packetType);
  printf("  Payload Length:  %u bytes\n", packet.payloadLength);
  printf("  Checksum:        0x%02X %s\n", packet.checksum,
         packet.isValid() ? "✓" : "✗");
  printf("\n");
  
  switch (packet.packetType) {
    case LAB72_CMD_REQUEST_DATA:
      printf("[CMD] REQUEST_DATA received\n");
      printf("      → Sending sensor data...\n");
      sendSensorData();
      break;
      
    case LAB72_CMD_SET_LED:
      if (packet.payloadLength > 0) {
        ledState = packet.payload[0] ? HIGH : LOW;
        digitalWrite(LED_BUILTIN, ledState);
        printf("[CMD] SET_LED: %s\n", ledState ? "ON" : "OFF");
        sendAck(packet.packetNumber, packet.sourceID);
      } else {
        printf("[ERROR] SET_LED: No payload\n");
      }
      break;
      
    default:
      printf("[ERROR] Unknown command type: 0x%02X\n", packet.packetType);
      break;
  }
  
  printf("════════════════════════════════════════════════════\n");
}

// ============================================================================
// Auto Send (Periodic)
// ============================================================================

void autoSendData() {
  unsigned long now = millis();
  if (now - lastAutoSend < LAB72_AUTO_SEND_PERIOD_MS) {
    return;
  }
  lastAutoSend = now;
  
  printf("[AUTO] Periodic sensor data transmission\n");
  sendSensorData();
}

// ============================================================================
// Serial Reception
// ============================================================================

void receiveAndParse() {
  while (COMM_SERIAL.available()) {
    uint8_t byte = COMM_SERIAL.read();
    
    if (parser.feedByte(byte)) {
      // Complete packet received
      const SerialPacket& packet = parser.getPacket();
      
      // Check if packet is for us
      if (packet.destinationID == LAB72_MCU1_ID || packet.destinationID == 0xFF) {
        processCommand(packet);
      }
      
      parser.reset();
    }
  }
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab7_2_mcu1() {
  // Initialize Serial for debug output
  SerialBegin();
  
  // Initialize communication serial
  COMM_SERIAL.begin(LAB72_SERIAL_BAUD);
  
  // Initialize LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Initialize sensor
  ultrasonic.begin();
  
  // Print welcome
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 7.2 MCU1: Serial Protocol Sender\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Device ID:         0x%02X (MCU1)\n", LAB72_MCU1_ID);
  printf("Comm Serial:       Serial1 @ %lu baud\n", (unsigned long)LAB72_SERIAL_BAUD);
  printf("Debug Serial:      Serial @ %lu baud\n", (unsigned long)SERIAL_BAUD);
  printf("Sensor:            HC-SR04 Ultrasonic\n");
  printf("Auto-send Period:  %lu ms\n", (unsigned long)LAB72_AUTO_SEND_PERIOD_MS);
  printf("\n");
  printf("Protocol Packet Format:\n");
  printf("  [START] [PKT#] [SRC] [DST] [TYPE] [LEN] [DATA...] [CHK] [END]\n");
  printf("  0x%02X    u8     u8    u8    u8     u8    ...      u8    0x%02X\n",
         SERIAL_PKT_START, SERIAL_PKT_END);
  printf("\n");
  printf("Supported Commands:\n");
  printf("  0x%02X - REQUEST_DATA: Send sensor reading\n", LAB72_CMD_REQUEST_DATA);
  printf("  0x%02X - SET_LED:      Control built-in LED\n", LAB72_CMD_SET_LED);
  printf("\n");
  printf("Features:\n");
  printf("  • Automatic periodic data transmission\n");
  printf("  • Command-based data requests\n");
  printf("  • LED control via protocol\n");
  printf("  • Packet validation with checksums\n");
  printf("\n");
  printf("Waiting for commands or auto-sending...\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  
  // Send initial packet
  sendSensorData();
}

void loop_lab7_2_mcu1() {
  // Receive and process commands
  receiveAndParse();
  
  // Auto-send data periodically
  autoSendData();
}
#ifndef LAB7_2_MCU1_HPP
#define LAB7_2_MCU1_HPP

/**
 * @file lab7_2_mcu1.hpp
 * @brief Lab 7.2 MCU1 - Serial Protocol Sender
 * 
 * MCU1 functionality:
 * - Reads HC-SR04 ultrasonic sensor
 * - Sends data via Serial protocol packets
 * - Responds to data requests from MCU2
 * - Implements SET_LED command
 */

void setup_lab7_2_mcu1();
void loop_lab7_2_mcu1();

#endif // LAB7_2_MCU1_HPP

