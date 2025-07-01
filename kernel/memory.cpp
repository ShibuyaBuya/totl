/*
 * ESP32-OS Memory Manager Implementation
 */

#include "memory.h"
#include <esp_heap_caps.h>

MemoryManager::MemoryManager() : memoryMutex(nullptr), totalAllocated(0), 
                                peakAllocated(0), allocationCount(0), freeCount(0) {
    // Initialize memory blocks
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        blocks[i].ptr = nullptr;
        blocks[i].size = 0;
        blocks[i].allocated = false;
        blocks[i].timestamp = 0;
        memset(blocks[i].tag, 0, sizeof(blocks[i].tag));
    }
}

MemoryManager::~MemoryManager() {
    shutdown();
}

bool MemoryManager::init() {
    // Create mutex for thread-safe operations
    memoryMutex = xSemaphoreCreateMutex();
    if (!memoryMutex) {
        Serial.println("MemoryManager: Failed to create mutex");
        return false;
    }
    
    Serial.println("MemoryManager: Memory manager initialized");
    return true;
}

void MemoryManager::shutdown() {
    if (memoryMutex) {
        xSemaphoreTake(memoryMutex, portMAX_DELAY);
        
        // Free all allocated blocks
        for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
            if (blocks[i].allocated && blocks[i].ptr) {
                ::free(blocks[i].ptr);
                blocks[i].ptr = nullptr;
                blocks[i].allocated = false;
            }
        }
        
        xSemaphoreGive(memoryMutex);
        vSemaphoreDelete(memoryMutex);
        memoryMutex = nullptr;
    }
}

void* MemoryManager::allocate(size_t size, const char* tag) {
    if (size == 0 || !memoryMutex) {
        return nullptr;
    }
    
    // Align size to memory boundary
    size = (size + MEMORY_ALIGNMENT - 1) & ~(MEMORY_ALIGNMENT - 1);
    
    if (xSemaphoreTake(memoryMutex, 1000) != pdTRUE) {
        return nullptr;
    }
    
    // Find free block slot
    int slot = findFreeBlock();
    if (slot < 0) {
        xSemaphoreGive(memoryMutex);
        Serial.println("MemoryManager: No free block slots available");
        return nullptr;
    }
    
    // Allocate memory
    void* ptr = malloc(size);
    if (!ptr) {
        xSemaphoreGive(memoryMutex);
        Serial.print("MemoryManager: Failed to allocate ");
        Serial.print(size);
        Serial.println(" bytes");
        return nullptr;
    }
    
    // Record allocation
    blocks[slot].ptr = ptr;
    blocks[slot].size = size;
    blocks[slot].allocated = true;
    blocks[slot].timestamp = millis();
    strncpy(blocks[slot].tag, tag ? tag : "unknown", sizeof(blocks[slot].tag) - 1);
    
    totalAllocated += size;
    allocationCount++;
    
    if (totalAllocated > peakAllocated) {
        peakAllocated = totalAllocated;
    }
    
    xSemaphoreGive(memoryMutex);
    
#if DEBUG_MEMORY
    Serial.print("MemoryManager: Allocated ");
    Serial.print(size);
    Serial.print(" bytes at 0x");
    Serial.print((uint32_t)ptr, HEX);
    Serial.print(" (tag: ");
    Serial.print(tag ? tag : "unknown");
    Serial.println(")");
#endif
    
    return ptr;
}

void MemoryManager::free(void* ptr) {
    if (!ptr || !memoryMutex) {
        return;
    }
    
    if (xSemaphoreTake(memoryMutex, 1000) != pdTRUE) {
        return;
    }
    
    // Find block
    int slot = findBlockByPtr(ptr);
    if (slot < 0) {
        xSemaphoreGive(memoryMutex);
        Serial.print("MemoryManager: Attempted to free untracked pointer 0x");
        Serial.println((uint32_t)ptr, HEX);
        return;
    }
    
    // Free memory
    ::free(ptr);
    
    totalAllocated -= blocks[slot].size;
    freeCount++;
    
#if DEBUG_MEMORY
    Serial.print("MemoryManager: Freed ");
    Serial.print(blocks[slot].size);
    Serial.print(" bytes at 0x");
    Serial.print((uint32_t)ptr, HEX);
    Serial.print(" (tag: ");
    Serial.print(blocks[slot].tag);
    Serial.println(")");
#endif
    
    // Clear block info
    blocks[slot].ptr = nullptr;
    blocks[slot].size = 0;
    blocks[slot].allocated = false;
    blocks[slot].timestamp = 0;
    memset(blocks[slot].tag, 0, sizeof(blocks[slot].tag));
    
    xSemaphoreGive(memoryMutex);
}

void* MemoryManager::reallocate(void* ptr, size_t newSize) {
    if (newSize == 0) {
        free(ptr);
        return nullptr;
    }
    
    if (!ptr) {
        return allocate(newSize);
    }
    
    // Find existing block
    if (xSemaphoreTake(memoryMutex, 1000) != pdTRUE) {
        return nullptr;
    }
    
    int slot = findBlockByPtr(ptr);
    if (slot < 0) {
        xSemaphoreGive(memoryMutex);
        return nullptr;
    }
    
    size_t oldSize = blocks[slot].size;
    char tag[16];
    strncpy(tag, blocks[slot].tag, sizeof(tag));
    
    xSemaphoreGive(memoryMutex);
    
    // Allocate new block
    void* newPtr = allocate(newSize, tag);
    if (!newPtr) {
        return nullptr;
    }
    
    // Copy data
    memcpy(newPtr, ptr, (oldSize < newSize) ? oldSize : newSize);
    
    // Free old block
    free(ptr);
    
    return newPtr;
}

int MemoryManager::findFreeBlock() {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (!blocks[i].allocated) {
            return i;
        }
    }
    return -1;
}

int MemoryManager::findBlockByPtr(void* ptr) {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (blocks[i].allocated && blocks[i].ptr == ptr) {
            return i;
        }
    }
    return -1;
}

void MemoryManager::printMemoryMap() {
    if (!memoryMutex) {
        return;
    }
    
    if (xSemaphoreTake(memoryMutex, 1000) != pdTRUE) {
        return;
    }
    
    Serial.println("Memory Map:");
    Serial.println("Address    Size     Tag              Age(ms)");
    Serial.println("-----------------------------------------------");
    
    uint32_t currentTime = millis();
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (blocks[i].allocated) {
            Serial.printf("0x%08X %8d %-16s %8d\n",
                         (uint32_t)blocks[i].ptr,
                         blocks[i].size,
                         blocks[i].tag,
                         currentTime - blocks[i].timestamp);
        }
    }
    
    xSemaphoreGive(memoryMutex);
}

void MemoryManager::printStatistics() {
    Serial.println("Memory Statistics:");
    Serial.print("Total Allocated: ");
    Serial.print(totalAllocated);
    Serial.println(" bytes");
    
    Serial.print("Peak Allocated: ");
    Serial.print(peakAllocated);
    Serial.println(" bytes");
    
    Serial.print("Allocations: ");
    Serial.println(allocationCount);
    
    Serial.print("Frees: ");
    Serial.println(freeCount);
    
    Serial.print("Available Heap: ");
    Serial.print(getAvailableHeap());
    Serial.println(" bytes");
    
    Serial.print("Largest Free Block: ");
    Serial.print(getLargestFreeBlock());
    Serial.println(" bytes");
}

uint32_t MemoryManager::getAvailableHeap() {
    return esp_get_free_heap_size();
}

uint32_t MemoryManager::getLargestFreeBlock() {
    return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
}
