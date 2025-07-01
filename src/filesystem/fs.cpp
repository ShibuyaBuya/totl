/*
 * ESP32-OS File System Implementation
 */

#include "fs.h"

FileSystem::FileSystem() : initialized(false), mounted(false), 
                          totalBytes(0), usedBytes(0) {
}

FileSystem::~FileSystem() {
    shutdown();
}

bool FileSystem::init() {
    if (initialized) {
        return true;
    }
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) { // Format if mount fails
        Serial.println("FileSystem: Failed to mount SPIFFS");
        return false;
    }
    
    mounted = true;
    updateStatistics();
    
    initialized = true;
    Serial.println("FileSystem: SPIFFS mounted successfully");
    Serial.printf("FileSystem: Total: %zu bytes, Used: %zu bytes, Free: %zu bytes\n",
                  totalBytes, usedBytes, getFreeBytes());
    
    return true;
}

void FileSystem::shutdown() {
    if (mounted) {
        SPIFFS.end();
        mounted = false;
    }
    initialized = false;
}

bool FileSystem::createFile(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        return false;
    }
    
    file.close();
    updateStatistics();
    return true;
}

bool FileSystem::deleteFile(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    bool result = SPIFFS.remove(path);
    if (result) {
        updateStatistics();
    }
    return result;
}

bool FileSystem::fileExists(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    return SPIFFS.exists(path);
}

bool FileSystem::renameFile(const char* oldPath, const char* newPath) {
    if (!initialized || !oldPath || !newPath) {
        return false;
    }
    
    return SPIFFS.rename(oldPath, newPath);
}

bool FileSystem::createDirectory(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    // SPIFFS doesn't have real directories, but we can simulate by creating
    // a placeholder file
    String dirMarker = String(path) + "/.dir";
    return createFile(dirMarker.c_str());
}

bool FileSystem::deleteDirectory(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    // Delete all files in the directory
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory()) {
        return false;
    }
    
    File file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        if (fileName.startsWith(path)) {
            file.close();
            SPIFFS.remove(fileName.c_str());
        } else {
            file.close();
        }
        file = root.openNextFile();
    }
    
    root.close();
    updateStatistics();
    return true;
}

bool FileSystem::directoryExists(const char* path) {
    if (!initialized || !path) {
        return false;
    }
    
    String dirMarker = String(path) + "/.dir";
    return fileExists(dirMarker.c_str());
}

File FileSystem::openFile(const char* path, const char* mode) {
    if (!initialized || !path || !mode) {
        return File();
    }
    
    return SPIFFS.open(path, mode);
}

bool FileSystem::writeFile(const char* path, const char* data) {
    if (!initialized || !path || !data) {
        return false;
    }
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        return false;
    }
    
    size_t bytesWritten = file.print(data);
    file.close();
    
    updateStatistics();
    return bytesWritten > 0;
}

bool FileSystem::writeFile(const char* path, const uint8_t* data, size_t length) {
    if (!initialized || !path || !data || length == 0) {
        return false;
    }
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        return false;
    }
    
    size_t bytesWritten = file.write(data, length);
    file.close();
    
    updateStatistics();
    return bytesWritten == length;
}

bool FileSystem::readFile(const char* path, String& content) {
    if (!initialized || !path) {
        return false;
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        return false;
    }
    
    content = file.readString();
    file.close();
    
    return true;
}

bool FileSystem::appendFile(const char* path, const char* data) {
    if (!initialized || !path || !data) {
        return false;
    }
    
    File file = SPIFFS.open(path, "a");
    if (!file) {
        return false;
    }
    
    size_t bytesWritten = file.print(data);
    file.close();
    
    updateStatistics();
    return bytesWritten > 0;
}

bool FileSystem::getFileInfo(const char* path, FileInfo& info) {
    if (!initialized || !path) {
        return false;
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        return false;
    }
    
    strncpy(info.name, path, sizeof(info.name) - 1);
    info.name[sizeof(info.name) - 1] = '\0';
    info.size = file.size();
    info.isDirectory = file.isDirectory();
    info.lastModified = file.getLastWrite();
    
    file.close();
    return true;
}

size_t FileSystem::getFileSize(const char* path) {
    if (!initialized || !path) {
        return 0;
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        return 0;
    }
    
    size_t size = file.size();
    file.close();
    return size;
}

void FileSystem::listFiles(const char* path) {
    if (!initialized) {
        Serial.println("File system not initialized");
        return;
    }
    
    File root = SPIFFS.open(path ? path : "/");
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return;
    }
    
    Serial.printf("Directory listing for: %s\n", path ? path : "/");
    Serial.println("Name                          Size");
    Serial.println("------------------------------------");
    
    File file = root.openNextFile();
    while (file) {
        Serial.printf("%-28s %8zu\n", file.name(), file.size());
        file.close();
        file = root.openNextFile();
    }
    
    root.close();
}

