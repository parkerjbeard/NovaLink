#include "RFD900.hpp"
#include "SCALPEL/Packet.hpp"
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace RocketLink {
namespace Radio {

RFD900::RFD900(const std::string& port, unsigned int baudRate)
    : serialPort(ioService), running(false), netID(25), 
      frequencyMin(915000), frequencyMax(928000), numChannels(20), dutyCycle(100) {
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

RFD900::~RFD900() {
    running = false;
    ioService.stop();
    if (ioThread.joinable()) {
        ioThread.join();
    }
    if (serialPort.is_open()) {
        serialPort.close();
    }
}

void RFD900::initialize() {
    std::string response;
    try {
        // Enter command mode
        sendCommand("+++", response);
        if (response != "OK") {
            throw RadioException("Failed to enter command mode");
        }

        // Set frequency hopping
        std::ostringstream cmd;
        cmd << "ATS6=" << frequencyMin << "\r\n";
        sendCommand(cmd.str(), response);
        cmd.str("");
        cmd << "ATS7=" << frequencyMax << "\r\n";
        sendCommand(cmd.str(), response);
        cmd.str("");
        cmd << "ATS8=" << static_cast<int>(numChannels) << "\r\n";
        sendCommand(cmd.str(), response);

        // Set network ID
        cmd.str("");
        cmd << "ATS3=" << static_cast<int>(netID) << "\r\n";
        sendCommand(cmd.str(), response);

        // Set duty cycle
        cmd.str("");
        cmd << "ATS16=" << dutyCycle << "\r\n";
        sendCommand(cmd.str(), response);

        // Enable MAVLink framing
        sendCommand("ATS4=1\r\n", response);

        // Save settings
        sendCommand("AT&W\r\n", response);

        // Exit command mode
        sendCommand("ATO\r\n", response);

    } catch (const RadioException& e) {
        throw RadioException("Initialization failed: " + std::string(e.what()));
    }

    // Start asynchronous read loop
    running = true;
    ioThread = std::thread([this]() { this->readLoop(); });
}

void RFD900::configure(const RadioConfig& config) {
    std::lock_guard<std::mutex> lock(statusMutex);
    currentConfig = config;

    // Apply new configuration
    try {
        std::string response;
        sendCommand("+++", response); // Enter command mode

        // Set frequency
        std::ostringstream cmd;
        cmd << "ATS1=" << config.frequencyHz / 1000 << "\r\n"; // Convert Hz to kHz
        sendCommand(cmd.str(), response);

        // Set power level
        cmd.str("");
        cmd << "ATS5=" << static_cast<int>(config.powerLevel) << "\r\n";
        sendCommand(cmd.str(), response);

        // Set other parameters as needed

        sendCommand("AT&W\r\n", response); // Save settings
        sendCommand("ATO\r\n", response);  // Exit command mode

    } catch (const RadioException& e) {
        throw RadioException("Configuration failed: " + std::string(e.what()));
    }

    currentStatus.isInitialized = true;
}

void RFD900::getStatus(RadioStatus& status) {
    std::lock_guard<std::mutex> lock(statusMutex);
    status = currentStatus;
}

void RFD900::sendPacket(const SCALPEL::Packet& packet) {
    std::vector<uint8_t> data = packet.assemble();
    
    // Add MAVLink framing
    std::vector<uint8_t> framedData;
    framedData.push_back(0xFE); // MAVLink v1 start byte
    framedData.push_back(data.size());
    framedData.push_back(0); // Sequence number (not used in this context)
    framedData.push_back(1); // System ID (arbitrary)
    framedData.push_back(1); // Component ID (arbitrary)
    framedData.push_back(0); // Message ID (0 for custom data)
    framedData.insert(framedData.end(), data.begin(), data.end());

    // Calculate CRC (simplified version, replace with actual MAVLink CRC if needed)
    uint16_t crc = 0xFFFF;
    for (const auto& byte : framedData) {
        crc ^= byte << 8;
        for (int i = 0; i < 8; ++i) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
        }
    }
    framedData.push_back(crc & 0xFF);
    framedData.push_back((crc >> 8) & 0xFF);

    try {
        boost::asio::write(serialPort, boost::asio::buffer(framedData));
        std::lock_guard<std::mutex> lock(statusMutex);
        currentStatus.packetsSent++;
    } catch (const boost::system::system_error& e) {
        std::lock_guard<std::mutex> lock(statusMutex);
        currentStatus.transmissionErrors++;
        throw RadioException("Failed to send packet: " + std::string(e.what()));
    }
}

bool RFD900::receivePacket(SCALPEL::Packet& packet) {
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

void RFD900::readLoop() {
    std::vector<uint8_t> buffer(1024);
    while (running) {
        try {
            size_t bytesRead = serialPort.read_some(boost::asio::buffer(buffer));
            if (bytesRead > 0) {
                std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesRead);
                processFrame(data);
            }
        } catch (const boost::system::system_error& e) {
            if (running) {
                std::lock_guard<std::mutex> lock(statusMutex);
                currentStatus.receptionErrors++;
            }
        }
    }
}

void RFD900::processFrame(const std::vector<uint8_t>& data) {
    // Simple MAVLink frame parsing (replace with full MAVLink parser if needed)
    if (data.size() < 8) return; // Minimum MAVLink frame size

    for (size_t i = 0; i < data.size() - 8; ++i) {
        if (data[i] == 0xFE) { // MAVLink v1 start byte
            size_t length = data[i + 1];
            if (i + length + 8 <= data.size()) {
                // Extract payload
                std::vector<uint8_t> payload(data.begin() + i + 6, data.begin() + i + 6 + length);

                try {
                    SCALPEL::Packet packet = SCALPEL::Packet::disassemble(payload);
                    
                    std::lock_guard<std::mutex> lock(queueMutex);
                    packetQueue.push(packet);
                    queueCondVar.notify_one();
                } catch (const std::exception& e) {
                    std::lock_guard<std::mutex> lock(statusMutex);
                    currentStatus.receptionErrors++;
                }

                i += length + 7; // Skip to end of frame
            }
        }
    }
}

void RFD900::sendCommand(const std::string& command, std::string& response) {
    boost::asio::write(serialPort, boost::asio::buffer(command));
    
    boost::asio::streambuf responseBuffer;
    boost::asio::read_until(serialPort, responseBuffer, "\r\n");
    
    std::istream responseStream(&responseBuffer);
    std::getline(responseStream, response);
}

void RFD900::parseTelemetry(const std::string& data) {
    std::istringstream iss(data);
    std::string key, value;

    while (std::getline(iss, key, '=') && std::getline(iss, value, ',')) {
        if (key == "RSSI") {
            currentStatus.signalStrength = std::stoi(value);
        }
        // Parse other telemetry data as needed
    }
}

} // namespace Radio
} // namespace RocketLink