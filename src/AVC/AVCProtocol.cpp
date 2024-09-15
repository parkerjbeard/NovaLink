#include "AVCProtocol.hpp"

namespace RocketLink {
namespace AVC {

AVCProtocol::AVCProtocol(std::shared_ptr<SCALPEL::Communicator> comm)
    : communicator(comm), running(false) {
    registerPayloadDescriptors();
}

AVCProtocol::~AVCProtocol() {
    stop();
}

void AVCProtocol::start() {
    if (running) {
        return;
    }
    running = true;

    // Start the retransmission handler thread
    retransThread = std::thread(&AVCProtocol::retransmissionHandler, this);

    // Register the data received callback
    communicator->start();
}

void AVCProtocol::stop() {
    if (!running) {
        return;
    }
    running = false;
    cv.notify_all();

    if (retransThread.joinable()) {
        retransThread.join();
    }

    communicator->stop();
}

void AVCProtocol::sendCommand(const Command& command) {
    if (!command.isValid()) {
        throw std::invalid_argument("Attempting to send an invalid command");
    }
    std::vector<uint8_t> encoded = command.encode();

    // Calculate checksum
    uint8_t crc = SCALPEL::Checksum::calculateCRC8(encoded.data(), encoded.size());

    // Assemble packet
    SCALPEL::Packet packet(encoded);

    // Add checksum
    std::vector<uint8_t> packetData = packet.assemble();
    packetData.push_back(crc);

    // Encode with COBS
    SCALPEL::COBS::COBSResult cobsResult = cobsEncoder.encode(packetData);
    std::vector<uint8_t> finalPacket = cobsResult.encodedPayload;

    sendRawPacket(finalPacket);

    // Store the command for acknowledgment tracking
    {
        std::lock_guard<std::mutex> lock(pendingMutex);
        PendingCommand pending;
        pending.command = command;
        pending.timestamp = std::chrono::steady_clock::now();
        // retryCount is already initialized to 0 in the default constructor
        pendingCommands[static_cast<uint8_t>(command.getCommandNumber())] = pending;
    }
}

void AVCProtocol::sendTelemetry(const Telemetry& telemetry) {
    std::vector<uint8_t> encoded = telemetry.encode();

    // Calculate checksum
    uint8_t crc = SCALPEL::Checksum::calculateCRC8(encoded.data(), encoded.size());

    // Assemble packet
    SCALPEL::Packet packet(encoded);

    // Add checksum
    std::vector<uint8_t> packetData = packet.assemble();
    packetData.push_back(crc);

    // Encode with COBS
    SCALPEL::COBS::COBSResult cobsResult = cobsEncoder.encode(packetData);
    std::vector<uint8_t> finalPacket = cobsResult.encodedPayload;

    sendRawPacket(finalPacket);
}

void AVCProtocol::sendRawPacket(const std::vector<uint8_t>& data) {
    // SCALPEL::COBS handles framing, but Communicator manages raw data
    communicator->send(data);
}

void AVCProtocol::onDataReceived(const std::vector<uint8_t>& data) {
    // Decode with COBS
    try {
        std::vector<uint8_t> decodedData = cobsDecoder.decode(data, 0); // Index managed internally

        // Verify checksum
        if (decodedData.size() < 1) {
            std::cerr << "Received data too short after decoding." << std::endl;
            return;
        }
        uint8_t receivedCrc = decodedData.back();
        decodedData.pop_back();

        // Calculate CRC
        uint8_t calculatedCrc = SCALPEL::Checksum::calculateCRC8(decodedData.data(), decodedData.size());

        // Explicitly compare the calculated CRC with the received CRC
        if (calculatedCrc != receivedCrc) {
            std::cerr << "Checksum validation failed. Received CRC: " << static_cast<int>(receivedCrc)
                      << ", Calculated CRC: " << static_cast<int>(calculatedCrc) << std::endl;
            return;
        }

        // Disassemble packet
        SCALPEL::Packet packet = SCALPEL::Packet::disassemble(decodedData);

        handleIncomingPacket(packet.getPayload());
    } catch (const std::exception& e) {
        std::cerr << "Error decoding received packet: " << e.what() << std::endl;
    }
}

void AVCProtocol::handleIncomingPacket(const std::vector<uint8_t>& data) {
    if (data.size() < 2) { // Minimum size: header + descriptor
        std::cerr << "Received packet too short." << std::endl;
        return;
    }

    uint8_t descriptor = data[1];
    std::lock_guard<std::mutex> lock(protocolMutex);

    auto it = descriptorHandlers.find(descriptor);
    if (it != descriptorHandlers.end()) {
        it->second(data);
    } else {
        std::cerr << "Unknown payload descriptor: " << static_cast<int>(descriptor) << std::endl;
    }
}

void AVCProtocol::handleAcknowledgment(uint8_t ackCommandNumber) {
    std::lock_guard<std::mutex> lock(pendingMutex);
    auto it = pendingCommands.find(ackCommandNumber);
    if (it != pendingCommands.end()) {
        // Command acknowledged, remove from pending
        pendingCommands.erase(it);
        std::cout << "Command " << static_cast<int>(ackCommandNumber) << " acknowledged." << std::endl;
    } else {
        std::cerr << "Received acknowledgment for unknown command: " << static_cast<int>(ackCommandNumber) << std::endl;
    }
}

void AVCProtocol::retransmissionHandler() {
    while (running) {
        auto now = std::chrono::steady_clock::now();
        std::vector<PendingCommand> toResend;

        {
            std::lock_guard<std::mutex> lock(pendingMutex);
            for (auto it = pendingCommands.begin(); it != pendingCommands.end(); ) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.timestamp);
                if (elapsed >= retryInterval) {
                    if (it->second.retryCount < maxRetries) {
                        toResend.push_back(it->second);
                        it->second.timestamp = now;
                        it->second.retryCount++;
                        ++it;
                    } else {
                        std::cerr << "Command " << static_cast<int>(it->first) << " timed out after "
                                  << maxRetries << " retries." << std::endl;
                        // Optionally notify CommandManager about the failure
                        it = pendingCommands.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        }

        for (const auto& pending : toResend) {
            std::cout << "Resending Command " << static_cast<int>(pending.command.getCommandNumber())
                      << " (Retry " << pending.retryCount << ")" << std::endl;
            sendRawPacket(pending.command.encode());
        }

        // Wait for the next interval or stop signal
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait_for(lock, retryInterval, [this]() { return !running.load(); });
    }
}

void AVCProtocol::registerPayloadDescriptors() {
    // Register Command Descriptor
    descriptorHandlers[static_cast<uint8_t>(PayloadDescriptor::COMMAND)] = 
        [](const std::vector<uint8_t>& data) {
            try {
                Command cmd = Command::decode(data);
                std::cout << "Received Command from " << static_cast<int>(cmd.getSenderID()) 
                          << " to " << static_cast<int>(cmd.getReceiverID()) << std::endl;
                // Process the command as needed
                // For example, execute the command actions
            } catch (const std::exception& e) {
                std::cerr << "Error decoding Command: " << e.what() << std::endl;
            }
        };

    // Register Telemetry A Descriptor
    descriptorHandlers[static_cast<uint8_t>(TelemetryDescriptor::TELEMETRY_A)] = 
        [](const std::vector<uint8_t>& data) {
            try {
                Telemetry telemetry = Telemetry::decode(data);
                std::cout << "Received Telemetry A from " << static_cast<int>(telemetry.getSenderID()) 
                          << " to " << static_cast<int>(telemetry.getReceiverID()) << std::endl;
                // Process the telemetry data as needed
                // For example, store it in TelemetryBuffer
            } catch (const std::exception& e) {
                std::cerr << "Error decoding Telemetry A: " << e.what() << std::endl;
            }
        };

    // Register Telemetry B Descriptor
    descriptorHandlers[static_cast<uint8_t>(TelemetryDescriptor::TELEMETRY_B)] = 
        [](const std::vector<uint8_t>& data) {
            try {
                Telemetry telemetry = Telemetry::decode(data);
                std::cout << "Received Telemetry B from " << static_cast<int>(telemetry.getSenderID()) 
                          << " to " << static_cast<int>(telemetry.getReceiverID()) << std::endl;
                // Process the telemetry data as needed
            } catch (const std::exception& e) {
                std::cerr << "Error decoding Telemetry B: " << e.what() << std::endl;
            }
        };

    // Register Acknowledgment Descriptor
    descriptorHandlers[static_cast<uint8_t>(PayloadDescriptor::ACKNOWLEDGMENT)] = 
        [this](const std::vector<uint8_t>& data) {
            if (data.size() < 3) { // Header (1) + Descriptor (1) + Acknowledged Command Number (1)
                std::cerr << "Invalid Acknowledgment packet size." << std::endl;
                return;
            }
            uint8_t ackCmdNum = data[2];
            this->handleAcknowledgment(ackCmdNum);
        };

    // Add more descriptors and handlers as needed
}

std::vector<uint8_t> AVCProtocol::encodeCommand(const Command& /* command */) {
    // Implement the encoding logic here
    // This is a placeholder implementation
    std::vector<uint8_t> encodedData;
    // ... encoding logic ...
    return encodedData;
}

Telemetry AVCProtocol::decodeTelemetry(const SCALPEL::Packet& packet) {
    // Get the payload from the packet
    std::vector<uint8_t> data = packet.getPayloadVector();

    // Implement the decoding logic here
    // This is a placeholder implementation
    Telemetry decodedTelemetry;
    // ... decoding logic ...
    return decodedTelemetry;
}

} // namespace AVC
} // namespace RocketLink