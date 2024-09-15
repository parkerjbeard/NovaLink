#ifndef ROCKETLINK_RADIO_RADIOINTERFACE_HPP
#define ROCKETLINK_RADIO_RADIOINTERFACE_HPP

#include "SCALPEL/Packet.hpp"
#include <cstdint>
#include <string>
#include <mutex>
#include <exception>
#include <memory>

namespace RocketLink {
namespace Radio {

/**
 * @brief Struct representing the configuration parameters for the radio module.
 */
struct RadioConfig {
    uint32_t frequencyHz;      ///< Operating frequency in Hertz.
    uint32_t baudRate;         ///< Communication speed in bits per second.
    uint8_t powerLevel;        ///< Transmission power level (e.g., 0-100).
    uint8_t channel;           ///< Radio channel to operate on.
    std::string mode;          ///< Operating mode (e.g., "AFSK", "FSK", "OOK").

    RadioConfig()
        : frequencyHz(915000000), baudRate(57600), powerLevel(50), channel(1), mode("AFSK") {}
};

/**
 * @brief Struct representing the status metrics of the radio module.
 */
struct RadioStatus {
    bool isInitialized;        ///< Indicates if the radio is initialized.
    uint32_t packetsSent;      ///< Total number of packets sent.
    uint32_t packetsReceived;  ///< Total number of packets received.
    uint32_t transmissionErrors; ///< Number of transmission errors.
    uint32_t receptionErrors;  ///< Number of reception errors.
    int32_t signalStrength;    ///< Current signal strength (RSSI).

    RadioStatus()
        : isInitialized(false), packetsSent(0), packetsReceived(0),
          transmissionErrors(0), receptionErrors(0), signalStrength(0) {}
};

/**
 * @brief Exception class for radio-related errors.
 */
class RadioException : public std::exception {
public:
    /**
     * @brief Constructs a RadioException with a specific error message.
     * @param message The error message.
     */
    explicit RadioException(const std::string& message)
        : msg_("RadioException: " + message) {}

    /**
     * @brief Retrieves the error message.
     * @return Error message as a C-string.
     */
    virtual const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

/**
 * @brief Abstract class defining a generic interface for radio modules.
 */
class RadioInterface {
public:
    virtual ~RadioInterface() = default;

    /**
     * @brief Prepares the radio module for operation.
     * @throws RadioException if initialization fails.
     */
    virtual void initialize() = 0;

    /**
     * @brief Sends a SCALPEL packet over the radio.
     * @param packet The packet to send.
     * @throws RadioException if sending fails.
     */
    virtual void sendPacket(const SCALPEL::Packet& packet) = 0;

    /**
     * @brief Receives a SCALPEL packet from the radio.
     * @param packet The packet received.
     * @return true if a packet was successfully received, false otherwise.
     * @throws RadioException if reception fails.
     */
    virtual bool receivePacket(SCALPEL::Packet& packet) = 0;

    /**
     * @brief Sets radio parameters based on the provided configuration.
     * @param config The configuration parameters.
     * @throws RadioException if configuration fails.
     */
    virtual void configure(const RadioConfig& config) = 0;

    /**
     * @brief Retrieves radio status metrics.
     * @param status The structure to populate with status metrics.
     * @throws RadioException if status retrieval fails.
     */
    virtual void getStatus(RadioStatus& status) = 0;

    // Prevent copying and assignment
    RadioInterface(const RadioInterface&) = delete;
    RadioInterface& operator=(const RadioInterface&) = delete;

protected:
    RadioInterface() = default;
};

} // namespace Radio
} // namespace RocketLink

#endif // ROCKETLINK_RADIO_RADIOINTERFACE_HPP
