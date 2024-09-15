#include <gtest/gtest.h>
#include "AVC/Command.hpp"

using namespace RocketLink::AVC;

class CommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for each test
    }

    // Helper function to create a sample command
    Command createSampleCommand() {
        return Command(1, 2, CommandNumber::FIN_TEST, {0x01, 0x02, 0x03});
    }
};

TEST_F(CommandTest, ConstructorAndGetters) {
    Command cmd = createSampleCommand();

    EXPECT_EQ(cmd.getSenderID(), 1);
    EXPECT_EQ(cmd.getReceiverID(), 2);
    EXPECT_EQ(cmd.getCommandNumber(), CommandNumber::FIN_TEST);
    EXPECT_EQ(cmd.getPayloadDescriptor(), PayloadDescriptor::COMMAND);
    EXPECT_EQ(cmd.getPayload(), std::vector<uint8_t>({0x01, 0x02, 0x03}));
}

TEST_F(CommandTest, Setters) {
    Command cmd;
    cmd.setSenderID(3);
    cmd.setReceiverID(4);
    cmd.setCommandNumber(CommandNumber::FIN_TEST);
    cmd.setPayload({0x04, 0x05});

    EXPECT_EQ(cmd.getSenderID(), 3);
    EXPECT_EQ(cmd.getReceiverID(), 4);
    EXPECT_EQ(cmd.getCommandNumber(), CommandNumber::FIN_TEST);
    EXPECT_EQ(cmd.getPayload(), std::vector<uint8_t>({0x04, 0x05}));
}

TEST_F(CommandTest, Encode) {
    Command cmd = createSampleCommand();
    std::vector<uint8_t> encoded = cmd.encode();

    EXPECT_EQ(encoded.size(), 7);
    EXPECT_EQ(encoded[0], 0x12);  // Packed header (1 << 4 | 2)
    EXPECT_EQ(encoded[1], static_cast<uint8_t>(PayloadDescriptor::COMMAND));
    EXPECT_EQ(encoded[2], static_cast<uint8_t>(CommandNumber::FIN_TEST));
    EXPECT_EQ(encoded[3], 3);  // Payload length
    EXPECT_EQ(encoded[4], 0x01);
    EXPECT_EQ(encoded[5], 0x02);
    EXPECT_EQ(encoded[6], 0x03);
}

TEST_F(CommandTest, Decode) {
    std::vector<uint8_t> encoded = {0x12, 0x01, 101, 3, 0x01, 0x02, 0x03};
    Command cmd = Command::decode(encoded);

    EXPECT_EQ(cmd.getSenderID(), 1);
    EXPECT_EQ(cmd.getReceiverID(), 2);
    EXPECT_EQ(cmd.getCommandNumber(), CommandNumber::FIN_TEST);
    EXPECT_EQ(cmd.getPayload(), std::vector<uint8_t>({0x01, 0x02, 0x03}));
}

TEST_F(CommandTest, IsValid) {
    Command validCmd = createSampleCommand();
    EXPECT_TRUE(validCmd.isValid());

    Command invalidCmd;
    EXPECT_FALSE(invalidCmd.isValid());
}

TEST_F(CommandTest, EncodeLargePayload) {
    std::vector<uint8_t> largePayload(255, 0xFF);
    Command cmd(1, 2, CommandNumber::FIN_TEST, largePayload);
    std::vector<uint8_t> encoded = cmd.encode();

    EXPECT_EQ(encoded.size(), 259);  // 1 (header) + 1 (descriptor) + 1 (command) + 1 (length) + 255 (payload)
}

TEST_F(CommandTest, EncodePayloadTooLarge) {
    std::vector<uint8_t> tooLargePayload(256, 0xFF);
    Command cmd(1, 2, CommandNumber::FIN_TEST, tooLargePayload);

    EXPECT_THROW(cmd.encode(), std::length_error);
}

TEST_F(CommandTest, DecodeInvalidData) {
    std::vector<uint8_t> tooShort = {0x12, 0x01, 101};
    EXPECT_THROW(Command::decode(tooShort), std::invalid_argument);

    std::vector<uint8_t> invalidDescriptor = {0x12, 0x03, 101, 0};
    EXPECT_THROW(Command::decode(invalidDescriptor), std::invalid_argument);

    std::vector<uint8_t> incompletePaylod = {0x12, 0x01, 101, 3, 0x01};
    EXPECT_THROW(Command::decode(incompletePaylod), std::invalid_argument);
}

TEST_F(CommandTest, HeaderPackUnpack) {
    CommandHeader hdr{5, 7};
    uint8_t packed = hdr.pack();
    EXPECT_EQ(packed, 0x57);

    CommandHeader unpacked;
    unpacked.unpack(packed);
    EXPECT_EQ(unpacked.senderID, 5);
    EXPECT_EQ(unpacked.receiverID, 7);
}
