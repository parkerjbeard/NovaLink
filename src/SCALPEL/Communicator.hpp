#ifndef COMMUNICATOR_HPP
#define COMMUNICATOR_HPP

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace SCALPEL {

/**
 * @class Communicator
 * @brief Handles low-level I/O operations with the physical communication interface.
 */
class Communicator {
public:
    /**
     * @brief Type alias for the received data callback function.
     */
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>&)>;

    /**
     * @brief Constructs the Communicator.
     * @param receiveCallback Callback function to handle received data.
     */
    Communicator(ReceiveCallback receiveCallback);

    /**
     * @brief Destructor to clean up resources.
     */
    ~Communicator();

    /**
     * @brief Starts the communication interface.
     */
    void start();

    /**
     * @brief Stops the communication interface.
     */
    void stop();

    /**
     * @brief Sends raw data through the communication interface.
     * @param data The data to send.
     */
    void send(const std::vector<uint8_t>& data);

private:
    /**
     * @brief Thread function for sending data.
     */
    void sendThreadFunc();

    /**
     * @brief Thread function for receiving data.
     */
    void receiveThreadFunc();

    // Callback to handle received data
    ReceiveCallback onReceive;

    // Threads for sending and receiving
    std::thread sendThread;
    std::thread receiveThread;

    // Queues and synchronization primitives for sending data
    std::queue<std::vector<uint8_t>> sendQueue;
    std::mutex sendMutex;
    std::condition_variable sendCV;

    // Atomic flag to control the running state
    std::atomic<bool> running;
};

} // namespace SCALPEL

#endif // COMMUNICATOR_HPP