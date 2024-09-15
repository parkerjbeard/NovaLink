#ifndef ROCKETLINK_AVC_COMMANDMANAGER_HPP
#define ROCKETLINK_AVC_COMMANDMANAGER_HPP

#include "AVC/Command.hpp"
#include "AVC/AVCProtocol.hpp"
#include <queue>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <optional>

namespace RocketLink {
namespace AVC {

/**
 * @brief Structure representing a pending command with its metadata.
 */
struct PendingCommand {
    Command command;
    int priority;
    std::chrono::steady_clock::time_point lastSentTime;
    int retryCount;

    PendingCommand(const Command& cmd, int prio)
        : command(cmd), priority(prio), lastSentTime(std::chrono::steady_clock::now()), retryCount(0) {}
};

/**
 * @brief Comparator for the priority queue to sort PendingCommands by priority and lastSentTime.
 */
struct ComparePendingCommand {
    bool operator()(const std::shared_ptr<PendingCommand>& a, const std::shared_ptr<PendingCommand>& b) const {
        if (a->priority == b->priority) {
            return a->lastSentTime > b->lastSentTime;
        }
        // Higher priority value means higher urgency
        return a->priority < b->priority;
    }
};

/**
 * @brief Class managing the priority-based command queue, handling timeouts and retransmissions.
 */
class CommandManager {
public:
    /**
     * @brief Constructs the CommandManager with a shared AVCProtocol instance.
     * @param avcProtocol Shared pointer to the AVCProtocol.
     */
    CommandManager(std::shared_ptr<AVCProtocol> avcProtocol);

    /**
     * @brief Destructor to clean up resources.
     */
    ~CommandManager();

    /**
     * @brief Adds a command to the queue with the specified priority.
     * @param command The Command object to add.
     * @param priority The priority of the command (higher value = higher priority).
     */
    bool addCommand(const Command& command, int priority);

    /**
     * @brief Handles an acknowledgment for a given command number.
     * Removes the command from the pending queue if it exists.
     * @param commandNumber The CommandNumber of the acknowledged command.
     */
    void handleAcknowledgment(uint8_t commandNumber);

    /**
     * @brief Starts the CommandManager's internal processing threads.
     */
    void start();

    /**
     * @brief Stops the CommandManager's internal processing threads.
     */
    void stop();

    /**
     * @brief Checks if the command queue is empty.
     * @return true if the queue is empty, false otherwise.
     */
    bool isQueueEmpty() const;

    /**
     * @brief Retrieves the next command from the queue.
     * @return An optional Command object. Contains a Command if available, otherwise std::nullopt.
     */
    std::optional<Command> getNextCommand();

private:
    /**
     * @brief The worker thread function that manages timeouts and retransmissions.
     */
    void workerThreadFunc();

    // Priority queue to manage pending commands
    std::priority_queue<
        std::shared_ptr<PendingCommand>,
        std::vector<std::shared_ptr<PendingCommand>>,
        ComparePendingCommand
    > commandQueue;

    // Map to track pending commands by CommandNumber for quick lookup
    std::unordered_map<uint8_t, std::shared_ptr<PendingCommand>> pendingCommandsMap;

    // Mutex to protect shared resources
    mutable std::mutex mutex_;

    // Condition variable to notify the worker thread
    std::condition_variable cv;

    // Worker thread for handling timeouts and retransmissions
    std::thread workerThread;

    // Flag to control the running state of the worker thread
    bool running;

    // Shared AVCProtocol instance for sending commands
    std::shared_ptr<AVCProtocol> avcProtocol_;

    // Configuration parameters
    const int maxRetries = 5;
    const std::chrono::milliseconds timeoutInterval = std::chrono::milliseconds(500);
};

} // namespace AVC
} // namespace RocketLink

#endif // ROCKETLINK_AVC_COMMANDMANAGER_HPP
