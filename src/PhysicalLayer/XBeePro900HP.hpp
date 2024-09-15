#ifndef ROCKETLINK_RADIO_XBEEPRO900HP_HPP
#define ROCKETLINK_RADIO_XBEEPRO900HP_HPP

#include "RadioInterface.hpp"
#include "SCALPEL/Packet.hpp"
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
 * @brief Class implementing the driver for the XBee Pro 900 HP radio module.
 */
class XBeePro900HP : public RadioInterface {
public:
    /**
     * @brief Constructs the XBeePro900HP driver with specified serial port settings.
     * @param port The serial port name (e.g., "/dev/ttyUSB0" or "COM3").
     * @param baudRate The baud rate for serial communication (default: 57600).
     */
    XBeePro900HP(const std::string& port, unsigned int baudRate = 57600);

    /**
     * @brief Destructor to clean up resources.
     */
    virtual ~XBeePro900HP();

    /**
     * @brief Prepares the XBee module for operation.
     * @throws RadioException if initialization fails.
     */
    void initialize() override;

    /**
     * @brief Sends a SCALPEL packet over the XBee radio.
     * @param packet The SCALPEL packet to send.
     * @throws RadioException if sending fails.
     */
    void sendPacket(const SCALPEL::Packet& packet) override;

    /**
     * @brief Receives a SCALPEL packet from the XBee radio.
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
     * @brief Processes incoming API frames.
     * @param data The raw data of the API frame.
     */
    void processFrame(const std::vector<uint8_t>& data);

    /**
     * @brief Sends an API frame to the XBee module.
     * @param frame The API frame to send.
     * @throws RadioException if sending fails.
     */
    void sendFrame(const std::vector<uint8_t>& frame);

    /**
     * @brief Constructs an API frame for transmission.
     * @param packet The SCALPEL packet to encapsulate.
     * @return The constructed API frame.
     */
    std::vector<uint8_t> constructTransmitRequest(const SCALPEL::Packet& packet);

    /**
     * @brief Parses an API frame into a SCALPEL packet.
     * @param frame The received API frame.
     * @param packet The parsed SCALPEL packet.
     * @return true if parsing is successful, false otherwise.
     */
    bool parseRxPacket(const std::vector<uint8_t>& frame, SCALPEL::Packet& packet);

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
};

} // namespace Radio
} // namespace RocketLink

#endif // ROCKETLINK_RADIO_XBEEPRO900HP_HPP