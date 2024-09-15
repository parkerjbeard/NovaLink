#include <gtest/gtest.h>
#include "SCALPEL/Checksum.hpp"

namespace SCALPEL {
namespace {

class ChecksumTest : public ::testing::Test {
protected:
    const uint8_t testData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
};

TEST_F(ChecksumTest, CalculateCRC8) {
    uint8_t crc = Checksum::calculateCRC8(testData, sizeof(testData));
    EXPECT_EQ(crc, 0x5B);  // Pre-calculated expected CRC-8 value
}

TEST_F(ChecksumTest, ValidateCRC8) {
    uint8_t correctCRC = Checksum::calculateCRC8(testData, sizeof(testData));
    EXPECT_TRUE(Checksum::validateCRC8(correctCRC, testData, sizeof(testData)));
    
    uint8_t incorrectCRC = correctCRC ^ 0xFF;  // Flip all bits to make it incorrect
    EXPECT_FALSE(Checksum::validateCRC8(incorrectCRC, testData, sizeof(testData)));
}

TEST_F(ChecksumTest, Calculate2BitChecksumSingleByte) {
    EXPECT_EQ(Checksum::calculate2BitChecksum(0x00), 0);  // No bits set
    EXPECT_EQ(Checksum::calculate2BitChecksum(0xFF), 0);  // All bits set (8 % 4 = 0)
    EXPECT_EQ(Checksum::calculate2BitChecksum(0x0F), 0);  // 4 bits set
    EXPECT_EQ(Checksum::calculate2BitChecksum(0x01), 1);  // 1 bit set
    EXPECT_EQ(Checksum::calculate2BitChecksum(0x03), 2);  // 2 bits set
    EXPECT_EQ(Checksum::calculate2BitChecksum(0x07), 3);  // 3 bits set
}

TEST_F(ChecksumTest, Validate2BitChecksumSingleByte) {
    EXPECT_TRUE(Checksum::validate2BitChecksum(0, 0x00));
    EXPECT_TRUE(Checksum::validate2BitChecksum(0, 0xFF));
    EXPECT_TRUE(Checksum::validate2BitChecksum(1, 0x01));
    EXPECT_TRUE(Checksum::validate2BitChecksum(2, 0x03));
    EXPECT_TRUE(Checksum::validate2BitChecksum(3, 0x07));
    
    EXPECT_FALSE(Checksum::validate2BitChecksum(1, 0x00));
    EXPECT_FALSE(Checksum::validate2BitChecksum(2, 0x01));
    EXPECT_FALSE(Checksum::validate2BitChecksum(3, 0x03));
}

TEST_F(ChecksumTest, Calculate2BitChecksumMultipleBytes) {
    EXPECT_EQ(Checksum::calculate2BitChecksum(testData, sizeof(testData)), 1);  // Pre-calculated expected value
    
    const uint8_t allZeros[4] = {0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(Checksum::calculate2BitChecksum(allZeros, sizeof(allZeros)), 0);
    
    const uint8_t allOnes[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(Checksum::calculate2BitChecksum(allOnes, sizeof(allOnes)), 0);
    
    const uint8_t mixedData[4] = {0x01, 0x03, 0x07, 0x0F};
    EXPECT_EQ(Checksum::calculate2BitChecksum(mixedData, sizeof(mixedData)), 2);
}

TEST_F(ChecksumTest, EdgeCases) {
    // Test with empty data
    EXPECT_EQ(Checksum::calculateCRC8(nullptr, 0), 0x00);
    EXPECT_EQ(Checksum::calculate2BitChecksum(nullptr, 0), 0);
    
    // Test with single byte
    uint8_t singleByte = 0xAA;
    EXPECT_EQ(Checksum::calculateCRC8(&singleByte, 1), 0x4B);  // Pre-calculated CRC-8 for 0xAA
    EXPECT_EQ(Checksum::calculate2BitChecksum(&singleByte, 1), 0);
}

}  // namespace
}  // namespace SCALPEL