void FileSystem::listFilesDetailed(const char* path) {
    if (!initialized) {
        Serial.println("File system not initialized");
        return;
    }
    
    File root = SPIFFS.open(path ? path : "/");
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return;
    }
    
    Serial.printf("Detailed directory listing for: %s\n", path ? path : "/");
    Serial.println("Name                          Size      Modified");
    Serial.println("------------------------------------------------");
    
    File file = root.openNextFile();
    while (file) {
        time_t lastWrite = file.getLastWrite();
        struct tm* timeinfo = localtime(&lastWrite);
        
        Serial.printf("%-28s %8zu  %04d-%02d-%02d %02d:%02d:%02d\n",
                     file.name(),
                     file.size(),
                     timeinfo->tm_year + 1900,
                     timeinfo->tm_mon + 1,
                     timeinfo->tm_mday,
                     timeinfo->tm_hour,
                     timeinfo->tm_min,
                     timeinfo->tm_sec);
        
        file.close();
        file = root.openNextFile();
    }
    
    root.close();
}

bool FileSystem::format() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("FileSystem: Formatting SPIFFS...");
    SPIFFS.end();
    mounted = false;
    
    if (!SPIFFS.format()) {
        Serial.println("FileSystem: Format failed");
        return false;
    }
    
    if (!SPIFFS.begin()) {
        Serial.println("FileSystem: Failed to remount after format");
        return false;
    }
    
    mounted = true;
    updateStatistics();
    
    Serial.println("FileSystem: Format completed successfully");
    return true;
}

bool FileSystem::check() {
    if (!initialized) {
        return false;
    }
    
    // Basic file system health check
    updateStatistics();
    
    // Check if we can create and delete a test file
    const char* testFile = "/test_fs_health";
    if (!writeFile(testFile, "test")) {
        return false;
    }
    
    if (!deleteFile(testFile)) {
        return false;
    }
    
    return true;
}

void FileSystem::updateStatistics() {
    if (!mounted) {
        totalBytes = usedBytes = 0;
        return;
    }
    
    totalBytes = SPIFFS.totalBytes();
    usedBytes = SPIFFS.usedBytes();
}

void FileSystem::printStatistics() {
    updateStatistics();
    
    Serial.println("File System Statistics:");
    Serial.println("======================");
    Serial.printf("Total Space:     %zu bytes (%.2f KB)\n", 
                  totalBytes, totalBytes / 1024.0);
    Serial.printf("Used Space:      %zu bytes (%.2f KB)\n", 
                  usedBytes, usedBytes / 1024.0);
    Serial.printf("Free Space:      %zu bytes (%.2f KB)\n", 
                  getFreeBytes(), getFreeBytes() / 1024.0);
    Serial.printf("Usage:           %.1f%%\n", getUsagePercent());
}

bool FileSystem::isValidPath(const char* path) {
    if (!path || strlen(path) == 0 || strlen(path) >= FS_MAX_PATH_LENGTH) {
        return false;
    }
    
    // Path must start with '/'
    if (path[0] != '/') {
        return false;
    }
    
    return true;
}

void FileSystem::formatPath(const char* path, char* formattedPath, size_t maxLen) {
    if (!path || !formattedPath || maxLen == 0) {
        return;
    }
    
    // Ensure path starts with '/'
    if (path[0] != '/') {
        snprintf(formattedPath, maxLen, "/%s", path);
    } else {
        strncpy(formattedPath, path, maxLen - 1);
        formattedPath[maxLen - 1] = '\0';
    }
}

const char* FileSystem::getFileExtension(const char* filename) {
    if (!filename) return "";
    
    const char* ext = strrchr(filename, '.');
    return ext ? ext + 1 : "";
}

void FileSystem::getBaseName(const char* path, char* basename, size_t maxLen) {
    if (!path || !basename || maxLen == 0) return;
    
    const char* lastSlash = strrchr(path, '/');
    const char* name = lastSlash ? lastSlash + 1 : path;
    
    strncpy(basename, name, maxLen - 1);
    basename[maxLen - 1] = '\0';
}

void FileSystem::getDirName(const char* path, char* dirname, size_t maxLen) {
    if (!path || !dirname || maxLen == 0) return;
    
    const char* lastSlash = strrchr(path, '/');
    if (!lastSlash) {
        strcpy(dirname, "/");
        return;
    }
    
    size_t len = lastSlash - path;
    if (len == 0) len = 1; // Root directory
    
    if (len >= maxLen) len = maxLen - 1;
    
    strncpy(dirname, path, len);
    dirname[len] = '\0';
}
