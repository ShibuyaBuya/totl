/*
 * ESP32-OS Kernel Header
 * Core kernel functionality including initialization and system management
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <vector>
#include <string>
#include "../config/config.h"
#include "scheduler.h"
#include "memory.h"

class Kernel {
private:
    Scheduler* scheduler;
    MemoryManager* memoryManager;
    SemaphoreHandle_t systemMutex;
    bool initialized;
    bool healthy;
    
    // System statistics
    unsigned long bootTime;
    unsigned long uptime;
    uint32_t totalTasks;
    uint32_t freeMem;
    uint32_t minFreeMem;
    
public:
    Kernel();
    ~Kernel();
    
    // Core kernel functions
    bool init();
    void diskList(std::vector<std::string> &disks);
    
    void shutdown();
    bool isHealthy() const { return healthy; }
    
    // Task management (wrapper for scheduler)
    bool createTask(const char* name, TaskFunction_t taskFunction, 
                   uint32_t stackSize, void* parameters, UBaseType_t priority);
    bool deleteTask(const char* name);
    void suspendTask(const char* name);
    void resumeTask(const char* name);
    
    // Memory management (wrapper for memory manager)
    void* allocateMemory(size_t size);
    void freeMemory(void* ptr);
    uint32_t getFreeMemory();
    uint32_t getMinFreeMemory();
    
    // System information
    void updateSystemStats();
    unsigned long getUptime() const { return uptime; }
    uint32_t getTotalTasks() const { return totalTasks; }
    const char* getVersion() const { return OS_VERSION; }
    
    // System control
    void reboot();
    void enterLowPowerMode();
    
    // Mutex operations
    bool takeMutex(TickType_t timeout = portMAX_DELAY);
    void giveMutex();
    
    // Access to subsystems
    Scheduler* getScheduler() { return scheduler; }
    MemoryManager* getMemoryManager() { return memoryManager; }
};

// Global kernel instance declaration
extern Kernel* kernel;

#endif // KERNEL_H
