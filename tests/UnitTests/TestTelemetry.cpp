#include <gtest/gtest.h>
#include "AVC/Telemetry.hpp"
#include <vector>
#include <cstring>

using namespace RocketLink::AVC;

class TelemetryTest : public ::testing::Test {
protected:
    Telemetry defaultTelemetry;
    Telemetry sampleTelemetry;

    void SetUp() override {
        uint8_t memLog[3] = {10, 20, 30};
        sampleTelemetry = Telemetry(1, 2, TelemetryDescriptor::TELEMETRY_A,
                                    1000, 2000, 100, 200, 300,
                                    10, 20, 30, 1, 2, 3, memLog, 0x0F);
    }
};

TEST_F(TelemetryTest, DefaultConstructor) {
    EXPECT_EQ(defaultTelemetry.getSenderID(), 0);
    EXPECT_EQ(defaultTelemetry.getReceiverID(), 0);
    EXPECT_EQ(defaultTelemetry.getDescriptor(), TelemetryDescriptor::TELEMETRY_A);
    EXPECT_EQ(defaultTelemetry.getVoltage1(), 0);
    EXPECT_EQ(defaultTelemetry.getVoltage2(), 0);
    EXPECT_EQ(defaultTelemetry.getPosX(), 0);
    EXPECT_EQ(defaultTelemetry.getPosY(), 0);
    EXPECT_EQ(defaultTelemetry.getPosZ(), 0);
    EXPECT_EQ(defaultTelemetry.getVelX(), 0);
    EXPECT_EQ(defaultTelemetry.getVelY(), 0);
    EXPECT_EQ(defaultTelemetry.getVelZ(), 0);
    EXPECT_EQ(defaultTelemetry.getAccX(), 0);
    EXPECT_EQ(defaultTelemetry.getAccY(), 0);
    EXPECT_EQ(defaultTelemetry.getAccZ(), 0);
    EXPECT_EQ(defaultTelemetry.getStatusFlags(), 0);

    uint8_t memLog[3];
    defaultTelemetry.getMemoryLog(memLog);
    EXPECT_EQ(memLog[0], 0);
    EXPECT_EQ(memLog[1], 0);
    EXPECT_EQ(memLog[2], 0);
}

TEST_F(TelemetryTest, ParameterizedConstructor) {
    EXPECT_EQ(sampleTelemetry.getSenderID(), 1);
    EXPECT_EQ(sampleTelemetry.getReceiverID(), 2);
    EXPECT_EQ(sampleTelemetry.getDescriptor(), TelemetryDescriptor::TELEMETRY_A);
    EXPECT_EQ(sampleTelemetry.getVoltage1(), 1000);
    EXPECT_EQ(sampleTelemetry.getVoltage2(), 2000);
    EXPECT_EQ(sampleTelemetry.getPosX(), 100);
    EXPECT_EQ(sampleTelemetry.getPosY(), 200);
    EXPECT_EQ(sampleTelemetry.getPosZ(), 300);
    EXPECT_EQ(sampleTelemetry.getVelX(), 10);
    EXPECT_EQ(sampleTelemetry.getVelY(), 20);
    EXPECT_EQ(sampleTelemetry.getVelZ(), 30);
    EXPECT_EQ(sampleTelemetry.getAccX(), 1);
    EXPECT_EQ(sampleTelemetry.getAccY(), 2);
    EXPECT_EQ(sampleTelemetry.getAccZ(), 3);
    EXPECT_EQ(sampleTelemetry.getStatusFlags(), 0x0F);

    uint8_t memLog[3];
    sampleTelemetry.getMemoryLog(memLog);
    EXPECT_EQ(memLog[0], 10);
    EXPECT_EQ(memLog[1], 20);
    EXPECT_EQ(memLog[2], 30);
}

