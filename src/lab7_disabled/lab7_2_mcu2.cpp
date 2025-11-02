/**
 * @file lab7_2_mcu2.cpp
 * @brief Lab 7.2 MCU2 - Serial Protocol Receiver (Unified)
 * 
 * Functionality:
 * - Receives packets from MCU1 via Serial/Serial1
 * - Parses and validates protocol packets
 * - Interprets sensor data
 * - Sends commands via user input:
 *   • "request" - Request sensor data
 *   • "led on/off" - Control MCU1 LED
 * - Displays data on LCD
 * 
 * Architecture:
 * - SerialProtocolParser: Packet parsing
 * - SerialPacket: Packet creation for commands
 * - LCD: Data display
 */

#include "lab7_2_mcu2.hpp"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "dd_serial.hpp"
#include "dd_lcd.hpp"
#include "serial_protocol.hpp"

// ============================================================================
// State
// ============================================================================

namespace {
  SerialProtocolParser parser;
  uint8_t packetCounter = 0;
  
  uint16_t lastDistance = 0;
  unsigned long lastPacketTime = 0;
  unsigned long lastLCDUpdate = 0;
  
  uint32_t packetsReceived = 0;
  uint32_t validPackets = 0;
  uint32_t invalidPackets = 0;
}

// Use Serial1 if available
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  #define COMM_SERIAL Serial1
#else
  #define COMM_SERIAL Serial
#endif

// ============================================================================
// Packet Sending
// ============================================================================

void sendCommand(uint8_t cmdType, const uint8_t* payload, uint8_t payloadLen) {
  SerialPacket packet(packetCounter++, LAB72_MCU2_ID, LAB72_MCU1_ID, cmdType);
  
  if (payloadLen > 0) {
    packet.setPayload(payload, payloadLen);
  } else {
    packet.updateChecksum();
  }
  
  uint8_t buffer[128];
  uint8_t size = packet.serialize(buffer);
  
  COMM_SERIAL.write(buffer, size);
  COMM_SERIAL.flush();
  
  printf("[TX] Command sent (Type: 0x%02X, Size: %u bytes)\n", cmdType, size);
}

void requestSensorData() {
  printf("[CMD] Requesting sensor data from MCU1...\n");
  sendCommand(LAB72_CMD_REQUEST_DATA, nullptr, 0);
}

void setLED(bool state) {
  uint8_t payload[1] = {state ? 1 : 0};
  printf("[CMD] Setting MCU1 LED: %s\n", state ? "ON" : "OFF");
  sendCommand(LAB72_CMD_SET_LED, payload, 1);
}

// ============================================================================
// Packet Processing
// ============================================================================

