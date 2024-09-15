#ifndef COBS_HPP
#define COBS_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>

namespace SCALPEL {

class COBS {
public:
    // Structure to hold the COBS encoding result
    struct COBSResult {
        std::vector<uint8_t> encodedPayload;
        uint8_t index; // Number of START_BYTE (170) replaced
    };

    // Encode the input data using COBS
    // Returns encoded payload and index
    COBSResult encode(const std::vector<uint8_t>& input) const;

    // Decode the input data using COBS
    // Takes encoded payload and index, returns decoded payload
    std::vector<uint8_t> decode(const std::vector<uint8_t>& encoded, uint8_t index) const;
};

} // namespace SCALPEL

#endif // COBS_HPP
