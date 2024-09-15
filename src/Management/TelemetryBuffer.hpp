#ifndef ROCKETLINK_AVC_TELEMETRYBUFFER_HPP
#define ROCKETLINK_AVC_TELEMETRYBUFFER_HPP

#include "AVC/Telemetry.hpp"
#include <vector>
#include <mutex>
#include <optional>

namespace RocketLink {
namespace AVC {

/**
 * @brief Class implementing a thread-safe circular buffer for Telemetry data.
 */
class TelemetryBuffer {
public:
    /**
     * @brief Constructs a TelemetryBuffer with the specified capacity.
     * @param capacity The maximum number of Telemetry objects the buffer can hold.
     */
    explicit TelemetryBuffer(size_t capacity);

    /**
     * @brief Adds a Telemetry object to the buffer.
     *        Overwrites the oldest data if the buffer is full.
     * @param telemetry The Telemetry object to add.
     */
    void addTelemetry(const Telemetry& telemetry);

    /**
     * @brief Retrieves the most recent Telemetry object.
     * @return An optional containing the latest Telemetry if available, or std::nullopt.
     */
    std::optional<Telemetry> getLatestTelemetry();

    /**
     * @brief Retrieves a Telemetry object at the specified index.
     *        Index 0 corresponds to the oldest data, and index size()-1 to the newest.
     * @param index The index of the Telemetry object to retrieve.
     * @return An optional containing the Telemetry object if index is valid, or std::nullopt.
     */
    std::optional<Telemetry> getTelemetryAt(size_t index);

    /**
     * @brief Retrieves all Telemetry data in the buffer in order from oldest to newest.
     * @return A vector containing all Telemetry objects.
     */
    std::vector<Telemetry> getAllTelemetry();

    /**
     * @brief Retrieves the current number of Telemetry objects in the buffer.
     * @return The number of Telemetry objects stored.
     */
    size_t size();

    /**
     * @brief Retrieves the maximum capacity of the buffer.
     * @return The maximum number of Telemetry objects the buffer can hold.
     */
    size_t capacity() const;

private:
    std::vector<Telemetry> buffer;      // Fixed-size buffer
    size_t head;                        // Points to the next write position
    size_t count;                       // Current number of elements in the buffer
    size_t maxCapacity;                 // Maximum capacity of the buffer
    std::mutex mutex_;                  // Mutex for thread-safe access
};

} // namespace AVC
} // namespace RocketLink

#endif // ROCKETLINK_AVC_TELEMETRYBUFFER_HPP
