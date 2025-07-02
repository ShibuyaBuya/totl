/*
 * ESP32-OS File System Header
 * Simple file system interface using SPIFFS
 */

#ifndef FS_T_H
#define FS_T_H

#include <Arduino.h>
#include "FS.h"
#include <SPIFFS.h>
#include "../config/config.h"
struct FileInfo {
    char name[FS_MAX_PATH_LENGTH];
    size_t size;
    bool isDirectory;
    uint32_t lastModified;
};

class FileSystem {
private:
    bool initialized;
    bool mounted;
    size_t totalBytes;
    size_t usedBytes;
    
    // File operations
    bool isValidPath(const char* path);
    void formatPath(const char* path, char* formattedPath, size_t maxLen);
    
public:
    FileSystem();
    ~FileSystem();
    
    bool init();
    void shutdown();
    
    // File operations
    bool createFile(const char* path);
    bool deleteFile(const char* path);
    bool fileExists(const char* path);
    bool renameFile(const char* oldPath, const char* newPath);
    
    // Directory operations
    bool createDirectory(const char* path);
    bool deleteDirectory(const char* path);
    bool directoryExists(const char* path);
    
    // File I/O
    File openFile(const char* path, const char* mode = "r");
    bool writeFile(const char* path, const char* data);
    bool writeFile(const char* path, const uint8_t* data, size_t length);
    bool readFile(const char* path, String& content);
    bool appendFile(const char* path, const char* data);
    
    // File information
    bool getFileInfo(const char* path, FileInfo& info);
    size_t getFileSize(const char* path);
    
    // Directory listing
    void listFiles(const char* path = "/");
    void listFilesDetailed(const char* path = "/");
    
    // File system information
    size_t getTotalBytes() const { return totalBytes; }
    size_t getUsedBytes() const { return usedBytes; }
    size_t getFreeBytes() const { return totalBytes - usedBytes; }
    float getUsagePercent() const { 
        return totalBytes > 0 ? (float)usedBytes / totalBytes * 100.0 : 0.0; 
    }
    
    // File system maintenance
    bool format();
    bool check();
    void updateStatistics();
    void printStatistics();
    
    // Utility functions
    static const char* getFileExtension(const char* filename);
    static void getBaseName(const char* path, char* basename, size_t maxLen);
    static void getDirName(const char* path, char* dirname, size_t maxLen);
};

#endif // FS_H
