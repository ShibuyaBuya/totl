/*
 * ESP32-OS Shell Implementation
 */

#include "shell.h"
#include <stdarg.h>

Shell::Shell() : bufferPos(0), echoEnabled(true), commands(nullptr) {
    memset(inputBuffer, 0, SHELL_BUFFER_SIZE);
}

Shell::~Shell() {
    shutdown();
}

bool Shell::init() {
    // Initialize command processor
    commands = new Commands();
    if (!commands || !commands->init()) {
        Serial.println("Shell: Failed to initialize command processor");
        return false;
    }
    
    clearBuffer();
    printBanner();
    printPrompt();
    
    Serial.println("Shell: Command interface initialized");
    return true;
}

void Shell::shutdown() {
    if (commands) {
        delete commands;
        commands = nullptr;
    }
}

void Shell::processInput() {
    while (Serial.available()) {
        char c = Serial.read();
        
        // Handle special characters
        if (c == '\r' || c == '\n') {
            handleEnter();
        } else if (c == 127 || c == 8) { // DEL or Backspace
            handleBackspace();
        } else if (c >= 32 && c <= 126) { // Printable characters
            handleChar(c);
        }
        // Ignore other control characters
    }
}

void Shell::handleChar(char c) {
    if (bufferPos < SHELL_BUFFER_SIZE - 1) {
        inputBuffer[bufferPos++] = c;
        inputBuffer[bufferPos] = '\0';
        
        if (echoEnabled) {
            Serial.print(c);
        }
    } else {
        // Buffer full - beep or ignore
        if (echoEnabled) {
            Serial.print('\a'); // ASCII bell character
        }
    }
}

void Shell::handleBackspace() {
    if (bufferPos > 0) {
        bufferPos--;
        inputBuffer[bufferPos] = '\0';
        
        if (echoEnabled) {
            Serial.print("\b \b"); // Backspace, space, backspace
        }
    }
}

void Shell::handleEnter() {
    if (echoEnabled) {
        Serial.println(); // New line
    }
    
    // Process command if buffer not empty
    if (bufferPos > 0) {
        processCommand(inputBuffer);
    }
    
    clearBuffer();
    printPrompt();
}

void Shell::processCommand(const char* cmdLine) {
    if (!cmdLine || !commands) {
        return;
    }
    
    // Skip leading whitespace
    while (*cmdLine == ' ' || *cmdLine == '\t') {
        cmdLine++;
    }
    
    // Ignore empty commands
    if (*cmdLine == '\0') {
        return;
    }
    
    // Parse command and arguments
    char cmd[32];
    char args[SHELL_MAX_ARGS][32];
    int argCount = 0;
    
    parseCommand(cmdLine, cmd, args, &argCount);
    
    // Execute command
    if (!commands->execute(cmd, args, argCount)) {
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        Serial.println("Type 'help' for available commands");
    }
}

void Shell::parseCommand(const char* cmdLine, char* cmd, char args[][32], int* argCount) {
    *argCount = 0;
    memset(cmd, 0, 32);
    
    // Parse command
    const char* ptr = cmdLine;
    int i = 0;
    
    // Extract command
    while (*ptr && *ptr != ' ' && *ptr != '\t' && i < 31) {
        cmd[i++] = *ptr++;
    }
    cmd[i] = '\0';
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    
    // Extract arguments
    while (*ptr && *argCount < SHELL_MAX_ARGS) {
        i = 0;
        memset(args[*argCount], 0, 32);
        
        // Handle quoted arguments
        bool quoted = false;
        if (*ptr == '"') {
            quoted = true;
            ptr++;
        }
        
        // Extract argument
        while (*ptr && i < 31) {
            if (quoted) {
                if (*ptr == '"') {
                    ptr++;
                    break;
                }
            } else {
                if (*ptr == ' ' || *ptr == '\t') {
                    break;
                }
            }
            args[*argCount][i++] = *ptr++;
        }
        
        args[*argCount][i] = '\0';
        (*argCount)++;
        
        // Skip trailing whitespace
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }
    }
}

void Shell::printPrompt() {
    Serial.print(SHELL_PROMPT);
}

void Shell::clearBuffer() {
    memset(inputBuffer, 0, SHELL_BUFFER_SIZE);
    bufferPos = 0;
}

void Shell::println(const char* text) {
    Serial.println(text);
}

void Shell::print(const char* text) {
    Serial.print(text);
}

void Shell::printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
}

bool Shell::executeCommand(const char* cmdLine) {
    if (!cmdLine || !commands) {
        return false;
    }
    
    processCommand(cmdLine);
    return true;
}

void Shell::clearScreen() {
    Serial.print("\033[2J\033[H"); // ANSI escape sequence to clear screen
}

void Shell::printBanner() {
    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32-OS Shell v1.0");
    Serial.println("  Custom Operating System for ESP32");
    Serial.println("========================================");
    Serial.println();
}
