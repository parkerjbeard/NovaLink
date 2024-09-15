#include <gtest/gtest.h>
#include "SCALPEL/Packet.hpp"

namespace SCALPEL {
namespace {

class PacketTest : public ::testing::Test {
protected:
    const std::vector<uint8_t> samplePayload = {0x01, 0x02, 0x03, 0x04, 0x05};
};

TEST_F(PacketTest, DefaultConstructor) {
    Packet packet;
    EXPECT_EQ(packet.getPayloadLength(), 0);
    EXPECT_TRUE(packet.getPayload().empty());
}

TEST_F(PacketTest, PayloadConstructor) {
    Packet packet(samplePayload);
    EXPECT_EQ(packet.getPayloadLength(), samplePayload.size());
    EXPECT_EQ(packet.getPayload(), samplePayload);
}

TEST_F(PacketTest, PayloadConstructorMaxLength) {
    std::vector<uint8_t> maxPayload(Packet::MAX_PAYLOAD_LENGTH, 0xFF);
    Packet packet(maxPayload);
    EXPECT_EQ(packet.getPayloadLength(), Packet::MAX_PAYLOAD_LENGTH);
}

TEST_F(PacketTest, PayloadConstructorExceedsMaxLength) {
    std::vector<uint8_t> oversizedPayload(Packet::MAX_PAYLOAD_LENGTH + 1, 0xFF);
    EXPECT_THROW(Packet packet(oversizedPayload), std::invalid_argument);
}

TEST_F(PacketTest, AssembleAndDisassemble) {
    Packet originalPacket(samplePayload);
    std::vector<uint8_t> assembledData = originalPacket.assemble();
    Packet reconstructedPacket = Packet::disassemble(assembledData);
    
    EXPECT_EQ(reconstructedPacket.getPayloadLength(), originalPacket.getPayloadLength());
    EXPECT_EQ(reconstructedPacket.getPayload(), originalPacket.getPayload());
}

TEST_F(PacketTest, AssembleStartByte) {
    Packet packet(samplePayload);
    std::vector<uint8_t> assembledData = packet.assemble();
    EXPECT_EQ(assembledData[0], Packet::START_BYTE);
}

TEST_F(PacketTest, DisassembleInvalidStartByte) {
    std::vector<uint8_t> invalidData = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_THROW(Packet::disassemble(invalidData), std::invalid_argument);
}

TEST_F(PacketTest, DisassembleTooShort) {
    std::vector<uint8_t> shortData = {Packet::START_BYTE, 0x01, 0x02};
    EXPECT_THROW(Packet::disassemble(shortData), std::invalid_argument);
}

TEST_F(PacketTest, AssemblePayloadLengthAndChecksum) {
    Packet packet(samplePayload);
    std::vector<uint8_t> assembledData = packet.assemble();
    uint8_t lengthByte = assembledData[1];
    uint8_t extractedLength = (lengthByte >> 2) & 0x3F;
    uint8_t extractedChecksum = lengthByte & 0x03;
    
    EXPECT_EQ(extractedLength, samplePayload.size());
    EXPECT_EQ(extractedChecksum, Checksum::calculate2BitChecksum(samplePayload.size()));
}

TEST_F(PacketTest, DisassembleInvalidPayloadLengthChecksum) {
    std::vector<uint8_t> invalidData = {Packet::START_BYTE, 0x15, 0x00, 0x01, 0x02, 0x03};
    EXPECT_THROW(Packet::disassemble(invalidData), std::runtime_error);
}

TEST_F(PacketTest, AssembleCOBSEncoding) {
    std::vector<uint8_t> payloadWithStartByte = {0x01, Packet::START_BYTE, 0x03};
    Packet packet(payloadWithStartByte);
    std::vector<uint8_t> assembledData = packet.assemble();
    
    // Ensure START_BYTE is not present in the encoded payload
    auto it = std::find(assembledData.begin() + 3, assembledData.end() - 1, Packet::START_BYTE);
    EXPECT_EQ(it, assembledData.end() - 1);
}

TEST_F(PacketTest, DisassembleCOBSDecoding) {
    std::vector<uint8_t> payloadWithStartByte = {0x01, Packet::START_BYTE, 0x03};
    Packet originalPacket(payloadWithStartByte);
    std::vector<uint8_t> assembledData = originalPacket.assemble();
    Packet reconstructedPacket = Packet::disassemble(assembledData);
    
    EXPECT_EQ(reconstructedPacket.getPayload(), payloadWithStartByte);
}

TEST_F(PacketTest, AssemblePayloadChecksum) {
    Packet packet(samplePayload);
    std::vector<uint8_t> assembledData = packet.assemble();
    uint8_t payloadChecksum = assembledData.back();
    
    EXPECT_EQ(payloadChecksum, Checksum::calculateCRC8(samplePayload.data(), samplePayload.size()));
}

TEST_F(PacketTest, DisassembleInvalidPayloadChecksum) {
    Packet packet(samplePayload);
    std::vector<uint8_t> assembledData = packet.assemble();
    assembledData.back() ^= 0xFF;  // Flip all bits in the checksum
    
    EXPECT_THROW(Packet::disassemble(assembledData), std::runtime_error);
}

TEST_F(PacketTest, GetPayloadVector) {
    Packet packet(samplePayload);
    std::vector<uint8_t> retrievedPayload = packet.getPayloadVector();
    EXPECT_EQ(retrievedPayload, samplePayload);
}

TEST_F(PacketTest, EmptyPayload) {
    std::vector<uint8_t> emptyPayload;
    Packet packet(emptyPayload);
    std::vector<uint8_t> assembledData = packet.assemble();
    Packet reconstructedPacket = Packet::disassemble(assembledData);
    
    EXPECT_EQ(reconstructedPacket.getPayloadLength(), 0);
    EXPECT_TRUE(reconstructedPacket.getPayload().empty());
}

}  // namespace
}  // namespace SCALPEL
