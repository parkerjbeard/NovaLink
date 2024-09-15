#include "TelemetryBuffer.hpp"

namespace RocketLink {
namespace AVC {

TelemetryBuffer::TelemetryBuffer(size_t capacity)
    : buffer(capacity), head(0), count(0), maxCapacity(capacity) {}

void TelemetryBuffer::addTelemetry(const Telemetry& telemetry) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer[head] = telemetry;
    head = (head + 1) % maxCapacity;
    if (count < maxCapacity) {
        ++count;
    }
}

std::optional<Telemetry> TelemetryBuffer::getLatestTelemetry() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count == 0) {
        return std::nullopt;
    }
    size_t latestIndex = (head + maxCapacity - 1) % maxCapacity;
    return buffer[latestIndex];
}

std::optional<Telemetry> TelemetryBuffer::getTelemetryAt(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index >= count) {
        return std::nullopt;
    }
    size_t actualIndex = (head + maxCapacity - count + index) % maxCapacity;
    return buffer[actualIndex];
}

std::vector<Telemetry> TelemetryBuffer::getAllTelemetry() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Telemetry> telemetryData;
    telemetryData.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        size_t index = (head + maxCapacity - count + i) % maxCapacity;
        telemetryData.push_back(buffer[index]);
    }
    return telemetryData;
}

size_t TelemetryBuffer::size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count;
}

size_t TelemetryBuffer::capacity() const {
    return maxCapacity;
}

} // namespace AVC
} // namespace RocketLink
