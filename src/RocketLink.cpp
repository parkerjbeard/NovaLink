#include "RocketLink.hpp"
#include "AVC/AVCProtocol.hpp"

namespace RocketLink {
namespace Core {

RocketLink::RocketLink(std::shared_ptr<Radio::RadioInterface> radioInterface)
    : avcProtocol(std::make_shared<AVC::AVCProtocol>(std::make_shared<SCALPEL::Communicator>(
          [](const std::vector<uint8_t>& /*data*/) {
              // Handle received data
              // This lambda will be called when data is received
          }
      ))),
      packetHandler(), // Initialize packetHandler if necessary
      radio(radioInterface),
      commandManager(std::make_shared<AVC::CommandManager>(avcProtocol)),
      telemetryBuffer(100), // Example capacity
      diagnostics(), // Default constructor
      userCallbacks(nullptr),
      logger(Logger::getInstance()),  // Singleton instance
      sendThread(), // Default initialization
      receiveThread(), // Default initialization
      isRunning(false),
      callbackMutex(),
      telemetryMutex(),
      sendCondition(),
      sendMutex() {  // Declare sendMutex as a member variable
    // Initialize logger with default settings
    logger.setLogLevel(LogLevel::INFO);
    logger.enableTimestamp(true);
    logger.enableThreadId(true);
    logger.addOutput(std::cout);
}

RocketLink::~RocketLink() {
    // Stop running threads
    isRunning.store(false);
    if (sendThread.joinable()) {
        sendCondition.notify_all();
        sendThread.join();
    }
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    logger.log(LogLevel::INFO, "RocketLink instance destroyed.");
}

bool RocketLink::initialize() {
    try {
        logger.log(LogLevel::INFO, "Initializing RocketLink system...");

        // Initialize the radio module
        logger.log(LogLevel::INFO, "Initializing radio module...");
        radio->initialize();
        logger.log(LogLevel::INFO, "Radio module initialized successfully.");

        // Initialize AVC Protocol
        logger.log(LogLevel::INFO, "Initializing AVC Protocol...");
        avcProtocol->start();
        logger.log(LogLevel::INFO, "AVC Protocol initialized successfully.");

        // Initialize Command Manager
        logger.log(LogLevel::INFO, "Initializing Command Manager...");
        commandManager->start();
        logger.log(LogLevel::INFO, "Command Manager initialized successfully.");

        // Start communication threads
        isRunning.store(true);
        sendThread = std::thread(&RocketLink::sendLoop, this);
        receiveThread = std::thread(&RocketLink::receiveLoop, this);
        logger.log(LogLevel::INFO, "Communication threads started.");

        logger.log(LogLevel::INFO, "RocketLink system initialized successfully.");
        return true;
    }
    catch (const std::exception& ex) {
        logger.log(LogLevel::ERROR, std::string("Initialization failed: ") + ex.what());
        return false;
    }
}

bool RocketLink::sendCommand(const AVC::Command& cmd) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (!isRunning.load()) {
        logger.log(LogLevel::WARNING, "Attempted to send command while system is not running.");
        return false;
    }

    bool queued = commandManager->addCommand(cmd, cmd.getPriority());
    if (queued) {
        logger.log(LogLevel::INFO, "Command queued successfully.");
        sendCondition.notify_one();
        return true;
    }
    else {
        logger.log(LogLevel::WARNING, "Failed to queue command.");
        return false;
    }
}

void RocketLink::registerCallbacks(API::Callbacks* callbacks) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    userCallbacks = callbacks;
    logger.log(LogLevel::INFO, "User callbacks registered.");
}

AVC::Telemetry RocketLink::getTelemetry() {
    std::lock_guard<std::mutex> lock(telemetryMutex);
    auto telemetryOpt = telemetryBuffer.getLatestTelemetry();
    if (telemetryOpt.has_value()) {
        return telemetryOpt.value();
    }
    else {
        logger.log(LogLevel::WARNING, "No telemetry data available.");
        return AVC::Telemetry();
    }
}

void RocketLink::sendLoop() {
    logger.log(LogLevel::INFO, "Send thread started.");
    while (isRunning.load()) {
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCondition.wait(lock, [this]() { return !commandManager->isQueueEmpty() || !isRunning.load(); });

        if (!isRunning.load()) {
            break;
        }

        // Retrieve the next command from CommandManager
        auto commandOpt = commandManager->getNextCommand();
        if (commandOpt.has_value()) {
            try {
                // Encode the command using AVCProtocol
                auto encodedCommand = avcProtocol->encodeCommand(commandOpt.value());

                // Create a SCALPEL::Packet with the encoded command
                SCALPEL::Packet packet(encodedCommand);

                // Transmit the packet via RadioInterface
                radio->sendPacket(packet.assemble());

                logger.log(LogLevel::DEBUG, "Command transmitted successfully.");
            }
            catch (const std::exception& ex) {
                logger.log(LogLevel::ERROR, std::string("Error in sendLoop: ") + ex.what());
                handleEvent("SendLoopError");
            }
        }
    }
    logger.log(LogLevel::INFO, "Send thread terminated.");
}

void RocketLink::receiveLoop() {
    logger.log(LogLevel::INFO, "Receive thread started.");
    while (isRunning.load()) {
        try {
            // Listen for incoming packets via RadioInterface
            SCALPEL::Packet receivedPacket;
            bool received = radio->receivePacket(receivedPacket);

            if (received) {
                // Decode telemetry data using AVCProtocol
                auto telemetry = avcProtocol->decodeTelemetry(receivedPacket);

                // Store telemetry data in the TelemetryBuffer
                telemetryBuffer.addTelemetry(telemetry);

                logger.log(LogLevel::DEBUG, "Telemetry data received and stored.");

                // Trigger relevant callbacks
                {
                    std::lock_guard<std::mutex> lock(callbackMutex);
                    if (userCallbacks) {
                        userCallbacks->invokeTelemetryCallback(telemetry);
                    }
                }

                // Update diagnostics information
                diagnostics.packetReceived();
            }
        }
        catch (const std::exception& ex) {
            logger.log(LogLevel::ERROR, std::string("Error in receiveLoop: ") + ex.what());
            handleEvent("ReceiveLoopError");
        }
    }
    logger.log(LogLevel::INFO, "Receive thread terminated.");
}

void RocketLink::handleEvent(const std::string& event) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (userCallbacks) {
        // Example: Invoke a generic event callback
        // You can extend this to handle different types of events
        logger.log(LogLevel::INFO, "Handling event: " + event);
        // userCallbacks->invokeEventCallback(event); // Assuming such a method exists
    }
}

} // namespace Core
} // namespace RocketLink