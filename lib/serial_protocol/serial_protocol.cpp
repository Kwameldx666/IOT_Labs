#include "serial_protocol.hpp"
#include <string.h>

// ============================================================================
// SerialPacket Implementation
// ============================================================================

SerialPacket::SerialPacket()
    : start(SERIAL_PKT_START)
    , packetNumber(0)
    , sourceID(0)
    , destinationID(0)
    , packetType(0)
    , payloadLength(0)
    , checksum(0)
    , end(SERIAL_PKT_END) {
  memset(payload, 0, sizeof(payload));
}

SerialPacket::SerialPacket(uint8_t pktNum, uint8_t src, uint8_t dst, uint8_t type)
    : start(SERIAL_PKT_START)
    , packetNumber(pktNum)
    , sourceID(src)
    , destinationID(dst)
    , packetType(type)
    , payloadLength(0)
    , checksum(0)
    , end(SERIAL_PKT_END) {
  memset(payload, 0, sizeof(payload));
}

bool SerialPacket::setPayload(const uint8_t* data, uint8_t len) {
  if (len > SERIAL_PKT_MAX_PAYLOAD) {
    return false;
  }
  
  payloadLength = len;
  memcpy(payload, data, len);
  updateChecksum();
  return true;
}

uint8_t SerialPacket::calculateChecksum() const {
  uint8_t sum = 0;
  sum += packetNumber;
  sum += sourceID;
  sum += destinationID;
  sum += packetType;
  sum += payloadLength;
  
  for (uint8_t i = 0; i < payloadLength; i++) {
    sum += payload[i];
  }
  
  return sum;
}

void SerialPacket::updateChecksum() {
  checksum = calculateChecksum();
}

bool SerialPacket::isValid() const {
  // Check markers
  if (start != SERIAL_PKT_START || end != SERIAL_PKT_END) {
    return false;
  }
  
  // Check payload length
  if (payloadLength > SERIAL_PKT_MAX_PAYLOAD) {
    return false;
  }
  
  // Verify checksum
  return (checksum == calculateChecksum());
}

uint8_t SerialPacket::serialize(uint8_t* buffer) const {
  uint8_t idx = 0;
  
  buffer[idx++] = start;
  buffer[idx++] = packetNumber;
  buffer[idx++] = sourceID;
  buffer[idx++] = destinationID;
  buffer[idx++] = packetType;
  buffer[idx++] = payloadLength;
  
  for (uint8_t i = 0; i < payloadLength; i++) {
    buffer[idx++] = payload[i];
  }
  
  buffer[idx++] = checksum;
  buffer[idx++] = end;
  
  return idx;
}

// ============================================================================
// SerialProtocolParser Implementation
// ============================================================================

SerialProtocolParser::SerialProtocolParser()
    : m_state(ParserState::WAIT_START)
    , m_index(0)
    , m_packetReady(false) {
}

void SerialProtocolParser::reset() {
  m_state = ParserState::WAIT_START;
  m_index = 0;
  m_packetReady = false;
  m_packet = SerialPacket();
}

bool SerialProtocolParser::feedByte(uint8_t byte) {
  m_packetReady = false;
  
  switch (m_state) {
    case ParserState::WAIT_START:
      if (byte == SERIAL_PKT_START) {
        m_packet.start = byte;
        m_state = ParserState::READ_HEADER;
        m_index = 0;
      }
      break;
      
    case ParserState::READ_HEADER:
      switch (m_index) {
        case 0: m_packet.packetNumber = byte; break;
        case 1: m_packet.sourceID = byte; break;
        case 2: m_packet.destinationID = byte; break;
        case 3: m_packet.packetType = byte; break;
        case 4:
          m_packet.payloadLength = byte;
          if (m_packet.payloadLength > SERIAL_PKT_MAX_PAYLOAD) {
            reset();  // Invalid length
            return false;
          }
          m_index = 0;
          if (m_packet.payloadLength > 0) {
            m_state = ParserState::READ_PAYLOAD;
          } else {
            m_state = ParserState::READ_CHECKSUM;
          }
          return false;
      }
      m_index++;
      break;
      
    case ParserState::READ_PAYLOAD:
      m_packet.payload[m_index++] = byte;
      if (m_index >= m_packet.payloadLength) {
        m_state = ParserState::READ_CHECKSUM;
      }
      break;
      
    case ParserState::READ_CHECKSUM:
      m_packet.checksum = byte;
      m_state = ParserState::READ_END;
      break;
      
    case ParserState::READ_END:
      m_packet.end = byte;
      
      // Validate complete packet
      if (m_packet.isValid()) {
        m_packetReady = true;
        m_state = ParserState::WAIT_START;
        return true;
      } else {
        reset();  // Invalid packet
      }
      break;
  }
  
  return false;
}
#ifndef SERIAL_PROTOCOL_HPP
#define SERIAL_PROTOCOL_HPP

