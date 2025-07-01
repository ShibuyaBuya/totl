/*
 * ESP32-OS Memory Manager Header
 * Memory allocation and management functionality
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "../config/config.h"

struct MemoryBlock {
    void* ptr;
    size_t size;
    bool allocated;
    uint32_t timestamp;
    char tag[16]; // For debugging
};

class MemoryManager {
private:
    MemoryBlock blocks[MAX_MEMORY_BLOCKS];
    SemaphoreHandle_t memoryMutex;
    uint32_t totalAllocated;
    uint32_t peakAllocated;
    uint32_t allocationCount;
    uint32_t freeCount;
    
    int findFreeBlock();
    int findBlockByPtr(void* ptr);
    void defragment();
    
public:
    MemoryManager();
    ~MemoryManager();
    
    bool init();
    void shutdown();
    
    // Memory allocation
    void* allocate(size_t size, const char* tag = "unknown");
    void free(void* ptr);
    void* reallocate(void* ptr, size_t newSize);
    
    // Memory information
    uint32_t getTotalAllocated() const { return totalAllocated; }
    uint32_t getPeakAllocated() const { return peakAllocated; }
    uint32_t getAllocationCount() const { return allocationCount; }
    uint32_t getFreeCount() const { return freeCount; }
    uint32_t getFragmentation();
    
    // Memory debugging
    void printMemoryMap();
    void printStatistics();
    bool checkIntegrity();
    
    // Heap management
    uint32_t getAvailableHeap();
    uint32_t getLargestFreeBlock();
};

#endif // MEMORY_H
