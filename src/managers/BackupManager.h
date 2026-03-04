#ifndef LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H

#include "../utils/DateUtils.h"
#include "../utils/FileHandler.h"


// 记录备份元数据
class BackupInfo {
public:
    std::string backupID;               // 备份 ID (从时间戳生成)
    std::string backupTime;             // 备份创建时间 (YYYY-MM-DD HH:MM:SS 格式)
    std::string description;            // 描述或注释信息
    bool isValid;                       // 备份完整性
    std::string backupPath;             // 完整备份路径

    BackupInfo();
    ~BackupInfo() = default;
    std::string toString() const;
    std::string toCSV() const;
    BackupInfo fromCSV(const std::string& csvLine);
};

// 备份与恢复操作
class BackupManager {
private:
    const std::string BACKUP_ROOT_DIR = "data/backup";                  // 备份根目录
    const std::string BACKUP_MANIFEST_FILE = "backup_manifest.txt";     // 备份元数据文件名
    const std::vector<std::string> DATA_FILES =                         // 需备份的文件
        {
        "books.csv",
        "members.csv",
        "transactions.csv",
        "reservations.csv",
        "settings.csv"
    };
    FileHandler filehandler;
    std::vector<BackupInfo> backupInfoList;                             // 全部备份信息
    std::string baseDir;                                                // 包含 data/ 的项目根目录
    std::string backupRootDir;                                          // 绝对备份根路径

    bool createBackupDirectory();                                                           // 创建一个备份根目录 (若不存在)
    bool initPaths();                                                                       // 解析 baseDir / backupRootDir
    std::string generateBackupID();                                                         // 生成备份 ID
    bool copyFile(const std::string& sourcePath, const std::string& destinationPath);       // 复制单个文件
    bool loadBackupManifest();                                                              // 从元文件中加载备份
    bool saveBackupManifest();                                                              // 向元文件中保存备份
    bool isValidBackup(const std::string& backupID);                                        // 检查备份完整性

public:
    BackupManager();
    ~BackupManager();
    bool backupData(const std::string& backupDescription = "Manual Backup");            // 完全数据备份
    bool restoreData(const std::string& backupId);                                         // 恢复备份
    std::vector<std::string> listBackupIds();                                              // 获取可用备份 ID
    std::vector<BackupInfo> listBackups();                                                 // 获取所有备份详细信息
    BackupInfo getBackupInfo(const std::string& backupID);                                 // 备份ID 相关备份详细信息
    BackupInfo getLatestBackup();                                                          // 获取最新的备份
    bool hasValidBackups();                                                                // 检查是否有较新的备份
    bool autoCleanOldBackups(int keepCount = 5);                                           // 自动清理过期备份（保留最新的 N 个备份）
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H