TEST_F(TelemetryTest, Setters) {
    defaultTelemetry.setSenderID(3);
    defaultTelemetry.setReceiverID(4);
    defaultTelemetry.setDescriptor(TelemetryDescriptor::TELEMETRY_B);
    defaultTelemetry.setVoltage1(3000);
    defaultTelemetry.setVoltage2(4000);
    defaultTelemetry.setPosX(400);
    defaultTelemetry.setPosY(500);
    defaultTelemetry.setPosZ(600);
    defaultTelemetry.setVelX(40);
    defaultTelemetry.setVelY(50);
    defaultTelemetry.setVelZ(60);
    defaultTelemetry.setAccX(4);
    defaultTelemetry.setAccY(5);
    defaultTelemetry.setAccZ(6);
    uint8_t newMemLog[3] = {40, 50, 60};
    defaultTelemetry.setMemoryLog(newMemLog);
    defaultTelemetry.setStatusFlags(0xF0);

    EXPECT_EQ(defaultTelemetry.getSenderID(), 3);
    EXPECT_EQ(defaultTelemetry.getReceiverID(), 4);
    EXPECT_EQ(defaultTelemetry.getDescriptor(), TelemetryDescriptor::TELEMETRY_B);
    EXPECT_EQ(defaultTelemetry.getVoltage1(), 3000);
    EXPECT_EQ(defaultTelemetry.getVoltage2(), 4000);
    EXPECT_EQ(defaultTelemetry.getPosX(), 400);
    EXPECT_EQ(defaultTelemetry.getPosY(), 500);
    EXPECT_EQ(defaultTelemetry.getPosZ(), 600);
    EXPECT_EQ(defaultTelemetry.getVelX(), 40);
    EXPECT_EQ(defaultTelemetry.getVelY(), 50);
    EXPECT_EQ(defaultTelemetry.getVelZ(), 60);
    EXPECT_EQ(defaultTelemetry.getAccX(), 4);
    EXPECT_EQ(defaultTelemetry.getAccY(), 5);
    EXPECT_EQ(defaultTelemetry.getAccZ(), 6);
    EXPECT_EQ(defaultTelemetry.getStatusFlags(), 0xF0);

    uint8_t memLog[3];
    defaultTelemetry.getMemoryLog(memLog);
    EXPECT_EQ(memLog[0], 40);
    EXPECT_EQ(memLog[1], 50);
    EXPECT_EQ(memLog[2], 60);
}

TEST_F(TelemetryTest, EncodeDecode) {
    std::vector<uint8_t> encoded = sampleTelemetry.encode();
    EXPECT_EQ(encoded.size(), 28); // Expected size of encoded data

    Telemetry decoded = Telemetry::decode(encoded);

    EXPECT_EQ(decoded.getSenderID(), sampleTelemetry.getSenderID());
    EXPECT_EQ(decoded.getReceiverID(), sampleTelemetry.getReceiverID());
    EXPECT_EQ(decoded.getDescriptor(), sampleTelemetry.getDescriptor());
    EXPECT_EQ(decoded.getVoltage1(), sampleTelemetry.getVoltage1());
    EXPECT_EQ(decoded.getVoltage2(), sampleTelemetry.getVoltage2());
    EXPECT_EQ(decoded.getPosX(), sampleTelemetry.getPosX());
    EXPECT_EQ(decoded.getPosY(), sampleTelemetry.getPosY());
    EXPECT_EQ(decoded.getPosZ(), sampleTelemetry.getPosZ());
    EXPECT_EQ(decoded.getVelX(), sampleTelemetry.getVelX());
    EXPECT_EQ(decoded.getVelY(), sampleTelemetry.getVelY());
    EXPECT_EQ(decoded.getVelZ(), sampleTelemetry.getVelZ());
    EXPECT_EQ(decoded.getAccX(), sampleTelemetry.getAccX());
    EXPECT_EQ(decoded.getAccY(), sampleTelemetry.getAccY());
    EXPECT_EQ(decoded.getAccZ(), sampleTelemetry.getAccZ());
    EXPECT_EQ(decoded.getStatusFlags(), sampleTelemetry.getStatusFlags());

    uint8_t decodedMemLog[3], sampleMemLog[3];
    decoded.getMemoryLog(decodedMemLog);
    sampleTelemetry.getMemoryLog(sampleMemLog);
    EXPECT_EQ(memcmp(decodedMemLog, sampleMemLog, 3), 0);
}

TEST_F(TelemetryTest, DecodeInvalidData) {
    std::vector<uint8_t> invalidData(27, 0); // Too short
    EXPECT_THROW(Telemetry::decode(invalidData), std::invalid_argument);
}

TEST_F(TelemetryTest, HeaderPacking) {
    TelemetryHeader header{0x0A, 0x0B};
    uint8_t packed = header.pack();
    EXPECT_EQ(packed, 0xAB);

    TelemetryHeader unpacked;
    unpacked.unpack(packed);
    EXPECT_EQ(unpacked.senderID, 0x0A);
    EXPECT_EQ(unpacked.receiverID, 0x0B);
}