#include "Diagnostics.hpp"
#include <stdexcept>

namespace RocketLink {
namespace Diagnostics {

// Updated constructor without RadioInterface reference
Diagnostics::Diagnostics()
    : snr_(0.0),
      ber_(0.0),
      cqi_(0.0),
      packetsSent_(0),
      packetsReceived_(0),
      packetsLost_(0),
      totalLatency_(0.0),
      maxLatency_(0.0),
      minLatency_(std::numeric_limits<double>::max()),
      latencyCount_(0)
{}

double Diagnostics::getSignalToNoiseRatio() const {
    std::lock_guard<std::mutex> lock(linkMetricsMutex_);
    return snr_;
}

double Diagnostics::getBitErrorRate() const {
    std::lock_guard<std::mutex> lock(linkMetricsMutex_);
    return ber_;
}

double Diagnostics::getCarrierToInterferenceRatio() const {
    std::lock_guard<std::mutex> lock(linkMetricsMutex_);
    return cqi_;
}

uint32_t Diagnostics::getPacketsSent() const {
    return packetsSent_.load();
}

uint32_t Diagnostics::getPacketsReceived() const {
    return packetsReceived_.load();
}

uint32_t Diagnostics::getPacketsLost() const {
    return packetsLost_.load();
}

double Diagnostics::getPacketLossRate() const {
    uint32_t sent = packetsSent_.load();
    uint32_t lost = packetsLost_.load();
    if (sent == 0) {
        return 0.0;
    }
    return static_cast<double>(lost) / static_cast<double>(sent) * 100.0;
}

double Diagnostics::getAverageLatency() const {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    if (latencyCount_ == 0) {
        return 0.0;
    }
    return totalLatency_ / static_cast<double>(latencyCount_);
}

double Diagnostics::getMaxLatency() const {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    return maxLatency_;
}

double Diagnostics::getMinLatency() const {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    if (minLatency_ == std::numeric_limits<double>::max()) {
        return 0.0;
    }
    return minLatency_;
}

void Diagnostics::updateSignalMetrics(double snr, double ber, double cqi) {
    std::lock_guard<std::mutex> lock(linkMetricsMutex_);
    snr_ = snr;
    ber_ = ber;
    cqi_ = cqi;
}

void Diagnostics::packetSent() {
    packetsSent_.fetch_add(1, std::memory_order_relaxed);
}

void Diagnostics::packetReceived() {
    packetsReceived_.fetch_add(1, std::memory_order_relaxed);
}

void Diagnostics::packetLost() {
    packetsLost_.fetch_add(1, std::memory_order_relaxed);
}

void Diagnostics::recordLatency(const std::chrono::high_resolution_clock::time_point& sentTime,
                                const std::chrono::high_resolution_clock::time_point& receivedTime) {
    std::chrono::duration<double, std::milli> latency = receivedTime - sentTime;
    double latencyMs = latency.count();

    std::lock_guard<std::mutex> lock(latencyMutex_);
    totalLatency_ += latencyMs;
    if (latencyMs > maxLatency_) {
        maxLatency_ = latencyMs;
    }
    if (latencyMs < minLatency_) {
        minLatency_ = latencyMs;
    }
    latencyCount_++;
}

} // namespace Diagnostics
} // namespace RocketLink
