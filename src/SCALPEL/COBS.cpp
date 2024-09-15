#include "COBS.hpp"
#include "Packet.hpp"
#include "Checksum.hpp"

namespace SCALPEL {

COBS::COBSResult COBS::encode(const std::vector<uint8_t>& input) const {
    COBSResult result;
    std::vector<uint8_t> encoded;
    encoded.reserve(input.size() + 2); // Estimate extra bytes

    size_t code_ptr = 0;
    encoded.push_back(0x00); // Placeholder for first code
    code_ptr = encoded.size() - 1;
    uint8_t code = 1;
    uint8_t index = 0;

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == Packet::START_BYTE) {
            encoded[code_ptr] = code;
            code = 1;
            encoded.push_back(0x00); // Placeholder for next code
            code_ptr = encoded.size() - 1;
            index++;
        } else {
            encoded.push_back(input[i]);
            code++;
            if (code == 0xFF) { // Maximum code value
                encoded[code_ptr] = code;
                code = 1;
                encoded.push_back(0x00); // Placeholder for next code
                code_ptr = encoded.size() - 1;
            }
        }
    }

    encoded[code_ptr] = code;

    result.encodedPayload = encoded;
    result.index = index;

    // Verify that no byte in encodedPayload equals START_BYTE
    for (const auto& byte : result.encodedPayload) {
        if (byte == Packet::START_BYTE) {
            throw std::runtime_error("COBS encoding failed to eliminate start byte from payload.");
        }
    }

    return result;
}

std::vector<uint8_t> COBS::decode(const std::vector<uint8_t>& encoded, uint8_t index) const {
    std::vector<uint8_t> decoded;
    decoded.reserve(encoded.size());

    size_t i = 0;

    while (i < encoded.size()) {
        uint8_t code = encoded[i];
        if (code == 0) {
            throw std::runtime_error("Invalid COBS encoding: code byte is zero.");
        }
        i++;
        for (uint8_t j = 1; j < code; j++) {
            if (i >= encoded.size()) {
                throw std::runtime_error("Invalid COBS encoding: not enough bytes.");
            }
            decoded.push_back(encoded[i]);
            i++;
        }
        if (code < 0xFF && i < encoded.size()) {
            decoded.push_back(Packet::START_BYTE);
            if (index == 0) {
                throw std::runtime_error("COBS decode: unexpected start byte.");
            }
            index--;
        }
    }

    if (index != 0) {
        throw std::runtime_error("COBS decode: index mismatch.");
    }

    return decoded;
}

} // namespace SCALPEL
