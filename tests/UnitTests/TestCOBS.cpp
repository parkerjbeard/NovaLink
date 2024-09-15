#include <gtest/gtest.h>
#include "SCALPEL/COBS.hpp"
#include "SCALPEL/Packet.hpp"

using namespace SCALPEL;

class COBSTest : public ::testing::Test {
protected:
    COBS cobs;
};

TEST_F(COBSTest, EncodeEmptyInput) {
    std::vector<uint8_t> input;
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload, (std::vector<uint8_t>{1}));
    EXPECT_EQ(result.index, 0);
}

TEST_F(COBSTest, EncodeNoStartByte) {
    std::vector<uint8_t> input{1, 2, 3, 4, 5};
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload, (std::vector<uint8_t>{6, 1, 2, 3, 4, 5}));
    EXPECT_EQ(result.index, 0);
}

TEST_F(COBSTest, EncodeWithStartByte) {
    std::vector<uint8_t> input{1, 2, Packet::START_BYTE, 3, 4};
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload, (std::vector<uint8_t>{3, 1, 2, 1, 3, 4}));
    EXPECT_EQ(result.index, 1);
}

TEST_F(COBSTest, EncodeWithMultipleStartBytes) {
    std::vector<uint8_t> input{Packet::START_BYTE, 1, Packet::START_BYTE, 2, Packet::START_BYTE};
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload, (std::vector<uint8_t>{1, 2, 1, 3, 1, 2, 1}));
    EXPECT_EQ(result.index, 3);
}

TEST_F(COBSTest, EncodeWithMaxRunLength) {
    std::vector<uint8_t> input(254, 1);
    auto result = cobs.encode(input);
    std::vector<uint8_t> expected(256);
    expected[0] = 255;
    std::fill(expected.begin() + 1, expected.end() - 1, 1);
    expected[255] = 1;
    EXPECT_EQ(result.encodedPayload, expected);
    EXPECT_EQ(result.index, 0);
}

TEST_F(COBSTest, DecodeEmptyInput) {
    std::vector<uint8_t> encoded{1};
    auto decoded = cobs.decode(encoded, 0);
    EXPECT_TRUE(decoded.empty());
}

TEST_F(COBSTest, DecodeNoStartByte) {
    std::vector<uint8_t> encoded{6, 1, 2, 3, 4, 5};
    auto decoded = cobs.decode(encoded, 0);
    EXPECT_EQ(decoded, (std::vector<uint8_t>{1, 2, 3, 4, 5}));
}

TEST_F(COBSTest, DecodeWithStartByte) {
    std::vector<uint8_t> encoded{3, 1, 2, 1, 3, 4};
    auto decoded = cobs.decode(encoded, 1);
    EXPECT_EQ(decoded, (std::vector<uint8_t>{1, 2, Packet::START_BYTE, 3, 4}));
}

TEST_F(COBSTest, DecodeWithMultipleStartBytes) {
    std::vector<uint8_t> encoded{1, 2, 1, 3, 1, 2, 1};
    auto decoded = cobs.decode(encoded, 3);
    EXPECT_EQ(decoded, (std::vector<uint8_t>{Packet::START_BYTE, 1, Packet::START_BYTE, 2, Packet::START_BYTE}));
}

TEST_F(COBSTest, DecodeWithMaxRunLength) {
    std::vector<uint8_t> encoded(256);
    encoded[0] = 255;
    std::fill(encoded.begin() + 1, encoded.end() - 1, 1);
    encoded[255] = 1;
    auto decoded = cobs.decode(encoded, 0);
    EXPECT_EQ(decoded, std::vector<uint8_t>(254, 1));
}

TEST_F(COBSTest, EncodeDecodeRoundTrip) {
    std::vector<uint8_t> input{1, 2, Packet::START_BYTE, 3, 4, Packet::START_BYTE, 5, 6};
    auto encoded = cobs.encode(input);
    auto decoded = cobs.decode(encoded.encodedPayload, encoded.index);
    EXPECT_EQ(input, decoded);
}

TEST_F(COBSTest, DecodeInvalidZeroCode) {
    std::vector<uint8_t> encoded{3, 1, 2, 0, 3, 4};
    EXPECT_THROW(cobs.decode(encoded, 1), std::runtime_error);
}

TEST_F(COBSTest, DecodeInvalidNotEnoughBytes) {
    std::vector<uint8_t> encoded{5, 1, 2, 3};
    EXPECT_THROW(cobs.decode(encoded, 0), std::runtime_error);
}

TEST_F(COBSTest, DecodeInvalidIndexMismatch) {
    std::vector<uint8_t> encoded{3, 1, 2, 1, 3, 4};
    EXPECT_THROW(cobs.decode(encoded, 0), std::runtime_error);
    EXPECT_THROW(cobs.decode(encoded, 2), std::runtime_error);
}

TEST_F(COBSTest, EncodeWithStartByteAtEnd) {
    std::vector<uint8_t> input{1, 2, 3, Packet::START_BYTE};
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload, (std::vector<uint8_t>{4, 1, 2, 3, 1}));
    EXPECT_EQ(result.index, 1);
}

TEST_F(COBSTest, EncodeAllStartBytes) {
    std::vector<uint8_t> input(10, Packet::START_BYTE);
    auto result = cobs.encode(input);
    std::vector<uint8_t> expected(11, 1);
    EXPECT_EQ(result.encodedPayload, expected);
    EXPECT_EQ(result.index, 10);
}

TEST_F(COBSTest, EncodeLargeInput) {
    std::vector<uint8_t> input(1000, 1);
    input[500] = Packet::START_BYTE;
    auto result = cobs.encode(input);
    EXPECT_EQ(result.encodedPayload.size(), 1003);
    EXPECT_EQ(result.index, 1);
}
