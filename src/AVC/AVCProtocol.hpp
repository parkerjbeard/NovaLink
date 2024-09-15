#ifndef ROCKETLINK_AVC_AVCPROTOCOL_HPP
#define ROCKETLINK_AVC_AVCPROTOCOL_HPP

#include "Command.hpp"
#include "Telemetry.hpp"
#include "SCALPEL/Communicator.hpp"
#include "SCALPEL/COBS.hpp"
#include "SCALPEL/Packet.hpp"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <chrono>
#include <memory>
#include <thread>
#include <condition_variable>
#include <functional>
#include <iostream>

namespace RocketLink {
namespace AVC {

/**
 * @brief Class implementing the AVC protocol logic.
 */
class AVCProtocol {
public:
    /**
     * @brief Constructs the AVCProtocol with dependencies.
     * @param communicator Pointer to the SCALPEL Communicator interface.
     */
    AVCProtocol(std::shared_ptr<SCALPEL::Communicator> communicator);

    /**
     * @brief Destructor to clean up resources.
     */
    ~AVCProtocol();

    /**
     * @brief Encodes and sends a Command.
     * @param command The Command object to send.
     */
    void sendCommand(const Command& command);

    /**
     * @brief Encodes and sends Telemetry data.
     * @param telemetry The Telemetry object to send.
     */
    void sendTelemetry(const Telemetry& telemetry);

    /**
     * @brief Starts the protocol's internal processes.
     */
    void start();

    /**
     * @brief Stops the protocol's internal processes.
     */
    void stop();

    /**
     * @brief Encodes a Command.
     * @param command The Command to encode.
     * @return The encoded Command data.
     */
    std::vector<uint8_t> encodeCommand(const Command& command);

    /**
     * @brief Decodes telemetry data from a SCALPEL::Packet.
     * @param packet The SCALPEL::Packet containing telemetry data.
     * @return The decoded Telemetry object.
     */
    Telemetry decodeTelemetry(const SCALPEL::Packet& packet);

private:
    /**
     * @brief Handles incoming packets by decoding them.
     * @param data The raw packet data.
     */
    void handleIncomingPacket(const std::vector<uint8_t>& data);

    /**
     * @brief Handles acknowledgment messages.
     * @param ackCommandNumber The command number being acknowledged.
     */
    void handleAcknowledgment(uint8_t ackCommandNumber);

    /**
     * @brief Manages retransmission of unacknowledged commands.
     */
    void retransmissionHandler();

    /**
     * @brief Sends a raw packet via the SCALPEL layer.
     * @param data The raw packet data.
     */
    void sendRawPacket(const std::vector<uint8_t>& data);

    /**
     * @brief Processes received data from the Communicator.
     * @param data The received raw data.
     */
    void onDataReceived(const std::vector<uint8_t>& data);

    /**
     * @brief Registers payload descriptors with their handling functions.
     */
    void registerPayloadDescriptors();

    // Dependency on SCALPEL Communicator
    std::shared_ptr<SCALPEL::Communicator> communicator;
    SCALPEL::COBS cobsEncoder;
    SCALPEL::COBS cobsDecoder;

    // Mapping of payload descriptors to handler functions
    std::unordered_map<uint8_t, std::function<void(const std::vector<uint8_t>&)>> descriptorHandlers;

    // Mutex for thread safety
    std::mutex protocolMutex;

    // Command acknowledgment tracking
    struct PendingCommand {
        Command command;
        std::chrono::steady_clock::time_point timestamp;
        int retryCount;

        PendingCommand() : retryCount(0) {}
    };

    std::unordered_map<uint8_t, PendingCommand> pendingCommands;
    std::mutex pendingMutex;

    // Retransmission settings
    const int maxRetries = 5;
    const std::chrono::milliseconds retryInterval = std::chrono::milliseconds(500);

    // Thread management
    std::thread retransThread;
    std::atomic<bool> running;
    std::condition_variable cv;
    std::mutex cvMutex;

    /**
     * @brief Calculates the CRC-8 checksum for the given data.
     * 
     * @param data Pointer to the data buffer.
     * @param length Number of bytes in data.
     * @return Calculated CRC-8 checksum.
     */
    uint8_t calculateChecksum(const uint8_t* data, size_t length);

};

} // namespace AVC
} // namespace RocketLink

#endif // ROCKETLINK_AVC_AVCPROTOCOL_HPP
