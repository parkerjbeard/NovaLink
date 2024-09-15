#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include <cstddef>
#include <mutex>
#include <vector>
#include <memory>
#include <cassert>
#include <type_traits>
#include <stdexcept>

// Include headers for the types you intend to instantiate MemoryPool with
#include "../SCALPEL/Packet.hpp"
#include "../AVC/Telemetry.hpp"
#include "../AVC/Command.hpp"

template <typename T>
class MemoryPool {
public:
    // Constructor that initializes the pool with a specified number of objects
    explicit MemoryPool(size_t poolSize);

    // Destructor to clean up allocated memory
    ~MemoryPool();

    // Allocates an object from the pool
    T* allocate();

    // Returns an object to the pool
    void deallocate(T* obj);

    // Clears the entire pool, making all objects available for allocation
    void reset();

    // Returns the current pool size
    size_t getPoolSize() const;

    // Returns the number of available objects in the pool
    size_t getAvailableObjects() const;

private:
    // Disable copy constructor and assignment operator
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Structure for the free list
    struct FreeBlock {
        FreeBlock* next;
    };

    // Initializes the free list
    void initializePool(size_t poolSize);

    FreeBlock* freeList;                // Head of the free list
    mutable std::mutex poolMutex;       // Mutex for thread safety
    std::vector<std::unique_ptr<char[]>> memoryChunks; // Storage for memory blocks
    size_t objectSize;                   // Size of each object
    size_t poolSize;                     // Total size of the pool
    size_t availableObjects;             // Number of available objects

    static constexpr size_t ALIGNMENT = alignof(std::max_align_t);

    static size_t alignedSize(size_t size) {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }
};

// Template Implementations

template <typename T>
MemoryPool<T>::MemoryPool(size_t poolSize)
    : freeList(nullptr),
      objectSize(alignedSize(sizeof(T))),
      poolSize(poolSize),
      availableObjects(poolSize) {
    static_assert(sizeof(T) >= sizeof(FreeBlock*), "Object size must be at least as large as a pointer");
    initializePool(poolSize);
}

template <typename T>
MemoryPool<T>::~MemoryPool() {
    // No need to explicitly delete memory, as std::unique_ptr will handle it
}

template <typename T>
void MemoryPool<T>::initializePool(size_t poolSize) {
    size_t totalSize = objectSize * poolSize;
    std::unique_ptr<char[]> chunk(new char[totalSize]);
    memoryChunks.emplace_back(std::move(chunk));

    char* memBlock = memoryChunks.back().get();
    for (size_t i = 0; i < poolSize; ++i) {
        char* objPtr = memBlock + i * objectSize;
        deallocate(reinterpret_cast<T*>(objPtr));
    }
}

template <typename T>
T* MemoryPool<T>::allocate() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (!freeList) {
        throw std::runtime_error("Memory pool exhausted");
    }

    FreeBlock* head = freeList;
    freeList = head->next;
    --availableObjects;

    return new (head) T();
}

template <typename T>
void MemoryPool<T>::deallocate(T* obj) {
    if (!obj) return;

    obj->~T();

    std::lock_guard<std::mutex> lock(poolMutex);
    FreeBlock* block = reinterpret_cast<FreeBlock*>(obj);
    block->next = freeList;
    freeList = block;
    ++availableObjects;
}

template <typename T>
void MemoryPool<T>::reset() {
    std::lock_guard<std::mutex> lock(poolMutex);
    freeList = nullptr;
    availableObjects = poolSize;
    initializePool(poolSize);
}

template <typename T>
size_t MemoryPool<T>::getPoolSize() const {
    return poolSize;
}

template <typename T>
size_t MemoryPool<T>::getAvailableObjects() const {
    return availableObjects;
}

#endif // MEMORYPOOL_HPP
