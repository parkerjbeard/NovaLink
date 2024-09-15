#ifndef ROCKETLINK_AVC_COMMAND_HPP
#define ROCKETLINK_AVC_COMMAND_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>

namespace RocketLink {
namespace AVC {

/**
 * @brief Enum representing unique command numbers.
 */
enum class CommandNumber : uint8_t {
    INVALID = 0,
    FIN_TEST = 101,
    // Add other commands here
};

/**
 * @brief Enum representing payload descriptors.
 */
enum class PayloadDescriptor : uint8_t {
    COMMAND = 0x01,
    ACKNOWLEDGMENT = 0x02,
    // Add other descriptors here
};

/**
 * @brief Structure representing the sender and receiver IDs packed into a single byte.
 */
struct CommandHeader {
    uint8_t senderID : 4;
    uint8_t receiverID : 4;

    /**
     * @brief Packs the senderID and receiverID into a single byte.
     * @return Packed byte.
     */
    uint8_t pack() const {
        return (receiverID << 4) | (senderID & 0x0F);
    }

    /**
     * @brief Unpacks a single byte into senderID and receiverID.
     * @param byte The byte to unpack.
     */
    void unpack(uint8_t byte) {
        senderID = byte & 0x0F;
        receiverID = (byte >> 4) & 0x0F;
    }
};

/**
 * @brief Class representing a Command.
 */
class Command {
public:
    /**
     * @brief Default constructor for Command.
     * Initializes the command to an invalid state.
     */
    Command() : header{0, 0}, commandNumber(CommandNumber::INVALID), priority(0) {}

    /**
     * @brief Constructs a Command with specified parameters.
     * @param senderID ID of the sender.
     * @param receiverID ID of the receiver.
     * @param commandNumber Unique command number.
     * @param payload Command-specific payload.
     */
    Command(uint8_t senderID, uint8_t receiverID, CommandNumber commandNumber, const std::vector<uint8_t>& payload);

    /**
     * @brief Encodes the command into a byte vector for transmission.
     * @return Encoded byte vector.
     */
    std::vector<uint8_t> encode() const;

    /**
     * @brief Decodes a byte vector into a Command object.
     * @param data The byte vector to decode.
     * @return Decoded Command object.
     * @throws std::invalid_argument if data is invalid.
     */
    static Command decode(const std::vector<uint8_t>& data);

    // Getters
    uint8_t getSenderID() const { return header.senderID; }
    uint8_t getReceiverID() const { return header.receiverID; }
    CommandNumber getCommandNumber() const;
    PayloadDescriptor getPayloadDescriptor() const;
    const std::vector<uint8_t>& getPayload() const;
    int getPriority() const { return priority; }

    // Setters
    void setSenderID(uint8_t senderID) { header.senderID = senderID & 0x0F; }
    void setReceiverID(uint8_t receiverID) { header.receiverID = receiverID & 0x0F; }
    void setCommandNumber(CommandNumber commandNumber);
    void setPayload(const std::vector<uint8_t>& payload);

    /**
     * @brief Checks if the command is valid.
     * @return true if the command is valid, false otherwise.
     */
    bool isValid() const {
        return commandNumber != CommandNumber::INVALID;
    }

private:
    CommandHeader header;
    CommandNumber commandNumber;
    std::vector<uint8_t> payload;
    int priority;
};

} // namespace AVC
} // namespace RocketLink

#endif // ROCKETLINK_AVC_COMMAND_HPP
