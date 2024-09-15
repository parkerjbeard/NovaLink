#include "CommandManager.hpp"

namespace RocketLink {
namespace AVC {

CommandManager::CommandManager(std::shared_ptr<AVCProtocol> avcProtocol)
    : running(false), avcProtocol_(avcProtocol) {}

CommandManager::~CommandManager() {
    stop();
}

void CommandManager::start() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running) {
            return;
        }
        running = true;
    }
    workerThread = std::thread(&CommandManager::workerThreadFunc, this);
}

void CommandManager::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running) {
            return;
        }
        running = false;
    }
    cv.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

bool CommandManager::addCommand(const Command& command, int priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pendingCmd = std::make_shared<PendingCommand>(command, priority);
    commandQueue.push(pendingCmd);
    
    // Convert CommandNumber to uint8_t
    uint8_t cmdNumber = static_cast<uint8_t>(command.getCommandNumber());
    pendingCommandsMap[cmdNumber] = pendingCmd;
    
    cv.notify_one();
    return true;
}

void CommandManager::handleAcknowledgment(uint8_t commandNumber) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pendingCommandsMap.find(commandNumber);
    if (it != pendingCommandsMap.end()) {
        // Remove from the map
        pendingCommandsMap.erase(it);
        // Note: It remains in the priority queue, but will be skipped in the worker thread
    }
}

void CommandManager::workerThreadFunc() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running) {
        if (commandQueue.empty()) {
            // Wait until a new command is added or stop is called
            cv.wait(lock, [this]() { return !running || !commandQueue.empty(); });
        } else {
            auto now = std::chrono::steady_clock::now();
            auto pendingCmd = commandQueue.top();

            // Calculate the next timeout time
            auto nextTimeout = pendingCmd->lastSentTime + timeoutInterval;

            if (now >= nextTimeout) {
                // Time to retransmit
                if (pendingCmd->retryCount >= maxRetries) {
                    // Exceeded max retries, remove from queue and map
                    uint8_t cmdNumber = static_cast<uint8_t>(pendingCmd->command.getCommandNumber());
                    pendingCommandsMap.erase(cmdNumber);
                    commandQueue.pop();
                    // Optionally, log the failure
                    // For production code, consider a callback or logging mechanism
                } else {
                    // Retransmit the command
                    avcProtocol_->sendCommand(pendingCmd->command);
                    // Update the last sent time and retry count
                    pendingCmd->lastSentTime = now;
                    pendingCmd->retryCount += 1;
                    // Reinsert into the queue
                    commandQueue.pop();
                    commandQueue.push(pendingCmd);
                }
            } else {
                // Wait until the next timeout or a new command is added
                cv.wait_until(lock, nextTimeout, [this]() { return !running; });
            }
        }
    }
}

bool CommandManager::isQueueEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return commandQueue.empty();
}

std::optional<Command> CommandManager::getNextCommand() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (commandQueue.empty()) {
        return std::nullopt;
    }
    auto pendingCmd = commandQueue.top();
    commandQueue.pop();
    uint8_t cmdNumber = static_cast<uint8_t>(pendingCmd->command.getCommandNumber());
    pendingCommandsMap.erase(cmdNumber);
    return pendingCmd->command;
}

} // namespace AVC
} // namespace RocketLink
