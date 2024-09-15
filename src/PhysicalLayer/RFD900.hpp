#ifndef ROCKETLINK_RADIO_RFD900_HPP
#define ROCKETLINK_RADIO_RFD900_HPP

#include "RadioInterface.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace RocketLink {
namespace Radio {

/**
 * @brief Class implementing the driver for the RFD900 radio module.
 */
class RFD900 : public RadioInterface {
public:
    /**
     * @brief Constructs the RFD900 driver with specified serial port settings.
     * @param port The serial port name (e.g., "/dev/ttyUSB0" or "COM3").
     * @param baudRate The baud rate for serial communication (default: 57600).
     */
    RFD900(const std::string& port, unsigned int baudRate = 57600);

    /**
     * @brief Destructor to clean up resources.
     */
    virtual ~RFD900();

    /**
     * @brief Prepares the RFD900 module for operation.
     * @throws RadioException if initialization fails.
     */
    void initialize() override;

    /**
     * @brief Sends a SCALPEL packet over the RFD900 radio.
     * @param packet The SCALPEL packet to send.
     * @throws RadioException if sending fails.
     */
    void sendPacket(const SCALPEL::Packet& packet) override;

    /**
     * @brief Receives a SCALPEL packet from the RFD900 radio.
     * @param packet The SCALPEL packet received.
     * @return true if a packet was successfully received, false otherwise.
     * @throws RadioException if reception fails.
     */
    bool receivePacket(SCALPEL::Packet& packet) override;

    /**
     * @brief Sets radio parameters based on the provided configuration.
     * @param config The configuration parameters.
     * @throws RadioException if configuration fails.
     */
    void configure(const RadioConfig& config) override;

    /**
     * @brief Retrieves radio status metrics.
     * @param status The structure to populate with status metrics.
     * @throws RadioException if status retrieval fails.
     */
    void getStatus(RadioStatus& status) override;

private:
    /**
     * @brief Reads data asynchronously from the serial port.
     */
    void readLoop();

    /**
     * @brief Processes incoming data frames.
     * @param data The raw data received.
     */
    void processFrame(const std::vector<uint8_t>& data);

    /**
     * @brief Sends a command to the RFD900 module.
     * @param command The command string to send.
     * @param response The response received from the module.
     * @throws RadioException if sending fails or no response is received.
     */
    void sendCommand(const std::string& command, std::string& response);

    /**
     * @brief Parses telemetry data from the RFD900 module.
     * @param data The raw telemetry data.
     */
    void parseTelemetry(const std::string& data);

    // Boost.Asio components
    boost::asio::io_service ioService;
    boost::asio::serial_port serialPort;
    std::thread ioThread;

    // Buffer for incoming data
    std::vector<uint8_t> readBuffer;
    std::mutex readMutex;

    // Configuration parameters
    RadioConfig currentConfig;

    // Status metrics
    RadioStatus currentStatus;
    std::mutex statusMutex;

    // Synchronization for received packets
    std::queue<SCALPEL::Packet> packetQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    std::atomic<bool> running;

    // RFD900-specific parameters
    uint8_t netID;
    uint32_t frequencyMin;
    uint32_t frequencyMax;
    uint8_t numChannels;
    uint16_t dutyCycle;
};

} // namespace Radio
} // namespace RocketLink

#endif // ROCKETLINK_RADIO_RFD900_HPP
