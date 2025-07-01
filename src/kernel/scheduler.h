/*
 * ESP32-OS Task Scheduler Header
 * Task management and scheduling functionality
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../config/config.h"

struct TaskInfo {
    char name[32];
    TaskHandle_t handle;
    uint32_t stackSize;
    UBaseType_t priority;
    eTaskState state;
    uint32_t stackHighWaterMark;
    bool active;
};

class Scheduler {
private:
    TaskInfo tasks[MAX_TASKS];
    uint16_t taskCount;
    SemaphoreHandle_t schedulerMutex;
    
    int findTaskByName(const char* name);
    int findFreeTaskSlot();
    
public:
    Scheduler();
    ~Scheduler();
    
    bool init();
    void shutdown();
    
    // Task management
    bool createTask(const char* name, TaskFunction_t taskFunction, 
                   uint32_t stackSize, void* parameters, UBaseType_t priority);
    bool deleteTask(const char* name);
    bool suspendTask(const char* name);
    bool resumeTask(const char* name);
    
    // Task information
    uint16_t getTaskCount() const { return taskCount; }
    bool getTaskInfo(const char* name, TaskInfo& info);
    void listTasks();
    
    // System tasks info
    void printTaskStats();
    uint32_t getTotalStackUsage();
    
    // Scheduler control
    void startScheduler();
    void suspendScheduler();
    void resumeScheduler();
};

#endif // SCHEDULER_H
