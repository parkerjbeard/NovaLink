#ifndef ROCKETLINK_DIAGNOSTICS_DIAGNOSTICS_HPP
#define ROCKETLINK_DIAGNOSTICS_DIAGNOSTICS_HPP

#include "PhysicalLayer/RadioInterface.hpp"
#include <chrono>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace RocketLink {
namespace Diagnostics {

/**
 * @brief Class responsible for collecting and providing diagnostics data
 *        related to radio communication.
 */
class Diagnostics {
public:
    // Updated constructor without RadioInterface reference
    Diagnostics();

    // Link Quality Metrics
    double getSignalToNoiseRatio() const;
    double getBitErrorRate() const;
    double getCarrierToInterferenceRatio() const;

    // Packet Loss Tracking
    uint32_t getPacketsSent() const;
    uint32_t getPacketsReceived() const;
    uint32_t getPacketsLost() const;
    double getPacketLossRate() const;

    // Latency Measurement
    double getAverageLatency() const;
    double getMaxLatency() const;
    double getMinLatency() const;

    // Methods to update diagnostics
    void updateSignalMetrics(double snr, double ber, double cqi);
    void packetSent();
    void packetReceived();
    void packetLost();
    void recordLatency(const std::chrono::high_resolution_clock::time_point& sentTime,
                      const std::chrono::high_resolution_clock::time_point& receivedTime);

private:
    // Link Quality Metrics
    double snr_;
    double ber_;
    double cqi_;
    mutable std::mutex linkMetricsMutex_;

    // Packet Loss Tracking
    std::atomic<uint32_t> packetsSent_;
    std::atomic<uint32_t> packetsReceived_;
    std::atomic<uint32_t> packetsLost_;

    // Latency Measurement
    double totalLatency_;
    double maxLatency_;
    double minLatency_;
    uint32_t latencyCount_;
    mutable std::mutex latencyMutex_;

};

} // namespace Diagnostics
} // namespace RocketLink

#endif // ROCKETLINK_DIAGNOSTICS_DIAGNOSTICS_HPP
