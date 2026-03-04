// BackupManager.cpp 实现

#include "BackupManager.h"
#include "../config/Config.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace {
std::string joinPath(const std::string& dir, const std::string& sub) {
    if (dir.empty()) return sub;
    char last = dir[dir.size() - 1];
    if (last == '/' || last == '\\') {
        return dir + sub;
    }
    return dir + "/" + sub;
}

std::string getCwd() {
#ifdef _WIN32
    char buf[MAX_PATH];
    if (_getcwd(buf, MAX_PATH) == nullptr) {
        return ".";
    }
    return std::string(buf);
#else
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == nullptr) {
        return ".";
    }
    return std::string(buf);
#endif
}

std::string findDataRoot() {
    FileHandler fh;
    std::string dir = getCwd();
    for (int i = 0; i < 5; ++i) {
        std::string books = joinPath(dir, Config::BOOKS_FILE);
        std::string members = joinPath(dir, Config::MEMBERS_FILE);
        std::string trans = joinPath(dir, Config::TRANSACTIONS_FILE);
        std::string reserv = joinPath(dir, Config::RESERVATIONS_FILE);
        std::string settings = joinPath(dir, Config::SETTINGS_FILE);

        if (fh.isFileExist(books) &&
            fh.isFileExist(members) &&
            fh.isFileExist(trans) &&
            fh.isFileExist(reserv) &&
            fh.isFileExist(settings)) {
            return dir;
        }
        // 上移一层目录
        std::string::size_type pos = dir.find_last_of("/\\");
        if (pos == std::string::npos) {
            break;
        }
        dir = dir.substr(0, pos);
    }
    return "";
}
}

BackupInfo::BackupInfo()
    : backupID(""),
      backupTime(""),
      description(""),
      isValid(false),
      backupPath("") {}

std::string BackupInfo::toString() const{
    std::ostringstream oss;
    oss << "备份 ID: " << backupID << "\n"
        << "备份时间: " << backupTime << "\n"
        << "描述: " << description << "\n"
        << "状态: " << (isValid ? "有效" : "无效") << "\n"
        << "备份路径: " << backupPath << "\n";

    return oss.str();
}

std::string BackupInfo::toCSV() const {
    std::ostringstream oss;
    oss << backupID << ","
        << backupTime << ","
        << description << ","
        << (isValid ? "1" : "0") << ","
        << backupPath;

    return oss.str();
}

BackupInfo BackupInfo::fromCSV(const std::string& csvLine) {
    BackupInfo backupInfo;
    std::istringstream iss(csvLine);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(iss, field, ',')) {
        fields.push_back(field);
    }

    if (fields.size() == 5) {
        backupInfo.backupID = fields[0];
        backupInfo.backupTime = fields[1];
        backupInfo.description = fields[2];
        backupInfo.isValid = (fields[3] == "1");
        backupInfo.backupPath = fields[4];
    }

    return backupInfo;
}

BackupManager::BackupManager() {
    initPaths();
    createBackupDirectory();
    loadBackupManifest();
}

BackupManager::~BackupManager() {
    saveBackupManifest();
}


// 创建一个备份根目录 (若不存在)
bool BackupManager::initPaths() {
    baseDir = findDataRoot();
    if (baseDir.empty()) {
        return false;
    }
    backupRootDir = joinPath(baseDir, BACKUP_ROOT_DIR);
    return true;
}

bool BackupManager::createBackupDirectory() {
    try {
        FileHandler fileHandler;
        if (backupRootDir.empty()) {
            return false;
        }
        if (!fileHandler.isFileExist(backupRootDir)) {
            fileHandler.createDirectory(backupRootDir);
            std::cout << "备份路径已创建: " << backupRootDir << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建备份目录错误:  " + std::string(e.what());
        return false;
    }
}

// 生成备份 ID (基于时间戳)
std::string BackupManager::generateBackupID() {
    time_t now = DateUtils::getCurrentTimestamp();
    std::tm *tm = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm);
    return {buffer};
}

// 复制单个文件
bool BackupManager::copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        std::ifstream ifs(sourcePath, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "打开源文件错误: " << sourcePath << std::endl;
            return false;
        }
        std::ofstream ofs(destinationPath, std::ios::binary);
        if (!ofs.is_open()) {
            ifs.close();
            std::cerr << "打开目标文件错误: " << destinationPath << std::endl;
            return false;
        }

        // 区块 I/O
        const int BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];

        while (ifs.read(buffer, BUFFER_SIZE) && ifs.gcount() > 0) {
            ofs.write(buffer, ifs.gcount());
        }

        ofs.close();
        ifs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "复制文件错误: " << destinationPath << std::endl;
        return false;
    }
}

// 从元文件加载备份
bool BackupManager::loadBackupManifest() {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string manifestPath = joinPath(backupRootDir, BACKUP_MANIFEST_FILE);
    backupInfoList.clear();         // 清除内存中的备份信息列表
    if (!fileHandler.isFileExist(manifestPath)) {
        return true;                // 首次运行时, 这是正常状况
    }

    try {
        std::ifstream ifs(manifestPath);
        if (!ifs.is_open()) {
            std::cerr << "打开备份元文件错误: " << manifestPath << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) {
                continue;
            }

            BackupInfo backupInfo;
            backupInfo = backupInfo.fromCSV(line);
            backupInfo.isValid = isValidBackup(backupInfo.backupID);
            backupInfoList.emplace_back(backupInfo);
        }
        ifs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载备份元文件错误: " << e.what() << std::endl;
        return false;
    }
}

