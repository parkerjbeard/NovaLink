#include <benchmark/benchmark.h>
#include "SCALPEL/Packet.hpp"
#include <vector>
#include <random>

// Helper function to generate random payload
std::vector<uint8_t> generateRandomPayload(size_t size) {
    std::vector<uint8_t> payload(size);
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<uint8_t> dis(0, 255);
    for(auto& byte : payload) {
        byte = dis(gen);
    }
    return payload;
}

// Benchmark Packet::assemble
static void BM_Packet_Assemble(benchmark::State& state) {
    size_t payloadSize = state.range(0);
    auto payload = generateRandomPayload(payloadSize);
    SCALPEL::Packet packet(payload);
    for(auto _ : state) {
        benchmark::DoNotOptimize(packet.assemble());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Packet_Assemble)->Arg(10)->Arg(20)->Arg(28)->Complexity();

// Benchmark Packet::disassemble
static void BM_Packet_Disassemble(benchmark::State& state) {
    size_t payloadSize = state.range(0);
    auto payload = generateRandomPayload(payloadSize);
    SCALPEL::Packet packet(payload);
    auto assembled = packet.assemble();
    for(auto _ : state) {
        benchmark::DoNotOptimize(SCALPEL::Packet::disassemble(assembled));
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Packet_Disassemble)->Arg(10)->Arg(20)->Arg(28)->Complexity();

#ifndef COMBINED_BENCHMARK
BENCHMARK_MAIN();
#endif