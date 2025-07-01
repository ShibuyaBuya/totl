/*
 * ESP32-OS Kernel Implementation
 * Core kernel functionality implementation
 */

#include "kernel.h"
#include <esp_system.h>

Kernel::Kernel() : scheduler(nullptr), memoryManager(nullptr), 
                   systemMutex(nullptr), initialized(false), healthy(false),
                   bootTime(0), uptime(0), totalTasks(0), freeMem(0), minFreeMem(0) {
}

Kernel::~Kernel() {
    shutdown();
}

bool Kernel::init() {
    if (initialized) {
        return true;
    }
    
    // Create system mutex for thread safety
    systemMutex = xSemaphoreCreateMutex();
    if (!systemMutex) {
        Serial.println("Kernel: Failed to create system mutex");
        return false;
    }
    
    // Initialize memory manager
    memoryManager = new MemoryManager();
    if (!memoryManager || !memoryManager->init()) {
        Serial.println("Kernel: Failed to initialize memory manager");
        return false;
    }
    
    // Initialize scheduler
    scheduler = new Scheduler();
    if (!scheduler || !scheduler->init()) {
        Serial.println("Kernel: Failed to initialize scheduler");
        return false;
    }
    
    // Record boot time
    bootTime = millis();
    
    // System is now healthy and initialized
    healthy = true;
    initialized = true;
    
    Serial.println("Kernel: Core system initialized successfully");
    return true;
}

void Kernel::shutdown() {
    if (!initialized) {
        return;
    }
    
    healthy = false;
    
    // Clean up scheduler
    if (scheduler) {
        delete scheduler;
        scheduler = nullptr;
    }
    
    // Clean up memory manager
    if (memoryManager) {
        delete memoryManager;
        memoryManager = nullptr;
    }
    
    // Clean up mutex
    if (systemMutex) {
        vSemaphoreDelete(systemMutex);
        systemMutex = nullptr;
    }
    
    initialized = false;
}

bool Kernel::createTask(const char* name, TaskFunction_t taskFunction, 
                       uint32_t stackSize, void* parameters, UBaseType_t priority) {
    if (!initialized || !scheduler) {
        return false;
    }
    
    if (takeMutex(1000)) {
        bool result = scheduler->createTask(name, taskFunction, stackSize, parameters, priority);
        if (result) {
            totalTasks++;
        }
        giveMutex();
        return result;
    }
    
    return false;
}

bool Kernel::deleteTask(const char* name) {
    if (!initialized || !scheduler) {
        return false;
    }
    
    if (takeMutex(1000)) {
        bool result = scheduler->deleteTask(name);
        if (result) {
            totalTasks--;
        }
        giveMutex();
        return result;
    }
    
    return false;
}

void Kernel::suspendTask(const char* name) {
    if (initialized && scheduler && takeMutex(1000)) {
        scheduler->suspendTask(name);
        giveMutex();
    }
}

void Kernel::resumeTask(const char* name) {
    if (initialized && scheduler && takeMutex(1000)) {
        scheduler->resumeTask(name);
        giveMutex();
    }
}

void* Kernel::allocateMemory(size_t size) {
    if (!initialized || !memoryManager) {
        return nullptr;
    }
    
    return memoryManager->allocate(size);
}

void Kernel::freeMemory(void* ptr) {
    if (initialized && memoryManager && ptr) {
        memoryManager->free(ptr);
    }
}

uint32_t Kernel::getFreeMemory() {
    if (!initialized) {
        return 0;
    }
    
    return esp_get_free_heap_size();
}

uint32_t Kernel::getMinFreeMemory() {
    if (!initialized) {
        return 0;
    }
    
    return esp_get_minimum_free_heap_size();
}

void Kernel::updateSystemStats() {
    if (!initialized) {
        return;
    }
    
    uptime = (millis() - bootTime) / 1000; // Convert to seconds
    freeMem = getFreeMemory();
    minFreeMem = getMinFreeMemory();
    
    // Check system health
    if (freeMem < 10240) { // Less than 10KB free memory is critical
        Serial.println("WARNING: Low memory condition detected");
        healthy = false;
    } else {
        healthy = true;
    }
}

void Kernel::reboot() {
    Serial.println("Kernel: System reboot requested");
    delay(1000);
    ESP.restart();
}

void Kernel::enterLowPowerMode() {
    Serial.println("Kernel: Entering low power mode");
    // Suspend non-critical tasks
    // This is a simplified implementation
    esp_deep_sleep_start();
}

bool Kernel::takeMutex(TickType_t timeout) {
    if (!systemMutex) {
        return false;
    }
    
    return xSemaphoreTake(systemMutex, timeout) == pdTRUE;
}

void Kernel::giveMutex() {
    if (systemMutex) {
        xSemaphoreGive(systemMutex);
    }
}
