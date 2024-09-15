#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>
#include "COBS.hpp"
#include "Checksum.hpp"

namespace SCALPEL {

class Packet {
public:
    static constexpr uint8_t START_BYTE = 170;
    static constexpr uint8_t MAX_PAYLOAD_LENGTH = 28;

    Packet();
    Packet(const std::vector<uint8_t>& payload);

    // Assemble the packet into a byte array
    std::vector<uint8_t> assemble() const;

    // Disassemble the packet from a byte array
    static Packet disassemble(const std::vector<uint8_t>& data);

    // Getters
    uint8_t getPayloadLength() const;
    const std::vector<uint8_t>& getPayload() const;

    // New method to get payload as vector
    std::vector<uint8_t> getPayloadVector() const { return payload; }

private:
    uint8_t payloadLength;
    uint8_t payloadLengthChecksum;
    uint8_t cobsIndex;
    uint8_t cobsChecksum;
    std::vector<uint8_t> payload;
    uint8_t checksum;

    void calculateChecksums();
    void validate() const;
};

} // namespace SCALPEL

#endif // PACKET_HPP
