#include <benchmark/benchmark.h>
#include "../../src/Utils/MemoryPool.hpp"
#include "../../src/Utils/Logger.hpp"
#include <vector>
#include <string>

// Benchmark for MemoryPool
class DummyObject {
public:
    int value;
    void* padding; // Add padding to ensure the object is at least as large as a pointer
    DummyObject() : value(0), padding(nullptr) {}
};

static void BM_MemoryPoolAllocateAndDeallocate(benchmark::State& state) {
    const size_t poolSize = 1000;
    MemoryPool<DummyObject> pool(poolSize);

    for (auto _ : state) {
        std::vector<DummyObject*> objects;
        objects.reserve(poolSize);

        for (size_t i = 0; i < poolSize; ++i) {
            objects.push_back(pool.allocate());
        }

        for (auto obj : objects) {
            pool.deallocate(obj);
        }
    }
}
BENCHMARK(BM_MemoryPoolAllocateAndDeallocate);

static void BM_StandardNewDelete(benchmark::State& state) {
    const size_t poolSize = 1000;

    for (auto _ : state) {
        std::vector<DummyObject*> objects;
        objects.reserve(poolSize);

        for (size_t i = 0; i < poolSize; ++i) {
            objects.push_back(new DummyObject());
        }

        for (auto obj : objects) {
            delete obj;
        }
    }
}
BENCHMARK(BM_StandardNewDelete);

// Benchmark for Logger
static void BM_LoggerPerformance(benchmark::State& state) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(LogLevel::DEBUG);

    std::stringstream ss;
    logger.addOutput(ss);

    for (auto _ : state) {
        logger.log(LogLevel::INFO, "This is a test log message");
    }
}
BENCHMARK(BM_LoggerPerformance);

#ifndef COMBINED_BENCHMARK
BENCHMARK_MAIN();
#endif
