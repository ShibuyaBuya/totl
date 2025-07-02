/*
 * ESP32-OS Shell Commands Implementation
 */

#include "commands.h"
#include "../kernel/kernel.h"
#include "../hal/hal.h"
#include "../filesystem/fs.h"
#include <WiFi.h>

// External references
extern Kernel* kernel;
extern HAL* hal;
extern FileSystem* fs_;

// Command table
const Command Commands::commandList[] = {
    {"help", "Show available commands", cmd_help},
    {"ls", "List files and directories", cmd_ls},
    {"ps", "Show running processes", cmd_ps},
    {"free", "Show memory usage", cmd_free},
    {"reboot", "Restart the system", cmd_reboot},
    {"info", "Show system information", cmd_info},
    {"uptime", "Show system uptime", cmd_uptime},
    {"tasks", "Show task information", cmd_tasks},
    {"mem", "Show detailed memory information", cmd_mem},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Echo text to output", cmd_echo},
    {"sleep", "Sleep for specified seconds", cmd_sleep},
    {"led", "Control built-in LED", cmd_led},
    {"wifi", "WiFi management commands", cmd_wifi}
};

const int Commands::commandCount = sizeof(commandList) / sizeof(Command);

Commands::Commands() {
}

Commands::~Commands() {
    shutdown();
}

bool Commands::init() {
    Serial.println("Commands: Command processor initialized");
    return true;
}

void Commands::shutdown() {
    // Nothing to clean up
}

bool Commands::execute(const char* cmd, char args[][32], int argCount) {
    if (!cmd) {
        return false;
    }
    
    // Find and execute command
    for (int i = 0; i < commandCount; i++) {
        if (strcasecmp(cmd, commandList[i].name) == 0) {
            commandList[i].handler(args, argCount);
            return true;
        }
    }
    
    return false;
}

void Commands::listCommands() {
    Serial.println("Available commands:");
    for (int i = 0; i < commandCount; i++) {
        Serial.printf("  %-12s - %s\n", commandList[i].name, commandList[i].description);
    }
}

bool Commands::isValidCommand(const char* cmd) {
    if (!cmd) return false;
    
    for (int i = 0; i < commandCount; i++) {
        if (strcasecmp(cmd, commandList[i].name) == 0) {
            return true;
        }
    }
    return false;
}

const char* Commands::getCommandDescription(const char* cmd) {
    if (!cmd) return nullptr;
    
    for (int i = 0; i < commandCount; i++) {
        if (strcasecmp(cmd, commandList[i].name) == 0) {
            return commandList[i].description;
        }
    }
    return nullptr;
}

// Command implementations

void Commands::cmd_help(char args[][32], int argCount) {
    if (argCount > 0) {
        // Show help for specific command
        
        const char* desc;
        for (int i = 0; i < commandCount; i++) {
            if (strcasecmp(args[0], commandList[i].name) == 0) {
                desc = commandList[i].description;
                break;
            }
        }
        if (desc) {
            Serial.printf("Command: %s\n", args[0]);
            Serial.printf("Description: %s\n", desc);
        } else {
            Serial.printf("Unknown command: %s\n", args[0]);
        }
    } else {
        // Show all commands
        Serial.println("ESP32-OS Command Reference:");
        Serial.println("===========================");
        Serial.println("Available commands:");
        for (int i = 0; i < commandCount; i++)
        {
            Serial.printf("  %-12s - %s\n", commandList[i].name, commandList[i].description);
        }
        Serial.println("\nUse 'help <command>' for detailed information about a specific command.");
    }
}

void Commands::cmd_ls(char args[][32], int argCount) {
    if (fs_) {
        fs_->listFiles();
    } else {
        Serial.println("File system not available");
    }
}

void Commands::cmd_ps(char args[][32], int argCount) {
    if (kernel && kernel->getScheduler()) {
        kernel->getScheduler()->listTasks();
    } else {
        Serial.println("Scheduler not available");
    }
}

void Commands::cmd_free(char args[][32], int argCount) {
    if (kernel) {
        uint32_t freeMem = kernel->getFreeMemory();
        uint32_t minFreeMem = kernel->getMinFreeMemory();
        
        char freeStr[32], minFreeStr[32];
        formatBytes(freeMem, freeStr, sizeof(freeStr));
        formatBytes(minFreeMem, minFreeStr, sizeof(minFreeStr));
        
        Serial.println("Memory Usage:");
        Serial.printf("Free Memory:     %s\n", freeStr);
        Serial.printf("Min Free Memory: %s\n", minFreeStr);
        
        if (kernel->getMemoryManager()) {
            kernel->getMemoryManager()->printStatistics();
        }
    } else {
        Serial.println("Kernel not available");
    }
}

void Commands::cmd_reboot(char args[][32], int argCount) {
    Serial.println("Rebooting system...");
    delay(1000);
    if (kernel) {
        kernel->reboot();
    } else {
        ESP.restart();
    }
}

void Commands::cmd_info(char args[][32], int argCount) {
    Serial.println("System Information:");
    Serial.println("==================");
    
    if (kernel) {
        Serial.printf("OS Version:      %s\n", kernel->getVersion());
    }
    Serial.printf("Build Date:      %s %s\n", OS_BUILD_DATE, OS_BUILD_TIME);
    Serial.printf("Chip Model:      %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision:   %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency:   %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size:      %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Flash Speed:     %d Hz\n", ESP.getFlashChipSpeed());
    
    if (kernel) {
        char uptimeStr[32];
        formatTime(kernel->getUptime(), uptimeStr, sizeof(uptimeStr));
        Serial.printf("Uptime:          %s\n", uptimeStr);
        Serial.printf("Total Tasks:     %d\n", kernel->getTotalTasks());
    }
}

