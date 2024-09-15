#include <benchmark/benchmark.h>
#include "Management/TelemetryBuffer.hpp"
#include "Management/CommandManager.hpp"
#include "AVC/AVCProtocol.hpp"
#include "AVC/Command.hpp"
#include "SCALPEL/Communicator.hpp"
#include <memory>
#include <vector>

// Mock Communicator for testing purposes
namespace SCALPEL {

class MockCommunicator : public Communicator {
public:
    MockCommunicator() : Communicator([](const std::vector<uint8_t>&){}) {}

    void send(const std::vector<uint8_t>& data) {
        // Mock send implementation (no operation)
    }

    // Note: receive() is likely private in the base class, so we don't override it here
};

} // namespace SCALPEL

// Benchmark for TelemetryBuffer::addTelemetry
static void BM_TelemetryBuffer_AddTelemetry(benchmark::State& state) {
    RocketLink::AVC::TelemetryBuffer buffer(state.range(0));
    RocketLink::AVC::Telemetry telemetry; // Assume default constructible
    for (auto _ : state) {
        buffer.addTelemetry(telemetry);
    }
}
BENCHMARK(BM_TelemetryBuffer_AddTelemetry)->Range(8, 8<<10);

// Benchmark for TelemetryBuffer::getLatestTelemetry
static void BM_TelemetryBuffer_GetLatestTelemetry(benchmark::State& state) {
    RocketLink::AVC::TelemetryBuffer buffer(state.range(0));
    RocketLink::AVC::Telemetry telemetry;
    buffer.addTelemetry(telemetry);
    for (auto _ : state) {
        auto latest = buffer.getLatestTelemetry();
        benchmark::DoNotOptimize(latest);
    }
}
BENCHMARK(BM_TelemetryBuffer_GetLatestTelemetry)->Range(8, 8<<10);

// Benchmark for CommandManager::addCommand
static void BM_CommandManager_AddCommand(benchmark::State& state) {
    auto communicator = std::make_shared<SCALPEL::MockCommunicator>();
    auto avcProtocol = std::make_shared<RocketLink::AVC::AVCProtocol>(communicator);
    RocketLink::AVC::CommandManager cmdManager(avcProtocol);
    cmdManager.start();
    RocketLink::AVC::Command command; // Assume default constructible
    for (auto _ : state) {
        cmdManager.addCommand(command, 1);
    }
    cmdManager.stop();
}
BENCHMARK(BM_CommandManager_AddCommand)->Range(8, 8<<10);

// Benchmark for CommandManager::handleAcknowledgment
static void BM_CommandManager_HandleAcknowledgment(benchmark::State& state) {
    auto communicator = std::make_shared<SCALPEL::MockCommunicator>();
    auto avcProtocol = std::make_shared<RocketLink::AVC::AVCProtocol>(communicator);
    RocketLink::AVC::CommandManager cmdManager(avcProtocol);
    cmdManager.start();
    RocketLink::AVC::Command command; // Assume default constructible
    cmdManager.addCommand(command, 1);
    for (auto _ : state) {
        cmdManager.handleAcknowledgment(static_cast<uint8_t>(0));
    }
    cmdManager.stop();
}
BENCHMARK(BM_CommandManager_HandleAcknowledgment)->Range(8, 8<<10);

// Benchmark for CommandManager::getNextCommand
static void BM_CommandManager_GetNextCommand(benchmark::State& state) {
    auto communicator = std::make_shared<SCALPEL::MockCommunicator>();
    auto avcProtocol = std::make_shared<RocketLink::AVC::AVCProtocol>(communicator);
    RocketLink::AVC::CommandManager cmdManager(avcProtocol);
    cmdManager.start();
    RocketLink::AVC::Command command; // Assume default constructible
    cmdManager.addCommand(command, 1);
    for (auto _ : state) {
        auto cmd = cmdManager.getNextCommand();
        benchmark::DoNotOptimize(cmd);
    }
    cmdManager.stop();
}
BENCHMARK(BM_CommandManager_GetNextCommand)->Range(8, 8<<10);

#ifndef COMBINED_BENCHMARK
BENCHMARK_MAIN();
#endif