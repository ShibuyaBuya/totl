/*
 * ESP32 Custom Operating System
 * Main Arduino sketch file - Entry point and system initialization
 * 
 * This file contains the setup() and loop() functions required by Arduino framework
 * and initializes all OS components including kernel, shell, and hardware abstraction
 */
#include "FS.h"
#include "SPIFFS.h"
#include "kernel/kernel.h"
#include "shell/shell.h"
#include "hal/hal.h"
#include "filesystem/fs.h"
#include "config/config.h"
#define FORMAT_SPIFFS_IF_FAILED true
// Global system objects
Kernel* kernel;
Shell* shell;
HAL* hal;
FileSystem* fs_;

void setup() {
    // Initialize serial communication for shell interface
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000); // Allow serial to stabilize
    
    // Print boot banner
     Serial.println("========================================");
    Serial.println("ESP32-OS v1.0 - Custom Operating System");
    Serial.println("========================================");
    Serial.println("Initializing system components...");
    
    // Initialize Hardware Abstraction Layer first
    hal = new HAL();
    if (!hal->init()) {
        Serial.println("FATAL: HAL initialization failed");
        while(1) delay(1000); // Halt system
    }
    Serial.println("[OK] Hardware Abstraction Layer initialized");
    
    // Initialize File System
    fs_ = new FileSystem();
    if (!fs_->init()) {
        Serial.println("WARNING: File System initialization failed");
        // Continue without filesystem - non-critical
    } else {
        Serial.println("[OK] File System initialized");
    }
    
    // Initialize Kernel (task scheduler and memory management)
    kernel = new Kernel();
    if (!kernel->init()) {
        Serial.println("FATAL: Kernel initialization failed");
        while(1) delay(1000); // Halt system
    }
    Serial.println("[OK] Kernel initialized");
    
    // Initialize Shell interface
    shell = new Shell();
    if (!shell->init()) {
        Serial.println("FATAL: Shell initialization failed");
        while(1) delay(1000); // Halt system
    }
    Serial.println("[OK] Shell initialized");
    
    // System initialization complete
    Serial.println("========================================");
    Serial.println("System boot complete!");
    Serial.println("Type 'help' for available commands");
    Serial.println("========================================");
    
    // Start the shell task
    kernel->createTask("shell_task", shellTask, 4096, NULL, 1);
    
    // Start system monitoring task
    kernel->createTask("monitor_task", monitorTask, 2048, NULL, 0);
}

void loop() {
    // Main loop - let FreeRTOS handle task scheduling
    // Keep the watchdog happy
    delay(1000);
    
    // Check for system critical errors
    if (kernel && !kernel->isHealthy()) {
        Serial.println("CRITICAL: Kernel health check failed - rebooting...");
        delay(1000);
        ESP.restart();
    }
}

// Shell task function - runs the command interface
void shellTask(void* parameter) {
    while (true) {
        if (shell) {
            shell->processInput();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent watchdog issues
    }
}

// System monitoring task - monitors system health and resources
void monitorTask(void* parameter) {
    while (true) {
        if (kernel) {
            kernel->updateSystemStats();
        }
        
        // Monitor every 5 seconds
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
