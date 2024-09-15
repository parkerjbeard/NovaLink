#include <benchmark/benchmark.h>
#include "SCALPEL/Checksum.hpp"
#include "SCALPEL/COBS.hpp"
#include "SCALPEL/Packet.hpp"

// Benchmark for Checksum::calculateCRC8
static void BM_CalculateCRC8(benchmark::State& state) {
    std::vector<uint8_t> data(state.range(0), 0xAB); // Sample data
    for (auto _ : state) {
        uint8_t crc = SCALPEL::Checksum::calculateCRC8(data.data(), data.size());
        benchmark::DoNotOptimize(crc);
    }
}
BENCHMARK(BM_CalculateCRC8)->Range(8, 8<<10);

// Benchmark for Checksum::validateCRC8
static void BM_ValidateCRC8(benchmark::State& state) {
    std::vector<uint8_t> data(state.range(0), 0xCD);
    uint8_t checksum = SCALPEL::Checksum::calculateCRC8(data.data(), data.size());
    for (auto _ : state) {
        bool valid = SCALPEL::Checksum::validateCRC8(checksum, data.data(), data.size());
        benchmark::DoNotOptimize(valid);
    }
}
BENCHMARK(BM_ValidateCRC8)->Range(8, 8<<10);

// Benchmark for COBS::encode
static void BM_COBS_Encode(benchmark::State& state) {
    std::vector<uint8_t> input(state.range(0), 0x00);
    SCALPEL::COBS cobs;
    for (auto _ : state) {
        SCALPEL::COBS::COBSResult result = cobs.encode(input);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_COBS_Encode)->Range(8, 8<<10);

// Benchmark for COBS::decode
static void BM_COBS_Decode(benchmark::State& state) {
    std::vector<uint8_t> input(state.range(0), 0x00);
    SCALPEL::COBS cobs;
    SCALPEL::COBS::COBSResult encoded = cobs.encode(input);
    for (auto _ : state) {
        std::vector<uint8_t> decoded = cobs.decode(encoded.encodedPayload, encoded.index);
        benchmark::DoNotOptimize(decoded);
    }
}
BENCHMARK(BM_COBS_Decode)->Range(8, 8<<10);

// Benchmark for Packet::assemble
static void BM_Packet_Assemble(benchmark::State& state) {
    std::vector<uint8_t> payload(state.range(0), 0xEF);
    SCALPEL::Packet packet(payload);
    for (auto _ : state) {
        std::vector<uint8_t> assembled = packet.assemble();
        benchmark::DoNotOptimize(assembled);
    }
}
BENCHMARK(BM_Packet_Assemble)->Range(8, 28);

// Benchmark for Packet::disassemble
static void BM_Packet_Disassemble(benchmark::State& state) {
    std::vector<uint8_t> payload(state.range(0), 0xEF);
    SCALPEL::Packet packet(payload);
    std::vector<uint8_t> assembled = packet.assemble();
    for (auto _ : state) {
        SCALPEL::Packet pkt = SCALPEL::Packet::disassemble(assembled);
        benchmark::DoNotOptimize(pkt);
    }
}
BENCHMARK(BM_Packet_Disassemble)->Range(8, 28);

#ifndef COMBINED_BENCHMARK
BENCHMARK_MAIN();
#endif