void processReceivedPacket(const SerialPacket& packet) {
  validPackets++;
  packetsReceived++;
  lastPacketTime = millis();
  
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║     Packet Received #%-5lu                        ║\n",
         (unsigned long)packetsReceived);
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  
  // Detailed packet breakdown
  printf("Packet Breakdown:\n");
  printf("  START:           0x%02X %s\n", packet.start,
         packet.start == SERIAL_PKT_START ? "✓" : "✗");
  printf("  Packet Number:   %u\n", packet.packetNumber);
  printf("  Source ID:       0x%02X (MCU%u)\n", packet.sourceID, packet.sourceID);
  printf("  Destination ID:  0x%02X (MCU%u)\n", packet.destinationID, packet.destinationID);
  printf("  Packet Type:     0x%02X ", packet.packetType);
  
  switch (packet.packetType) {
    case LAB72_CMD_RESPONSE_DATA: printf("(DATA RESPONSE)"); break;
    case LAB72_CMD_ACK: printf("(ACK)"); break;
    case LAB72_CMD_NACK: printf("(NACK)"); break;
    default: printf("(UNKNOWN)"); break;
  }
  printf("\n");
  
  printf("  Payload Length:  %u bytes\n", packet.payloadLength);
  printf("  Payload (hex):   ");
  for (uint8_t i = 0; i < packet.payloadLength; i++) {
    printf("%02X ", packet.payload[i]);
  }
  printf("\n");
  
  printf("  Checksum:        0x%02X %s\n", packet.checksum,
         packet.isValid() ? "✓" : "✗");
  printf("  END:             0x%02X %s\n", packet.end,
         packet.end == SERIAL_PKT_END ? "✓" : "✗");
  printf("\n");
  
  // Interpret data based on type
  if (packet.packetType == LAB72_CMD_RESPONSE_DATA) {
    if (packet.payloadLength >= 2) {
      lastDistance = (static_cast<uint16_t>(packet.payload[0]) << 8) | packet.payload[1];
      
      printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
      printf("  📏 SENSOR DATA DECODED\n");
      printf("  Distance: %u cm\n", lastDistance);
      printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
  } else if (packet.packetType == LAB72_CMD_ACK) {
    if (packet.payloadLength >= 1) {
      printf("✓ ACK received for packet #%u\n", packet.payload[0]);
    }
  }
  
  printf("\n");
  printf("Statistics:\n");
  printf("  Total Received:  %lu\n", (unsigned long)packetsReceived);
  printf("  Valid Packets:   %lu (%.1f%%)\n", (unsigned long)validPackets,
         packetsReceived > 0 ? (validPackets * 100.0f) / packetsReceived : 0.0f);
  printf("  Invalid Packets: %lu (%.1f%%)\n", (unsigned long)invalidPackets,
         packetsReceived > 0 ? (invalidPackets * 100.0f) / packetsReceived : 0.0f);
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

// ============================================================================
// LCD Display
// ============================================================================

void updateLCD() {
  unsigned long now = millis();
  if (now - lastLCDUpdate < 300) {
    return;
  }
  lastLCDUpdate = now;
  
  dd_lcd_clear();
  
  char line1[17];
  snprintf(line1, sizeof(line1), "Dist: %u cm", lastDistance);
  dd_lcd_write(0, 0, line1);
  
  char line2[17];
  unsigned long timeSince = (millis() - lastPacketTime) / 1000;
  snprintf(line2, sizeof(line2), "RX:%lu %lus ago",
           (unsigned long)validPackets, timeSince);
  dd_lcd_write(1, 0, line2);
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
      if (packet.destinationID == LAB72_MCU2_ID || packet.destinationID == 0xFF) {
        processReceivedPacket(packet);
      }
      
      parser.reset();
    }
  }
}

// ============================================================================
// Command Processing
// ============================================================================

