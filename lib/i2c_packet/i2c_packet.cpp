#include "i2c_packet.hpp"

I2CPacket::I2CPacket()
    : header(I2C_PACKET_HEADER)
    , dataType(PacketDataType::DISTANCE_CM)
    , distance(0)
    , reserved(0)
    , checksum(0)
    , footer(I2C_PACKET_FOOTER) {
}

I2CPacket::I2CPacket(uint16_t dist, PacketDataType type)
    : header(I2C_PACKET_HEADER)
    , dataType(type)
    , distance(dist)
    , reserved(0)
    , checksum(0)
    , footer(I2C_PACKET_FOOTER) {
  checksum = calculateChecksum();
}

uint8_t I2CPacket::calculateChecksum() const {
  // Simple XOR checksum
  uint8_t sum = 0;
  sum ^= header;
  sum ^= static_cast<uint8_t>(dataType);
  sum ^= (distance >> 8) & 0xFF;  // High byte
  sum ^= distance & 0xFF;         // Low byte
  sum ^= (reserved >> 8) & 0xFF;
  sum ^= reserved & 0xFF;
  sum ^= footer;
  return sum;
}

bool I2CPacket::isValid() const {
  // Check header and footer
  if (header != I2C_PACKET_HEADER || footer != I2C_PACKET_FOOTER) {
    return false;
  }
  
  // Verify checksum
  uint8_t expectedChecksum = calculateChecksum();
  return (checksum == expectedChecksum);
}

void I2CPacket::serialize(uint8_t* buffer) const {
  buffer[0] = header;
  buffer[1] = static_cast<uint8_t>(dataType);
  buffer[2] = (distance >> 8) & 0xFF;  // High byte (big-endian)
  buffer[3] = distance & 0xFF;         // Low byte
  buffer[4] = (reserved >> 8) & 0xFF;
  buffer[5] = reserved & 0xFF;
  buffer[6] = checksum;
  buffer[7] = footer;
}

bool I2CPacket::deserialize(const uint8_t* buffer) {
  header = buffer[0];
  dataType = static_cast<PacketDataType>(buffer[1]);
  distance = (static_cast<uint16_t>(buffer[2]) << 8) | buffer[3];  // Big-endian
  reserved = (static_cast<uint16_t>(buffer[4]) << 8) | buffer[5];
  checksum = buffer[6];
  footer = buffer[7];
  
  return isValid();
}
#ifndef I2C_PACKET_HPP
#define I2C_PACKET_HPP

#include <stdint.h>

/**
 * @file i2c_packet.hpp
 * @brief I²C Data Packet Encoding/Decoding
 * 
 * Packet Format (8 bytes):
 * [0] Header (0xAA)
 * [1] Data Type
 * [2-3] Distance (uint16_t, big-endian)
 * [4-5] Reserved/Extra data
 * [6] Checksum
 * [7] Footer (0x55)
 */

#define I2C_PACKET_HEADER 0xAA
#define I2C_PACKET_FOOTER 0x55
#define I2C_PACKET_SIZE 8

/**
 * @enum PacketDataType
 * @brief Type of data in packet
 */
enum class PacketDataType : uint8_t {
  DISTANCE_CM = 0x01,
  DISTANCE_INCH = 0x02,
  ERROR = 0xFF
};

/**
 * @struct I2CPacket
 * @brief I²C data packet structure
 */
struct I2CPacket {
  uint8_t header;
  PacketDataType dataType;
  uint16_t distance;
  uint16_t reserved;
  uint8_t checksum;
  uint8_t footer;
  
  /**
   * @brief Default constructor
   */
  I2CPacket();
  
  /**
   * @brief Constructor with distance
   * @param dist Distance value
   * @param type Data type
   */
  I2CPacket(uint16_t dist, PacketDataType type = PacketDataType::DISTANCE_CM);
  
  /**
   * @brief Calculate checksum
   * @return Checksum value
   */
  uint8_t calculateChecksum() const;
  
  /**
   * @brief Validate packet
   * @return true if valid
   */
  bool isValid() const;
  
  /**
   * @brief Serialize to byte array
   * @param buffer Output buffer (must be >= I2C_PACKET_SIZE)
   */
  void serialize(uint8_t* buffer) const;
  
  /**
   * @brief Deserialize from byte array
   * @param buffer Input buffer
   * @return true if deserialization successful
   */
  bool deserialize(const uint8_t* buffer);
};

#endif // I2C_PACKET_HPP

