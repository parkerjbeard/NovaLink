#include "Communicator.hpp"
#include <iostream>
// Include your specific hardware communication library headers here
// For example, for serial communication, you might include <serial/serial.h>

namespace SCALPEL {

Communicator::Communicator(ReceiveCallback receiveCallback)
    : onReceive(receiveCallback), running(false) {}

Communicator::~Communicator() {
    stop();
}

void Communicator::start() {
    running = true;
    sendThread = std::thread(&Communicator::sendThreadFunc, this);
    receiveThread = std::thread(&Communicator::receiveThreadFunc, this);
}

void Communicator::stop() {
    if (running) {
        running = false;
        sendCV.notify_all();

        if (sendThread.joinable()) {
            sendThread.join();
        }
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }
}

void Communicator::send(const std::vector<uint8_t>& data) {
    {
        std::lock_guard<std::mutex> lock(sendMutex);
        sendQueue.push(data);
    }
    sendCV.notify_one();
}

void Communicator::sendThreadFunc() {
    while (running) {
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCV.wait(lock, [this]() { return !sendQueue.empty() || !running; });

        while (!sendQueue.empty()) {
            std::vector<uint8_t> data = sendQueue.front();
            sendQueue.pop();
            lock.unlock();

            // Implement the actual send logic here.
            // For example, write to a serial port.
            // Example:
            // serialPort.write(data);

            // Placeholder for send operation
            std::cout << "Sending data:";
            for (auto byte : data) {
                std::cout << " " << static_cast<int>(byte);
            }
            std::cout << std::endl;

            lock.lock();
        }
    }
}

void Communicator::receiveThreadFunc() {
    while (running) {
        // Implement the actual receive logic here.
        // For example, read from a serial port.
        // Example:
        // std::vector<uint8_t> incomingData = serialPort.read();

        // Placeholder for receive operation
        // Simulate receiving data
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::vector<uint8_t> incomingData = { /* populate with received bytes */ };

        if (!incomingData.empty()) {
            onReceive(incomingData);
        }
    }
}

} // namespace SCALPEL