#ifndef ROCKETLINK_AVC_TELEMETRY_HPP
#define ROCKETLINK_AVC_TELEMETRY_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cstring> // Added for memset in constructor

namespace RocketLink {
namespace AVC {

/**
 * @brief Enum representing unique telemetry descriptors.
 */
enum class TelemetryDescriptor : uint8_t {
    TELEMETRY_A = 0x10,
    TELEMETRY_B = 0x11,
};

/**
 * @brief Structure representing the sender and receiver IDs packed into a single byte.
 */
struct TelemetryHeader {
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
 * @brief Class representing Telemetry data.
 */
class Telemetry {
public:
    Telemetry()
        : header{0, 0}, descriptor(TelemetryDescriptor::TELEMETRY_A),
          voltage1(0), voltage2(0),
          posX(0), posY(0), posZ(0),
          velX(0), velY(0), velZ(0),
          accX(0), accY(0), accZ(0),
          statusFlags(0) {
        std::memset(memoryLog, 0, sizeof(memoryLog));
    }

    /**
     * @brief Constructs Telemetry with specified parameters.
     * @param senderID ID of the sender.
     * @param receiverID ID of the receiver.
     * @param descriptor Telemetry descriptor type.
     * @param voltage1 Ground power input in millivolts.
     * @param voltage2 Flight battery in millivolts.
     * @param posX X-coordinate in meters.
     * @param posY Y-coordinate in meters.
     * @param posZ Z-coordinate in meters.
     * @param velX X-velocity in decimeters per second.
     * @param velY Y-velocity in decimeters per second.
     * @param velZ Z-velocity in decimeters per second.
     * @param accX X-acceleration in centimeters per second squared.
     * @param accY Y-acceleration in centimeters per second squared.
     * @param accZ Z-acceleration in centimeters per second squared.
     * @param memoryLog Major, communication, and data log percentages.
     * @param statusFlags Packed boolean status flags.
     */
    Telemetry(uint8_t senderID,
              uint8_t receiverID,
              TelemetryDescriptor descriptor,
              uint16_t voltage1,
              uint16_t voltage2,
              int16_t posX,
              int16_t posY,
              int16_t posZ,
              int16_t velX,
              int16_t velY,
              int16_t velZ,
              int16_t accX,
              int16_t accY,
              int16_t accZ,
              uint8_t memoryLog[3],
              uint8_t statusFlags);

    /**
     * @brief Encodes the telemetry data into a byte vector for transmission.
     * @return Encoded byte vector.
     */
    std::vector<uint8_t> encode() const;

    /**
     * @brief Decodes a byte vector into a Telemetry object.
     * @param data The byte vector to decode.
     * @return Decoded Telemetry object.
     * @throws std::invalid_argument if data is invalid.
     */
    static Telemetry decode(const std::vector<uint8_t>& data);

    // Getters (Only senderID and receiverID are defined inline)
    uint8_t getSenderID() const { return header.senderID; }
    uint8_t getReceiverID() const { return header.receiverID; }

    uint16_t getVoltage1() const;
    uint16_t getVoltage2() const;
    int16_t getPosX() const;
    int16_t getPosY() const;
    int16_t getPosZ() const;
    int16_t getVelX() const;
    int16_t getVelY() const;
    int16_t getVelZ() const;
    int16_t getAccX() const;
    int16_t getAccY() const;
    int16_t getAccZ() const;
    void getMemoryLog(uint8_t (&memoryLogOut)[3]) const;
    uint8_t getStatusFlags() const;
    TelemetryDescriptor getDescriptor() const; // Moved below to match declarations

    // Setters (Only senderID and receiverID are defined inline)
    void setSenderID(uint8_t senderID) { header.senderID = senderID & 0x0F; }
    void setReceiverID(uint8_t receiverID) { header.receiverID = receiverID & 0x0F; }

    // Setters for other fields
    void setDescriptor(TelemetryDescriptor descriptorType);
    void setVoltage1(uint16_t voltage1);
    void setVoltage2(uint16_t voltage2);
    void setPosX(int16_t posX);
    void setPosY(int16_t posY);
    void setPosZ(int16_t posZ);
    void setVelX(int16_t velX);
    void setVelY(int16_t velY);
    void setVelZ(int16_t velZ);
    void setAccX(int16_t accX);
    void setAccY(int16_t accY);
    void setAccZ(int16_t accZ);
    void setMemoryLog(const uint8_t memoryLogIn[3]);
    void setStatusFlags(uint8_t statusFlags);

private:
    TelemetryHeader header;
    TelemetryDescriptor descriptor;

    uint16_t voltage1; // Millivolts
    uint16_t voltage2; // Millivolts

    int16_t posX; // Meters
    int16_t posY; // Meters
    int16_t posZ; // Meters

    int16_t velX; // Decimeters per second
    int16_t velY; // Decimeters per second
    int16_t velZ; // Decimeters per second

    int16_t accX; // Centimeters per second squared
    int16_t accY; // Centimeters per second squared
    int16_t accZ; // Centimeters per second squared

    uint8_t memoryLog[3]; // Percentages
    uint8_t statusFlags;   // Packed boolean flags
};

} // namespace AVC
} // namespace RocketLink

#endif // ROCKETLINK_AVC_TELEMETRY_HPP