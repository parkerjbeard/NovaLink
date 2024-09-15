#ifndef CHECKSUM_HPP
#define CHECKSUM_HPP

#include <cstdint>
#include <cstddef>

namespace SCALPEL {

/**
 * @class Checksum
 * @brief Provides functions to calculate and validate checksums for error detection.
 */
class Checksum {
public:
    /**
     * @brief Calculate CRC-8 checksum for the given data.
     * 
     * @param data Pointer to data buffer.
     * @param length Number of bytes in data.
     * @return Calculated CRC-8 checksum.
     */
    static uint8_t calculateCRC8(const uint8_t* data, size_t length);

    /**
     * @brief Validate CRC-8 checksum for the given data.
     * 
     * @param checksum The received CRC-8 checksum.
     * @param data Pointer to data buffer.
     * @param length Number of bytes in data.
     * @return True if checksum is valid, false otherwise.
     */
    static bool validateCRC8(uint8_t checksum, const uint8_t* data, size_t length);

    /**
     * @brief Calculate 2-bit checksum for a single byte.
     * 
     * The 2-bit checksum is calculated as the number of set bits modulo 4.
     * 
     * @param byte The input byte.
     * @return Calculated 2-bit checksum.
     */
    static uint8_t calculate2BitChecksum(uint8_t byte);

    /**
     * @brief Validate 2-bit checksum for a single byte.
     * 
     * @param expected The expected 2-bit checksum.
     * @param byte The input byte.
     * @return True if checksum is valid, false otherwise.
     */
    static bool validate2BitChecksum(uint8_t expected, uint8_t byte);

    /**
     * @brief Calculate 2-bit checksum for multiple bytes.
     * 
     * The 2-bit checksum is the sum of set bits in all bytes modulo 4.
     * 
     * @param data Pointer to data buffer.
     * @param length Number of bytes in data.
     * @return Calculated 2-bit checksum.
     */
    static uint8_t calculate2BitChecksum(const uint8_t* data, size_t length);
};

} // namespace SCALPEL

#endif // CHECKSUM_HPP