void processUserCommand(const char* command) {
  if (strcmp(command, "request") == 0) {
    requestSensorData();
    return;
  }
  
  if (strcmp(command, "led") == 0 || strncmp(command, "led ", 4) == 0) {
    const char* state = command + 3;
    while (*state == ' ') state++;  // Skip spaces
    
    if (strcmp(state, "on") == 0) {
      setLED(true);
    } else if (strcmp(state, "off") == 0) {
      setLED(false);
    } else {
      printf("[ERROR] Usage: led on/off\n");
    }
    return;
  }
  
  if (strcmp(command, "stats") == 0) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║     Reception Statistics                          ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Packets Received:    %lu\n", (unsigned long)packetsReceived);
    printf("Valid Packets:       %lu (%.1f%%)\n", (unsigned long)validPackets,
           packetsReceived > 0 ? (validPackets * 100.0f) / packetsReceived : 0.0f);
    printf("Invalid Packets:     %lu (%.1f%%)\n", (unsigned long)invalidPackets,
           packetsReceived > 0 ? (invalidPackets * 100.0f) / packetsReceived : 0.0f);
    printf("Last Distance:       %u cm\n", lastDistance);
    printf("Time Since Last RX:  %lu seconds\n", 
           (millis() - lastPacketTime) / 1000);
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
    return;
  }
  
  if (strcmp(command, "reset") == 0) {
    packetsReceived = 0;
    validPackets = 0;
    invalidPackets = 0;
    printf("[INFO] Statistics reset\n");
    return;
  }
  
  if (strcmp(command, "help") == 0) {
    printf("\n");
    printf("Available Commands:\n");
    printf("  request    - Request sensor data from MCU1\n");
    printf("  led on     - Turn on MCU1 LED\n");
    printf("  led off    - Turn off MCU1 LED\n");
    printf("  stats      - Show reception statistics\n");
    printf("  reset      - Reset statistics\n");
    printf("  help       - Show this help\n");
    printf("\n");
    return;
  }
  
  printf("[ERROR] Unknown command: %s\n", command);
  printf("Type 'help' for available commands\n");
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab7_2_mcu2() {
  // Initialize Serial for debug
  SerialBegin();
  
  // Initialize LCD
  dd_lcd_init();
  dd_lcd_clear();
  
  // Initialize communication serial
  COMM_SERIAL.begin(LAB72_SERIAL_BAUD);
  
  // Print welcome
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 7.2 MCU2: Serial Protocol Receiver\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Device ID:         0x%02X (MCU2)\n", LAB72_MCU2_ID);
  printf("Comm Serial:       Serial1 @ %lu baud\n", (unsigned long)LAB72_SERIAL_BAUD);
  printf("Debug Serial:      Serial @ %lu baud\n", (unsigned long)SERIAL_BAUD);
  printf("\n");
  printf("Protocol Packet Format:\n");
  printf("  [START] [PKT#] [SRC] [DST] [TYPE] [LEN] [DATA...] [CHK] [END]\n");
  printf("  0x%02X    u8     u8    u8    u8     u8    ...      u8    0x%02X\n",
         SERIAL_PKT_START, SERIAL_PKT_END);
  printf("\n");
  printf("Features:\n");
  printf("  • Automatic packet parsing\n");
  printf("  • Checksum validation\n");
  printf("  • Detailed packet breakdown\n");
  printf("  • LCD display of sensor data\n");
  printf("  • Command sending to MCU1\n");
  printf("\n");
  printf("Commands:\n");
  printf("  request    - Request data from MCU1\n");
  printf("  led on/off - Control MCU1 LED\n");
  printf("  stats      - Show statistics\n");
  printf("  help       - Show help\n");
  printf("\n");
  printf("Waiting for packets from MCU1...\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  
  // Initial LCD
  dd_lcd_write(0, 0, "MCU2: Receiver");
  dd_lcd_write(1, 0, "Waiting...");
}

void loop_lab7_2_mcu2() {
  static char commandBuffer[64];
  static bool commandReady = false;
  
  // Non-blocking command reading from debug Serial
  if (Serial.available()) {
    static int bufferIndex = 0;
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        commandBuffer[bufferIndex] = '\0';
        commandReady = true;
      }
    } else if (bufferIndex < 63) {
      commandBuffer[bufferIndex++] = c;
    }
    
    if (commandReady) {
      processUserCommand(commandBuffer);
      bufferIndex = 0;
      commandReady = false;
      memset(commandBuffer, 0, sizeof(commandBuffer));
    }
  }
  
  // Receive and parse packets from COMM_SERIAL
  receiveAndParse();
  
  // Update LCD
  updateLCD();
}
#ifndef LAB7_2_MCU2_HPP
#define LAB7_2_MCU2_HPP

/**
 * @file lab7_2_mcu2.hpp
 * @brief Lab 7.2 MCU2 - Serial Protocol Receiver
 * 
 * MCU2 functionality:
 * - Receives packets from MCU1 via Serial
 * - Parses and validates packets
 * - Interprets sensor data
 * - Can send commands to MCU1 (REQUEST_DATA, SET_LED)
 */

void setup_lab7_2_mcu2();
void loop_lab7_2_mcu2();

#endif // LAB7_2_MCU2_HPP

