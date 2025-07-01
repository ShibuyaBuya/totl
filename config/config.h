/*
 * ESP32-OS Configuration Header
 * System-wide configuration constants and settings
 */

#ifndef CONFIG_H
#define CONFIG_H

// Serial Communication Settings
#define SERIAL_BAUD_RATE 115200
#define SHELL_BUFFER_SIZE 256
#define SHELL_MAX_ARGS 16

// Memory Management Settings
#define MAX_HEAP_SIZE (200 * 1024)  // 200KB heap limit
#define MEMORY_ALIGNMENT 4
#define MAX_MEMORY_BLOCKS 64

// Task Scheduler Settings
#define MAX_TASKS 16
#define DEFAULT_STACK_SIZE 2048
#define IDLE_TASK_STACK_SIZE 1024

// File System Settings
#define FS_MAX_FILES 32
#define FS_MAX_PATH_LENGTH 64
#define FS_BLOCK_SIZE 512

// Hardware Settings
#define LED_BUILTIN_PIN 2
#define WATCHDOG_TIMEOUT_SECONDS 30

// System Information
#define OS_VERSION "1.0.0"
#define OS_BUILD_DATE __DATE__
#define OS_BUILD_TIME __TIME__

// Shell Prompt
#define SHELL_PROMPT "esp32-os> "

// Debug Settings
#define DEBUG_ENABLED 1
#define DEBUG_MEMORY 1
#define DEBUG_SCHEDULER 1

#endif // CONFIG_H
