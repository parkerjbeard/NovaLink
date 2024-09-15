#include "XBeePro900HP.hpp"
#include "SCALPEL/Packet.hpp"
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <stdexcept>

namespace RocketLink {
namespace Radio {

XBeePro900HP::XBeePro900HP(const std::string& port, unsigned int baudRate)
    : serialPort(ioService), running(false) {
    try {
        serialPort.open(port);
        serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    } catch (const boost::system::system_error& e) {
        throw RadioException("Failed to open serial port: " + std::string(e.what()));
    }
}

XBeePro900HP::~XBeePro900HP() {
    running = false;
    ioService.stop();
    if (ioThread.joinable()) {
        ioThread.join();
    }
    if (serialPort.is_open()) {
        serialPort.close();
    }
}

void XBeePro900HP::initialize() {
    // Set API mode 2 (escaped)
    std::vector<uint8_t> apiModeFrame = {0x7E, 0x00, 0x05, 0x08, 0x02, 0x4D, 0x79, 0x00, 0xE6};
    try {
        sendFrame(apiModeFrame);
    } catch (const RadioException& e) {
        throw RadioException("Failed to set API mode: " + std::string(e.what()));
    }

    // Start asynchronous read loop
    running = true;
    ioThread = std::thread([this]() { this->readLoop(); });
}

void XBeePro900HP::configure(const RadioConfig& config) {
    std::lock_guard<std::mutex> lock(statusMutex);
    currentConfig = config;

    // Example: Configure frequency, baud rate, power level, channel, mode
    // The actual AT commands and parameters depend on the XBee module documentation

    // Set frequency
    // Placeholder: Replace with actual AT commands
    // Similarly set other parameters

    // Example for setting PAN ID (if applicable)
    // Placeholder: Implement actual AT frame construction and sending

    // After configuration, update the currentStatus as initialized
    currentStatus.isInitialized = true;
}

void XBeePro900HP::getStatus(RadioStatus& status) {
    std::lock_guard<std::mutex> lock(statusMutex);
    status = currentStatus;
}

void XBeePro900HP::sendPacket(const SCALPEL::Packet& packet) {
    std::vector<uint8_t> frame = constructTransmitRequest(packet);
    try {
        sendFrame(frame);
        std::lock_guard<std::mutex> lock(statusMutex);
        currentStatus.packetsSent++;
    } catch (const RadioException& e) {
        std::lock_guard<std::mutex> lock(statusMutex);
        currentStatus.transmissionErrors++;
        throw;
    }
}

bool XBeePro900HP::receivePacket(SCALPEL::Packet& packet) {
    std::unique_lock<std::mutex> lock(queueMutex);
    if (packetQueue.empty()) {
        // Wait for a packet to be available
        if (queueCondVar.wait_for(lock, std::chrono::milliseconds(100)) == std::cv_status::timeout) {
            return false;
        }
    }

    if (!packetQueue.empty()) {
        packet = packetQueue.front();
        packetQueue.pop();
        std::lock_guard<std::mutex> statusLock(statusMutex);
        currentStatus.packetsReceived++;
        return true;
    }

    return false;
}

void XBeePro900HP::readLoop() {
    while (running) {
        try {
            // Read until start delimiter 0x7E is found
            uint8_t startByte;
            boost::asio::read(serialPort, boost::asio::buffer(&startByte, 1));
            if (startByte != 0x7E) {
                continue; // Invalid start byte
            }

            // Read length (2 bytes)
            uint8_t lengthBytes[2];
            boost::asio::read(serialPort, boost::asio::buffer(lengthBytes, 2));
            uint16_t length = (lengthBytes[0] << 8) | lengthBytes[1];

            // Read the frame data and checksum
            std::vector<uint8_t> frameData(length + 1); // +1 for checksum
            boost::asio::read(serialPort, boost::asio::buffer(frameData.data(), frameData.size()));

            // Verify checksum
            uint8_t calculatedChecksum = 0;
            for (size_t i = 0; i < length; ++i) {
                calculatedChecksum += frameData[i];
            }
            calculatedChecksum = 0xFF - calculatedChecksum;
            if (calculatedChecksum != frameData[length]) {
                std::lock_guard<std::mutex> lock(statusMutex);
                currentStatus.receptionErrors++;
                continue; // Checksum mismatch
            }

            // Process the frame
            std::vector<uint8_t> completeFrame;
            completeFrame.push_back(startByte);
            completeFrame.push_back(lengthBytes[0]);
            completeFrame.push_back(lengthBytes[1]);
            completeFrame.insert(completeFrame.end(), frameData.begin(), frameData.end() - 1); // Exclude checksum
            processFrame(completeFrame);
        } catch (const boost::system::system_error& e) {
            if (running) {
                std::lock_guard<std::mutex> lock(statusMutex);
                currentStatus.receptionErrors++;
            }
        }
    }
}

void XBeePro900HP::processFrame(const std::vector<uint8_t>& frame) {
    if (frame.size() < 4) {
        return; // Frame too short
    }

    uint8_t frameType = frame[3];
    switch (frameType) {
        case 0x90: { // Receive Packet
            SCALPEL::Packet packet;
            if (parseRxPacket(frame, packet)) {
                std::lock_guard<std::mutex> lock(queueMutex);
                packetQueue.push(packet);
                queueCondVar.notify_one();
            } else {
                std::lock_guard<std::mutex> lock(statusMutex);
                currentStatus.receptionErrors++;
            }
            break;
        }
        case 0x8B: { // Modem Status
            // Handle modem status if needed
            break;
        }
        // Handle other frame types as needed
        default:
            break;
    }
}

void XBeePro900HP::sendFrame(const std::vector<uint8_t>& frame) {
    boost::system::error_code ec;
    boost::asio::write(serialPort, boost::asio::buffer(frame), ec);
    if (ec) {
        throw RadioException("Failed to send frame: " + ec.message());
    }
}

std::vector<uint8_t> XBeePro900HP::constructTransmitRequest(const SCALPEL::Packet& packet) {
    std::vector<uint8_t> frame;

    // Start delimiter
    frame.push_back(0x7E);

    // Placeholder for length (to be filled later)
    frame.push_back(0x00);
    frame.push_back(0x00);

    // Frame type: Transmit Request (0x10)
    frame.push_back(0x10);

    // Frame ID
    frame.push_back(0x01); // Arbitrary frame ID

    // 64-bit destination address (Assuming broadcast for simplicity)
    for (int i = 0; i < 8; ++i) {
        frame.push_back(0xFF);
    }

    // 16-bit destination address (0xFFFE for unknown)
    frame.push_back(0xFF);
    frame.push_back(0xFE);

    // Broadcast radius
    frame.push_back(0x00); // Default

    // Options
    frame.push_back(0x00); // Default

    // RF data (payload)
    std::vector<uint8_t> payload = packet.assemble();
    frame.insert(frame.end(), payload.begin(), payload.end());

    // Calculate length
    uint16_t length = frame.size() - 3; // Exclude start delimiter and length bytes
    frame[1] = (length >> 8) & 0xFF;
    frame[2] = length & 0xFF;

    // Calculate checksum
    uint8_t checksum = 0;
    for (size_t i = 3; i < frame.size(); ++i) {
        checksum += frame[i];
    }
    checksum = 0xFF - checksum;
    frame.push_back(checksum);

    return frame;
}

bool XBeePro900HP::parseRxPacket(const std::vector<uint8_t>& frame, SCALPEL::Packet& packet) {
    // Example parsing for 0x90 frame type
    // Frame structure:
    // 0x7E | Length (2) | Frame Type (0x90) | 64-bit addr (8) | 16-bit addr (2) | options (1) | RF data
    if (frame.size() < 15) {
        return false; // Frame too short
    }

    // Extract RF data
    size_t rfDataStart = 14; // 0-based index
    std::vector<uint8_t> rfData(frame.begin() + rfDataStart, frame.end());

    try {
        packet = SCALPEL::Packet::disassemble(rfData);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace Radio
} // namespace RocketLink