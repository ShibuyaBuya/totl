/*
 * ESP32-OS Shell Header
 * Command line interface and input processing
 */

#ifndef SHELL_H
#define SHELL_H

#include <Arduino.h>
#include "../config/config.h"
#include "commands.h"

class Shell {
private:
    char inputBuffer[SHELL_BUFFER_SIZE];
    uint16_t bufferPos;
    bool echoEnabled;
    Commands* commands;
    
    void processCommand(const char* cmdLine);
    void parseCommand(const char* cmdLine, char* cmd, char args[][32], int* argCount);
    void printPrompt();
    void clearBuffer();
    
    // Input handling
    void handleBackspace();
    void handleEnter();
    void handleChar(char c);
    
public:
    Shell();
    ~Shell();
    
    bool init();
    void shutdown();
    
    // Main shell loop
    void processInput();
    
    // Shell control
    void setEcho(bool enabled) { echoEnabled = enabled; }
    bool getEcho() const { return echoEnabled; }
    
    // Output functions
    void println(const char* text);
    void print(const char* text);
    void printf(const char* format, ...);
    
    // Command execution
    bool executeCommand(const char* cmdLine);
    
    // Shell utilities
    void clearScreen();
    void printBanner();
};

// Task functions
void shellTask(void* parameter);

#endif // SHELL_H
