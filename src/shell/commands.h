/*
 * ESP32-OS Shell Commands Header
 * Built-in command implementations
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>
#include "../config/config.h"

// Forward declarations
class Kernel;
class Shell;

struct Command {
    const char* name;
    const char* description;
    void (*handler)(char args[][32], int argCount);
};

class Commands {
private:
    static const Command commandList[];
    static const int commandCount;
    
    // Command handlers
    static void cmd_help(char args[][32], int argCount);
    static void cmd_ls(char args[][32], int argCount);
    static void cmd_ps(char args[][32], int argCount);
    static void cmd_free(char args[][32], int argCount);
    static void cmd_reboot(char args[][32], int argCount);
    static void cmd_info(char args[][32], int argCount);
    static void cmd_uptime(char args[][32], int argCount);
    static void cmd_tasks(char args[][32], int argCount);
    static void cmd_mem(char args[][32], int argCount);
    static void cmd_clear(char args[][32], int argCount);
    static void cmd_echo(char args[][32], int argCount);
    static void cmd_sleep(char args[][32], int argCount);
    static void cmd_led(char args[][32], int argCount);
    static void cmd_wifi(char args[][32], int argCount);
    
    // Utility functions
    static void printUsage(const char* command, const char* usage);
    static bool parseInteger(const char* str, int* value);
    static void formatTime(unsigned long seconds, char* buffer, size_t bufferSize);
    static void formatBytes(uint32_t bytes, char* buffer, size_t bufferSize);
    
public:
    Commands();
    ~Commands();
    
    bool init();
    void shutdown();
    
    // Command execution
    bool execute(const char* cmd, char args[][32], int argCount);
    
    // Command management
    void listCommands();
    bool isValidCommand(const char* cmd);
    const char* getCommandDescription(const char* cmd);
};

#endif // COMMANDS_H