#include <stdint.h>

/**
 * @file serial_protocol.hpp
 * @brief Serial Communication Protocol with Structured Packets
 * 
 * Packet Format:
 * [START] [PKT_NUM] [SRC_ID] [DST_ID] [TYPE] [LEN] [PAYLOAD...] [CHECKSUM] [END]
 * 
 * Fields:
 * - START: Start marker (0x7E)
 * - PKT_NUM: Packet number (uint8_t, incremental)
 * - SRC_ID: Source device ID
 * - DST_ID: Destination device ID
 * - TYPE: Packet type/command
 * - LEN: Payload length
 * - PAYLOAD: Data (0-32 bytes)
 * - CHECKSUM: Sum of all numeric fields
 * - END: End marker (0x7F)
 */

#define SERIAL_PKT_START 0x7E
#define SERIAL_PKT_END 0x7F
#define SERIAL_PKT_MAX_PAYLOAD 32
#define SERIAL_PKT_OVERHEAD 8  // START + header fields + CHECKSUM + END

/**
 * @struct SerialPacket
 * @brief Serial protocol packet structure
 */
struct SerialPacket {
  uint8_t start;
  uint8_t packetNumber;
  uint8_t sourceID;
  uint8_t destinationID;
  uint8_t packetType;
  uint8_t payloadLength;
  uint8_t payload[SERIAL_PKT_MAX_PAYLOAD];
  uint8_t checksum;
  uint8_t end;
  
  /**
   * @brief Default constructor
   */
  SerialPacket();
  
  /**
   * @brief Constructor with parameters
   */
  SerialPacket(uint8_t pktNum, uint8_t src, uint8_t dst, uint8_t type);
  
  /**
   * @brief Set payload data
   * @param data Payload data
   * @param len Length of payload
   * @return true if successful
   */
  bool setPayload(const uint8_t* data, uint8_t len);
  
  /**
   * @brief Calculate checksum
   * @return Checksum value
   */
  uint8_t calculateChecksum() const;
  
  /**
   * @brief Update checksum
   */
  void updateChecksum();
  
  /**
   * @brief Validate packet
   * @return true if valid
   */
  bool isValid() const;
  
  /**
   * @brief Serialize to byte array
   * @param buffer Output buffer
   * @return Number of bytes written
   */
  uint8_t serialize(uint8_t* buffer) const;
  
  /**
   * @brief Get total packet size
   * @return Size in bytes
   */
  uint8_t getSize() const { return SERIAL_PKT_OVERHEAD + payloadLength; }
};

/**
 * @class SerialProtocolParser
 * @brief Parser for receiving serial packets
 */
class SerialProtocolParser {
public:
  SerialProtocolParser();
  
  /**
   * @brief Reset parser state
   */
  void reset();
  
  /**
   * @brief Feed byte to parser
   * @param byte Incoming byte
   * @return true if complete packet received
   */
  bool feedByte(uint8_t byte);
  
  /**
   * @brief Get received packet
   * @return Reference to packet
   */
  const SerialPacket& getPacket() const { return m_packet; }
  
  /**
   * @brief Check if packet is ready
   * @return true if complete and valid packet received
   */
  bool isPacketReady() const { return m_packetReady; }

private:
  enum class ParserState {
    WAIT_START,
    READ_HEADER,
    READ_PAYLOAD,
    READ_CHECKSUM,
    READ_END
  };
  
  SerialPacket m_packet;
  ParserState m_state;
  uint8_t m_index;
  bool m_packetReady;
};

#endif // SERIAL_PROTOCOL_HPP

