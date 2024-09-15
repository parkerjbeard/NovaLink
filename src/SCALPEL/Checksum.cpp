#include "Checksum.hpp"

namespace SCALPEL {

uint8_t Checksum::calculateCRC8(const uint8_t* data, size_t length) {
    uint8_t crc = 0x00; // Initial value
    const uint8_t polynomial = 0x07; // CRC-8 polynomial x^8 + x^2 + x + 1

    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

bool Checksum::validateCRC8(uint8_t checksum, const uint8_t* data, size_t length) {
    uint8_t calculated = calculateCRC8(data, length);
    return calculated == checksum;
}

uint8_t Checksum::calculate2BitChecksum(uint8_t byte) {
    uint8_t bitCount = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        if (byte & (1 << i)) {
            ++bitCount;
        }
    }
    return bitCount % 4;
}

bool Checksum::validate2BitChecksum(uint8_t expected, uint8_t byte) {
    uint8_t calculated = calculate2BitChecksum(byte);
    return calculated == (expected & 0x03); // Ensure only 2 bits are compared
}

uint8_t Checksum::calculate2BitChecksum(const uint8_t* data, size_t length) {
    uint8_t totalBitCount = 0;
    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (byte & (1 << bit)) {
                ++totalBitCount;
            }
        }
    }
    return totalBitCount % 4;
}

} // namespace SCALPEL
