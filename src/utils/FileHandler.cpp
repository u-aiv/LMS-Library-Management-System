// FileHandler.h 实现

#include "FileHandler.h"
#include <fstream>
#include <iostream>

#ifdef _WIN32
    #include <direct.h>         // _mkdir for Windows
    #include <sys/stat.h>
    #include <windows.h>
#else
    #include <sys/stat.h>       // mkdir/stat for Unix
    #include <unistd.h>
#endif


std::vector<std::string> FileHandler::readCSV(const std::string& filePath) {
    if (cache.find(filePath) != cache.end()) {
        return cache[filePath];
    }

    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        throw std::runtime_error ("打开文件错误: " + filePath);
    }
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }
    cache[filePath] = lines;

    return lines;
}

void FileHandler::writeCSV(const std::string& filePath, const std::vector<std::string>& lines) {
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        throw std::runtime_error ("打开文件错误: " + filePath);
    }
    for (const auto& line : lines) {
        ofs << line << "\n";
    }
    cache[filePath] = lines;
}

bool FileHandler::isFileExist(const std::string& filePath) {
#ifdef _WIN32
    struct _stat info;
    return _stat(filePath.c_str(), &info) == 0;
#else
    struct stat info;
    return stat(filePath.c_str(), &info) == 0;
#endif
}

void FileHandler::createFileIfNotExist(const std::string& filePath) {
    if (isFileExist(filePath)) return;
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        throw std::runtime_error ("打开文件错误: " + filePath);
    }
}

void FileHandler::clearCache() {
    cache.clear();
}

void FileHandler::clearCache(const std::string& filePath) {
    cache.erase(filePath);
}

bool FileHandler::createDirectory(const std::string& filePath) {
#ifdef _WIN32
    std::string path = filePath;
    // 将分隔符规范化为反斜杠
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '/') path[i] = '\\';
    }

    for (size_t i = 1; i < path.size(); i++) {
        if (path[i] == '\\') {
            path[i] = '\0';
            CreateDirectoryA(path.c_str(), NULL);
            path[i] = '\\';
        }
    }
    BOOL result = CreateDirectoryA(path.c_str(), NULL);
    return result || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    std::string path = filePath;
    for (size_t i = 1; i < path.size(); i++) {
        if (path[i] == '/') {
            path[i] = '\0';
            if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
                path[i] = '/';
                return false;
            }
            path[i] = '/';
        }
    }
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}
