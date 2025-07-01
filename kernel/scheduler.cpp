/*
 * ESP32-OS Task Scheduler Implementation
 */

#include "scheduler.h"

Scheduler::Scheduler() : taskCount(0), schedulerMutex(nullptr) {
    // Initialize task array
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = false;
        tasks[i].handle = nullptr;
        memset(tasks[i].name, 0, sizeof(tasks[i].name));
    }
}

Scheduler::~Scheduler() {
    shutdown();
}

bool Scheduler::init() {
    // Create mutex for thread-safe operations
    schedulerMutex = xSemaphoreCreateMutex();
    if (!schedulerMutex) {
        Serial.println("Scheduler: Failed to create mutex");
        return false;
    }
    
    Serial.println("Scheduler: Task scheduler initialized");
    return true;
}

void Scheduler::shutdown() {
    // Delete all active tasks
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active && tasks[i].handle) {
            vTaskDelete(tasks[i].handle);
            tasks[i].active = false;
            tasks[i].handle = nullptr;
        }
    }
    
    taskCount = 0;
    
    if (schedulerMutex) {
        vSemaphoreDelete(schedulerMutex);
        schedulerMutex = nullptr;
    }
}

bool Scheduler::createTask(const char* name, TaskFunction_t taskFunction, 
                          uint32_t stackSize, void* parameters, UBaseType_t priority) {
    if (!name || !taskFunction || !schedulerMutex) {
        return false;
    }
    
    if (xSemaphoreTake(schedulerMutex, 1000) != pdTRUE) {
        return false;
    }
    
    // Check if task already exists
    if (findTaskByName(name) >= 0) {
        xSemaphoreGive(schedulerMutex);
        Serial.print("Scheduler: Task '");
        Serial.print(name);
        Serial.println("' already exists");
        return false;
    }
    
    // Find free slot
    int slot = findFreeTaskSlot();
    if (slot < 0) {
        xSemaphoreGive(schedulerMutex);
        Serial.println("Scheduler: No free task slots available");
        return false;
    }
    
    // Create FreeRTOS task
    BaseType_t result = xTaskCreate(
        taskFunction,
        name,
        stackSize / sizeof(StackType_t),
        parameters,
        priority,
        &tasks[slot].handle
    );
    
    if (result != pdPASS) {
        xSemaphoreGive(schedulerMutex);
        Serial.print("Scheduler: Failed to create task '");
        Serial.print(name);
        Serial.println("'");
        return false;
    }
    
    // Fill task info
    strncpy(tasks[slot].name, name, sizeof(tasks[slot].name) - 1);
    tasks[slot].stackSize = stackSize;
    tasks[slot].priority = priority;
    tasks[slot].active = true;
    tasks[slot].state = eReady;
    
    taskCount++;
    
    xSemaphoreGive(schedulerMutex);
    
    Serial.print("Scheduler: Task '");
    Serial.print(name);
    Serial.println("' created successfully");
    
    return true;
}

bool Scheduler::deleteTask(const char* name) {
    if (!name || !schedulerMutex) {
        return false;
    }
    
    if (xSemaphoreTake(schedulerMutex, 1000) != pdTRUE) {
        return false;
    }
    
    int slot = findTaskByName(name);
    if (slot < 0) {
        xSemaphoreGive(schedulerMutex);
        return false;
    }
    
    // Delete FreeRTOS task
    if (tasks[slot].handle) {
        vTaskDelete(tasks[slot].handle);
    }
    
    // Clear task info
    tasks[slot].active = false;
    tasks[slot].handle = nullptr;
    memset(tasks[slot].name, 0, sizeof(tasks[slot].name));
    
    taskCount--;
    
    xSemaphoreGive(schedulerMutex);
    
    Serial.print("Scheduler: Task '");
    Serial.print(name);
    Serial.println("' deleted");
    
    return true;
}

bool Scheduler::suspendTask(const char* name) {
    if (!name || !schedulerMutex) {
        return false;
    }
    
    if (xSemaphoreTake(schedulerMutex, 1000) != pdTRUE) {
        return false;
    }
    
    int slot = findTaskByName(name);
    if (slot >= 0 && tasks[slot].handle) {
        vTaskSuspend(tasks[slot].handle);
        tasks[slot].state = eSuspended;
        xSemaphoreGive(schedulerMutex);
        return true;
    }
    
    xSemaphoreGive(schedulerMutex);
    return false;
}

bool Scheduler::resumeTask(const char* name) {
    if (!name || !schedulerMutex) {
        return false;
    }
    
    if (xSemaphoreTake(schedulerMutex, 1000) != pdTRUE) {
        return false;
    }
    
    int slot = findTaskByName(name);
    if (slot >= 0 && tasks[slot].handle) {
        vTaskResume(tasks[slot].handle);
        tasks[slot].state = eReady;
        xSemaphoreGive(schedulerMutex);
        return true;
    }
    
    xSemaphoreGive(schedulerMutex);
    return false;
}

void Scheduler::listTasks() {
    if (!schedulerMutex) {
        return;
    }
    
    if (xSemaphoreTake(schedulerMutex, 1000) != pdTRUE) {
        return;
    }
    
    Serial.println("Active Tasks:");
    Serial.println("Name              Priority  State     Stack");
    Serial.println("----------------------------------------");
    
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active) {
            // Update task state
            if (tasks[i].handle) {
                tasks[i].state = eTaskGetState(tasks[i].handle);
                tasks[i].stackHighWaterMark = uxTaskGetStackHighWaterMark(tasks[i].handle);
            }
            
            Serial.printf("%-16s %8d  %-8s %6d\n", 
                         tasks[i].name,
                         tasks[i].priority,
                         (tasks[i].state == eReady) ? "Ready" :
                         (tasks[i].state == eRunning) ? "Running" :
                         (tasks[i].state == eBlocked) ? "Blocked" :
                         (tasks[i].state == eSuspended) ? "Suspended" : "Unknown",
                         tasks[i].stackHighWaterMark);
        }
    }
    
    xSemaphoreGive(schedulerMutex);
}

int Scheduler::findTaskByName(const char* name) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active && strcmp(tasks[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int Scheduler::findFreeTaskSlot() {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].active) {
            return i;
        }
    }
    return -1;
}

void Scheduler::printTaskStats() {
    Serial.print("Total Tasks: ");
    Serial.println(taskCount);
    Serial.print("Free Task Slots: ");
    Serial.println(MAX_TASKS - taskCount);
}