// 从元文件中保存备份
bool BackupManager::saveBackupManifest() {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string manifestPath = joinPath(backupRootDir, BACKUP_MANIFEST_FILE);
    try {
        std::ofstream ofs(manifestPath);
        if (!ofs.is_open()) {
            std::cerr << "打开备份元文件错误: " << manifestPath << std::endl;
            return false;
        }

        for (const auto& backupInfo : backupInfoList) {
            std::string csvLine = backupInfo.toCSV();
            ofs << csvLine << "\n";
        }
        ofs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存备份元文件错误: " << e.what() << std::endl;
        return false;
    }
}

// 检查备份完整性
bool BackupManager::isValidBackup(const std::string& backupID) {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string backupPath = joinPath(backupRootDir, backupID);
    if (!fileHandler.isFileExist(backupPath)) {
        std::cerr << "打开备份路径错误: " << backupPath << std::endl;
        return false;
    }

    for (const auto& datafile : DATA_FILES) {
        std::string datafilePath = backupPath + "/" + datafile;
        if (!fileHandler.isFileExist(datafilePath)) {
            std::cerr << "打开数据文件路径错误: " << datafilePath << std::endl;
            return false;
        }
    }
    return true;
}

// 完全数据备份
bool BackupManager::backupData(const std::string& backupDescription) {
    if (backupRootDir.empty() && !initPaths()) {
        std::cerr << "备份失败: 数据路径未找到" << std::endl;
        return false;
    }
    std::string backupID = generateBackupID();
    std::string backupPath = joinPath(backupRootDir, backupID);
    try {
        FileHandler fileHandler;
        if (fileHandler.isFileExist(backupPath)) {
            std::cerr << "备份已存在: " << backupPath << std::endl;
            return false;
        }

        if (!fileHandler.createDirectory(backupPath)) {
            std::cerr << "创建备份目录失败: " << backupPath << std::endl;
            return false;
        }
        bool allCopied = true;
        for (const auto& datafile : DATA_FILES) {
            std::string sourcePath = joinPath(baseDir, Config::DATA_DIR + datafile);
            std::string destinationPath = joinPath(backupPath, datafile);
            if (!fileHandler.isFileExist(sourcePath)) {
                std::cerr << "打开源文件失败: " << sourcePath << std::endl;
                allCopied = false;
                continue;
            }
            if (!copyFile(sourcePath, destinationPath)) {
                std::cerr << "复制文件失败: " << sourcePath << std::endl;
                allCopied = false;
            }
        }
        if (!allCopied) {
            std::cerr << "备份失败: 一个或多个源文件缺失或复制时失败" << std::endl;
            return false;
        }

        BackupInfo backupInfo;
        backupInfo.backupID = backupID;
        backupInfo.backupTime = DateUtils::getCurrentDateTime();
        backupInfo.description = backupDescription;
        backupInfo.isValid = true;
        backupInfo.backupPath = backupPath;

        backupInfoList.emplace_back(backupInfo);

        if (!saveBackupManifest()) {
            std::cerr << "保存备份源文件失败 " << backupPath << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载备份源文件失败: " << e.what() << std::endl;
        return false;
    }
}

// 备份恢复
bool BackupManager::restoreData(const std::string& backupId) {
    if (backupRootDir.empty() && !initPaths()) {
        std::cerr << "恢复失败: 数据路径未找到" << std::endl;
        return false;
    }
    std::string backupPath = joinPath(backupRootDir, backupId);
    FileHandler fileHandler;

    try {
        if (!fileHandler.isFileExist(backupPath)) {
            std::cerr << "打开备份路径目录失败: " << backupPath << std::endl;
            return false;
        }
        if (!isValidBackup(backupId)) {
            std::cerr << "备份 ID 无效: " << backupId << std::endl;
            return false;
        }

        for (const auto& datafile : DATA_FILES) {
            std::string sourcePath = joinPath(backupPath, datafile);
            std::string destinationPath = joinPath(baseDir, Config::DATA_DIR + datafile);
            if (!fileHandler.isFileExist(sourcePath)) {
                std::cerr << "打开源目录错误: " << sourcePath << std::endl;
                continue;
            }
            if (!copyFile(sourcePath, destinationPath)) {
                std::cerr << "复制备份元文件错误: " << sourcePath << std::endl;
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "恢复数据错误: " << e.what() << std::endl;
        return false;
    }
}

// 获取可用的备份 ID
std::vector<std::string> BackupManager::listBackupIds() {
    std::vector<std::string> backupIds;

    backupIds.reserve(backupInfoList.size());
    for (const auto& backupInfo : backupInfoList) {
        backupIds.emplace_back(backupInfo.backupID);
    }
    return backupIds;
}

// 获取所有备份详细信息
std::vector<BackupInfo> BackupManager::listBackups() {
    return backupInfoList;
}

// 备份 ID 搜索备份信息
BackupInfo BackupManager::getBackupInfo(const std::string& backupID) {
    for (const auto& backupInfo : backupInfoList) {
        if (backupInfo.backupID == backupID) {
            return backupInfo;
        }
    }
    return {};      // 若未找到
}

// 获取最新备份
BackupInfo BackupManager::getLatestBackup() {
    if (backupInfoList.empty()) {
        return {};
    }
    return backupInfoList.back();
}

// 检查是否有有效的更新
bool BackupManager::hasValidBackups() {
    for (const auto& backupInfo : backupInfoList) {
        if (backupInfo.isValid) {
            return true;
        }
    }
    return false;
}

// 自动清理过期备份 (保留前 N 个备份)
bool BackupManager::autoCleanOldBackups(int keepCount) {
    try {
        while (static_cast<int>(backupInfoList.size()) > keepCount) {
            backupInfoList.erase(backupInfoList.begin());
        }
        saveBackupManifest();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "自动清理旧备份失败: " << e.what() << std::endl;
        return false;
    }
}