void Commands::cmd_uptime(char args[][32], int argCount) {
    if (kernel) {
        char uptimeStr[64];
        formatTime(kernel->getUptime(), uptimeStr, sizeof(uptimeStr));
        Serial.printf("System uptime: %s\n", uptimeStr);
    } else {
        Serial.println("Kernel not available");
    }
}

void Commands::cmd_tasks(char args[][32], int argCount) {
    if (kernel && kernel->getScheduler()) {
        kernel->getScheduler()->printTaskStats();
        Serial.println();
        kernel->getScheduler()->listTasks();
    } else {
        Serial.println("Scheduler not available");
    }
}

void Commands::cmd_mem(char args[][32], int argCount) {
    if (kernel && kernel->getMemoryManager()) {
        kernel->getMemoryManager()->printMemoryMap();
        Serial.println();
        kernel->getMemoryManager()->printStatistics();
    } else {
        Serial.println("Memory manager not available");
    }
}

void Commands::cmd_clear(char args[][32], int argCount) {
    Serial.print("\033[2J\033[H"); // ANSI clear screen
}

void Commands::cmd_echo(char args[][32], int argCount) {
    for (int i = 0; i < argCount; i++) {
        if (i > 0) Serial.print(" ");
        Serial.print(args[i]);
    }
    Serial.println();
}

void Commands::cmd_sleep(char args[][32], int argCount) {
    if (argCount < 1) {
        printUsage("sleep", "sleep <seconds>");
        return;
    }
    
    int seconds;
    if (!parseInteger(args[0], &seconds) || seconds < 0) {
        Serial.println("Invalid sleep duration");
        return;
    }
    
    Serial.printf("Sleeping for %d seconds...\n", seconds);
    delay(seconds * 1000);
    Serial.println("Sleep completed");
}

void Commands::cmd_led(char args[][32], int argCount) {
    if (argCount < 1) {
        printUsage("led", "led <on|off|toggle>");
        return;
    }
    
    if (hal) {
        if (strcasecmp(args[0], "on") == 0) {
            hal->setLED(true);
            Serial.println("LED turned on");
        } else if (strcasecmp(args[0], "off") == 0) {
            hal->setLED(false);
            Serial.println("LED turned off");
        } else if (strcasecmp(args[0], "toggle") == 0) {
            hal->toggleLED();
            Serial.println("LED toggled");
        } else {
            Serial.println("Invalid LED command. Use: on, off, or toggle");
        }
    } else {
        Serial.println("HAL not available");
    }
}

void Commands::cmd_wifi(char args[][32], int argCount) {
    if (argCount < 1) {
        printUsage("wifi", "wifi <status|scan|connect|disconnect>");
        return;
    }
    
    if (strcasecmp(args[0], "status") == 0) {
        Serial.printf("WiFi Status: %s\n", 
                     WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        }
    } else if (strcasecmp(args[0], "scan") == 0) {
        Serial.println("Scanning for WiFi networks...");
        int networks = WiFi.scanNetworks();
        if (networks == 0) {
            Serial.println("No networks found");
        } else {
            Serial.printf("Found %d networks:\n", networks);
            for (int i = 0; i < networks; i++) {
                Serial.printf("%2d: %-32s (%d dBm) %s\n", 
                             i + 1,
                             WiFi.SSID(i).c_str(),
                             WiFi.RSSI(i),
                             WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Encrypted");
            }
        }
    } else if (strcasecmp(args[0], "disconnect") == 0) {
        WiFi.disconnect();
        Serial.println("WiFi disconnected");
    } else {
        Serial.println("Invalid WiFi command");
    }
}

// Utility functions

void Commands::printUsage(const char* command, const char* usage) {
    Serial.printf("Usage: %s\n", usage);
}

bool Commands::parseInteger(const char* str, int* value) {
    if (!str || !value) return false;
    
    char* endPtr;
    long result = strtol(str, &endPtr, 10);
    
    if (*endPtr != '\0') return false; // Not a valid integer
    if (result < INT_MIN || result > INT_MAX) return false; // Out of range
    
    *value = (int)result;
    return true;
}

void Commands::formatTime(unsigned long seconds, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize < 32) return;
    
    unsigned long days = seconds / 86400;
    seconds %= 86400;
    unsigned long hours = seconds / 3600;
    seconds %= 3600;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    
    if (days > 0) {
        snprintf(buffer, bufferSize, "%lu days, %02lu:%02lu:%02lu", 
                days, hours, minutes, seconds);
    } else {
        snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu", 
                hours, minutes, seconds);
    }
}

void Commands::formatBytes(uint32_t bytes, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize < 16) return;
    
    if (bytes >= 1048576) { // >= 1MB
        snprintf(buffer, bufferSize, "%.2f MB", bytes / 1048576.0);
    } else if (bytes >= 1024) { // >= 1KB
        snprintf(buffer, bufferSize, "%.2f KB", bytes / 1024.0);
    } else {
        snprintf(buffer, bufferSize, "%u bytes", bytes);
    }
}
