/**
 * @file lab7_1_mcu1.cpp
 * @brief Lab 7.1 MCU1 - I²C Slave with HC-SR04 Sensor
 * 
 * Functionality:
 * - Reads HC-SR04 ultrasonic sensor periodically
 * - Packages distance data into I²C packets
 * - Acts as I²C slave, responding to read requests
 * - Reports status via Serial (STDIO)
 * 
 * Architecture:
 * - UltrasonicHCSR04: Sensor reading
 * - I2CPacket: Data packaging
 * - Wire (I²C slave): Communication
 */

#include "lab7_1_mcu1.hpp"

#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>

#include "config.h"
#include "dd_serial.hpp"
#include "ultrasonic_hcsr04.hpp"
#include "i2c_packet.hpp"

// ============================================================================
// Hardware & State
// ============================================================================

namespace {
  UltrasonicHCSR04 ultrasonic(LAB71_ULTRASONIC_TRIG_PIN,
                               LAB71_ULTRASONIC_ECHO_PIN,
                               LAB71_ULTRASONIC_MAX_DISTANCE);
  
  I2CPacket currentPacket;
  uint8_t packetBuffer[I2C_PACKET_SIZE];
  
  unsigned long lastMeasurement = 0;
  unsigned long lastReport = 0;
  uint32_t measurementCount = 0;
  uint32_t i2cRequestCount = 0;
}

// ============================================================================
// I²C Slave Callbacks
// ============================================================================

void onI2CRequest() {
  // MCU2 is requesting data
  i2cRequestCount++;
  
  // Send current packet
  Wire.write(packetBuffer, I2C_PACKET_SIZE);
  
  printf("[I2C] Request #%lu - Sent distance: %u cm\n",
         (unsigned long)i2cRequestCount,
         currentPacket.distance);
}

void onI2CReceive(int numBytes) {
  // Not used in this application (slave only sends data)
  // Clear buffer
  while (Wire.available()) {
    Wire.read();
  }
}

// ============================================================================
// Sensor Reading
// ============================================================================

void updateSensorReading() {
  unsigned long now = millis();
  if (now - lastMeasurement < LAB71_MEASUREMENT_INTERVAL_MS) {
    return;
  }
  lastMeasurement = now;
  
  // Measure distance
  uint16_t distance = ultrasonic.measureDistanceCm();
  measurementCount++;
  
  // Create packet
  if (distance > 0) {
    currentPacket = I2CPacket(distance, PacketDataType::DISTANCE_CM);
    printf("[SENSOR] Measurement #%lu: %u cm\n", 
           (unsigned long)measurementCount, distance);
  } else {
    currentPacket = I2CPacket(0, PacketDataType::ERROR);
    printf("[SENSOR] Measurement #%lu: ERROR (timeout or out of range)\n",
           (unsigned long)measurementCount);
  }
  
  // Serialize packet to buffer (ready for I²C transmission)
  currentPacket.serialize(packetBuffer);
}

// ============================================================================
// Status Report
// ============================================================================

void printStatusReport() {
  unsigned long now = millis();
  if (now - lastReport < LAB71_REPORT_PERIOD_MS) {
    return;
  }
  lastReport = now;
  
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║     MCU1 Status Report (I²C Slave)                ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("I²C Configuration:\n");
  printf("  Role:            SLAVE\n");
  printf("  Address:         0x%02X\n", LAB71_I2C_ADDRESS);
  printf("  Clock:           %lu Hz\n", (unsigned long)LAB71_I2C_CLOCK);
  printf("\n");
  printf("Sensor Status:\n");
  printf("  Type:            HC-SR04 Ultrasonic\n");
  printf("  Trigger Pin:     %d\n", LAB71_ULTRASONIC_TRIG_PIN);
  printf("  Echo Pin:        %d\n", LAB71_ULTRASONIC_ECHO_PIN);
  printf("  Max Range:       %d cm\n", LAB71_ULTRASONIC_MAX_DISTANCE);
  printf("  Current Reading: %u cm\n", currentPacket.distance);
  printf("\n");
  printf("Statistics:\n");
  printf("  Measurements:    %lu\n", (unsigned long)measurementCount);
  printf("  I²C Requests:    %lu\n", (unsigned long)i2cRequestCount);
  printf("  Packet Valid:    %s\n", currentPacket.isValid() ? "YES" : "NO");
  printf("\n");
  printf("Current Packet (hex):\n");
  printf("  ");
  for (int i = 0; i < I2C_PACKET_SIZE; i++) {
    printf("%02X ", packetBuffer[i]);
  }
  printf("\n");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab7_1_mcu1() {
  // Initialize Serial
  SerialBegin();
  
  // Initialize sensor
  ultrasonic.begin();
  
  // Initialize I²C as slave
  Wire.begin(LAB71_I2C_ADDRESS);
  Wire.onRequest(onI2CRequest);
  Wire.onReceive(onI2CReceive);
  
  // Initial measurement
  uint16_t distance = ultrasonic.measureDistanceCm();
  currentPacket = I2CPacket(distance > 0 ? distance : 0, 
                            distance > 0 ? PacketDataType::DISTANCE_CM : PacketDataType::ERROR);
  currentPacket.serialize(packetBuffer);
  
  // Print welcome
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 7.1 MCU1: I²C Slave - Ultrasonic Sensor\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Role:              I²C SLAVE\n");
  printf("I²C Address:       0x%02X\n", LAB71_I2C_ADDRESS);
  printf("Sensor:            HC-SR04 Ultrasonic\n");
  printf("Measurement Rate:  %lu ms\n", (unsigned long)LAB71_MEASUREMENT_INTERVAL_MS);
  printf("\n");
  printf("Packet Format:\n");
  printf("  [0] Header:      0x%02X\n", I2C_PACKET_HEADER);
  printf("  [1] Data Type:   0x01 (CM) / 0xFF (ERROR)\n");
  printf("  [2-3] Distance:  uint16_t (big-endian)\n");
  printf("  [4-5] Reserved:  0x0000\n");
  printf("  [6] Checksum:    XOR of all bytes\n");
  printf("  [7] Footer:      0x%02X\n", I2C_PACKET_FOOTER);
  printf("\n");
  printf("Features:\n");
  printf("  • Continuous sensor polling\n");
  printf("  • Automatic packet encoding\n");
  printf("  • Checksum validation\n");
  printf("  • I²C slave response on request\n");
  printf("\n");
  printf("Waiting for I²C requests from MCU2...\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
}

void loop_lab7_1_mcu1() {
  // Update sensor reading
  updateSensorReading();
  
  // Print periodic status
  printStatusReport();
}
#ifndef LAB7_1_MCU1_HPP
#define LAB7_1_MCU1_HPP

/**
 * @file lab7_1_mcu1.hpp
 * @brief Lab 7.1 MCU1 - I²C Slave (Ultrasonic Sensor Interface)
 * 
 * MCU1 acts as I²C slave:
 * - Continuously reads HC-SR04 ultrasonic sensor
 * - Packages data into I²C packets
 * - Responds to I²C read requests from MCU2
 */

void setup_lab7_1_mcu1();
void loop_lab7_1_mcu1();

#endif // LAB7_1_MCU1_HPP

