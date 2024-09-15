#ifndef ROCKETLINK_CORE_ROCKETLINK_HPP
#define ROCKETLINK_CORE_ROCKETLINK_HPP

#include "AVC/AVCProtocol.hpp"
#include "SCALPEL/Packet.hpp"
#include "PhysicalLayer/RadioInterface.hpp"
#include "Management/CommandManager.hpp"
#include "Management/TelemetryBuffer.hpp"
#include "Diagnostics/Diagnostics.hpp"
#include "API/Callbacks.hpp"
#include "Utils/Logger.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace RocketLink {
namespace Core {

/**
 * @brief The RocketLink class serves as the main interface for the RocketLink package.
 *        It provides methods for initializing the communication system, sending commands,
 *        receiving telemetry data, and handling event callbacks.
 */
class RocketLink {
public:
    /**
     * @brief Constructs a RocketLink instance with the specified radio interface.
     * @param radio A shared pointer to a RadioInterface implementation.
     */
    explicit RocketLink(std::shared_ptr<Radio::RadioInterface> radio);

    /**
     * @brief Destructor to clean up resources and stop internal threads.
     */
    ~RocketLink();

    /**
     * @brief Initializes all components, including the selected radio module.
     *        Sets up necessary threads for handling communication.
     * @return true on successful initialization, false otherwise.
     */
    bool initialize();

    /**
     * @brief Queues a command for transmission to the rocket.
     * @param cmd The Command object to send.
     * @return true if the command is successfully queued, false otherwise.
     */
    bool sendCommand(const AVC::Command& cmd);

    /**
     * @brief Allows users to register callback functions for various events.
     * @param callbacks Pointer to a Callbacks instance containing user-defined callbacks.
     */
    void registerCallbacks(API::Callbacks* callbacks);

    /**
     * @brief Provides access to the latest telemetry data received from the rocket.
     * @return The most recent Telemetry object.
     */
    AVC::Telemetry getTelemetry();

private:
    /**
     * @brief The main loop for sending commands from the CommandManager.
     *        Encodes commands, packetizes them, and transmits via the RadioInterface.
     */
    void sendLoop();

    /**
     * @brief The main loop for receiving telemetry data.
     *        Listens for incoming packets, decodes telemetry, and triggers callbacks.
     */
    void receiveLoop();

    /**
     * @brief Handles events such as new telemetry data, command acknowledgments, or errors.
     *        Invokes the corresponding user-defined callback functions.
     * @param event The event type to handle.
     */
    void handleEvent(const std::string& event);

    // Component instances
    std::shared_ptr<AVC::AVCProtocol> avcProtocol;                                         ///< Manages AVC protocol operations
    SCALPEL::Packet packetHandler;                                                    ///< Handles SCALPEL packet operations
    std::shared_ptr<Radio::RadioInterface> radio;                        ///< Abstracted radio interface
    std::shared_ptr<AVC::CommandManager> commandManager;                           ///< Manages command queue and retransmissions
    AVC::TelemetryBuffer telemetryBuffer;                         ///< Buffers incoming telemetry data
    Diagnostics::Diagnostics diagnostics;                                 ///< Collects diagnostic information
    API::Callbacks* userCallbacks;                                       ///< User-registered callbacks
    Logger& logger;                                                 ///< Logger instance for logging events

    // Thread management
    std::thread sendThread;                                                            ///< Thread for sending commands
    std::thread receiveThread;                                                         ///< Thread for receiving telemetry
    std::atomic<bool> isRunning;                                                       ///< Flag to control the running state

    // Synchronization primitives
    std::mutex callbackMutex;                                                          ///< Mutex for protecting userCallbacks
    std::mutex telemetryMutex;                                                         ///< Mutex for accessing telemetry data
    std::condition_variable sendCondition;                                            ///< Condition variable for sending thread
    std::mutex sendMutex; // Added as per the code block to apply changes from

};

} // namespace Core
} // namespace RocketLink

#endif // ROCKETLINK_CORE_ROCKETLINK_HPP