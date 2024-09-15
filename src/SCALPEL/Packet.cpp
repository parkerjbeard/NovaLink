#include "Packet.hpp"

namespace SCALPEL {

Packet::Packet() : payloadLength(0), payloadLengthChecksum(0), cobsIndex(0), cobsChecksum(0), checksum(0) {}

Packet::Packet(const std::vector<uint8_t>& payloadData) {
    if (payloadData.size() > MAX_PAYLOAD_LENGTH) {
        throw std::invalid_argument("Payload length exceeds maximum allowed size.");
    }
    payload = payloadData;
    payloadLength = static_cast<uint8_t>(payload.size());
    calculateChecksums();
}

std::vector<uint8_t> Packet::assemble() const {
    std::vector<uint8_t> packet;
    packet.push_back(START_BYTE);

    // Payload Length Byte
    uint8_t lengthByte = (payloadLength & 0x3F) << 2 | (payloadLengthChecksum & 0x03);
    packet.push_back(lengthByte);

    // COBS Encoding
    COBS cobs;
    COBS::COBSResult result = cobs.encode(payload);
    std::vector<uint8_t> encodedPayload = result.encodedPayload;

    // Ensure no byte equals START_BYTE after COBS
    for (const auto& byte : encodedPayload) {
        if (byte == START_BYTE) {
            throw std::runtime_error("COBS encoding failed to eliminate start byte from payload.");
        }
    }

    // COBS Byte
    uint8_t index = result.index; // Use the index from COBSResult
    uint8_t cobsChk = Checksum::calculate2BitChecksum(encodedPayload.data(), encodedPayload.size()) & 0x03;
    uint8_t cobsByte = (index & 0x3F) << 2 | (cobsChk & 0x03);
    packet.push_back(cobsByte);

    // Payload
    packet.insert(packet.end(), encodedPayload.begin(), encodedPayload.end());

    // Checksum Byte (CRC-8)
    uint8_t payloadChk = Checksum::calculateCRC8(payload.data(), payload.size());
    packet.push_back(payloadChk);

    return packet;
}

Packet Packet::disassemble(const std::vector<uint8_t>& data) {
    if (data.size() < 4) { // Minimum packet size
        throw std::invalid_argument("Data size too small to be a valid packet.");
    }

    if (data[0] != START_BYTE) {
        throw std::invalid_argument("Invalid start byte.");
    }

    Packet pkt;

    // Payload Length Byte
    uint8_t lengthByte = data[1];
    pkt.payloadLength = (lengthByte >> 2) & 0x3F;
    pkt.payloadLengthChecksum = lengthByte & 0x03;

    // Verify payload length checksum
    uint8_t expectedLengthChecksum = Checksum::calculate2BitChecksum(pkt.payloadLength);
    if (pkt.payloadLengthChecksum != expectedLengthChecksum) {
        throw std::runtime_error("Invalid payload length checksum.");
    }

    // COBS Byte
    uint8_t cobsByte = data[2];
    pkt.cobsIndex = (cobsByte >> 2) & 0x3F;
    pkt.cobsChecksum = cobsByte & 0x03;

    // Extract payload
    size_t payloadStart = 3;
    size_t payloadEnd = data.size() - 1; // Exclude checksum byte
    if (payloadEnd < payloadStart) {
        throw std::invalid_argument("Invalid payload boundaries.");
    }

    std::vector<uint8_t> encodedPayload(data.begin() + payloadStart, data.begin() + payloadEnd);

    // Verify COBS checksum
    uint8_t calculatedCobsChecksum = Checksum::calculate2BitChecksum(encodedPayload.data(), encodedPayload.size()) & 0x03;
    if (pkt.cobsChecksum != calculatedCobsChecksum) {
        throw std::runtime_error("Invalid COBS checksum.");
    }

    // COBS Decoding
    COBS cobs;
    pkt.payload = cobs.decode(encodedPayload, pkt.cobsIndex);

    if (pkt.payload.size() != pkt.payloadLength) {
        throw std::runtime_error("Payload length mismatch after decoding.");
    }

    // Verify payload checksum (CRC-8)
    pkt.checksum = data.back();
    uint8_t calculatedPayloadChecksum = Checksum::calculateCRC8(pkt.payload.data(), pkt.payload.size());
    if (pkt.checksum != calculatedPayloadChecksum) {
        throw std::runtime_error("Invalid payload checksum.");
    }

    return pkt;
}

uint8_t Packet::getPayloadLength() const {
    return payloadLength;
}

const std::vector<uint8_t>& Packet::getPayload() const {
    return payload;
}

void Packet::calculateChecksums() {
    // Calculate 2-bit checksum for payload length
    payloadLengthChecksum = Checksum::calculate2BitChecksum(payloadLength);

    // COBS checksum and payload checksum are handled in assemble
    // No additional actions required here
}

void Packet::validate() const {
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        throw std::invalid_argument("Payload length exceeds maximum allowed size.");
    }

    // Additional validations can be added here
}

} // namespace SCALPEL
