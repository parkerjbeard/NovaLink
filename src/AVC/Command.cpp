#include "Command.hpp"
#include <stdexcept>

namespace RocketLink {
namespace AVC {

Command::Command(uint8_t senderID, uint8_t receiverID, CommandNumber cmdNumber, const std::vector<uint8_t>& payloadData)
    : commandNumber(cmdNumber), payload(payloadData), priority(0) {
    header.senderID = senderID;
    header.receiverID = receiverID;
}

std::vector<uint8_t> Command::encode() const {
    std::vector<uint8_t> encoded;
    // Header: Sender and Receiver IDs packed into one byte
    encoded.push_back(header.pack());

    // Payload Descriptor
    encoded.push_back(static_cast<uint8_t>(PayloadDescriptor::COMMAND));

    // Command Number
    encoded.push_back(static_cast<uint8_t>(commandNumber));

    // Payload Length
    if (payload.size() > 255) {
        throw std::length_error("Payload size exceeds maximum allowed length of 255 bytes.");
    }
    encoded.push_back(static_cast<uint8_t>(payload.size()));

    // Payload Data
    encoded.insert(encoded.end(), payload.begin(), payload.end());

    return encoded;
}

Command Command::decode(const std::vector<uint8_t>& data) {
    if (data.size() < 4) { // Minimum size: header + descriptor + command number + payload length
        throw std::invalid_argument("Data too short to decode Command.");
    }

    CommandHeader hdr;
    hdr.unpack(data[0]);

    PayloadDescriptor descriptor = static_cast<PayloadDescriptor>(data[1]);
    if (descriptor != PayloadDescriptor::COMMAND) {
        throw std::invalid_argument("Invalid Payload Descriptor for Command.");
    }

    CommandNumber cmdNumber = static_cast<CommandNumber>(data[2]);

    uint8_t payloadLength = data[3];
    if (data.size() < 4 + payloadLength) {
        throw std::invalid_argument("Data does not contain full payload.");
    }

    std::vector<uint8_t> payloadData(data.begin() + 4, data.begin() + 4 + payloadLength);

    return Command(hdr.senderID, hdr.receiverID, cmdNumber, payloadData);
}

CommandNumber Command::getCommandNumber() const {
    return commandNumber;
}

PayloadDescriptor Command::getPayloadDescriptor() const {
    return PayloadDescriptor::COMMAND;
}

const std::vector<uint8_t>& Command::getPayload() const {
    return payload;
}

void Command::setCommandNumber(CommandNumber cmdNumber) {
    commandNumber = cmdNumber;
}

void Command::setPayload(const std::vector<uint8_t>& payloadData) {
    if (payloadData.size() > 255) {
        throw std::length_error("Payload size exceeds maximum allowed length of 255 bytes.");
    }
    payload = payloadData;
}

} // namespace AVC
} // namespace RocketLink
