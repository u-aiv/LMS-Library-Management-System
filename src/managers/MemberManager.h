#ifndef LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H

#include "../models/Member.h"
#include "../utils/FileHandler.h"
#include <vector>
#include <string>

class MemberManager {
private:
    std::vector<Member> members;
    std::string filePath;
    FileHandler fileHandler;

    // 数据持久化
    // 助手: 从文件中加载会员数据
    void loadFromFile();

    // 助手: 将会员数据保存到文件
    void saveToFile();

    // 用于批量操作
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // 助手: 检查自动保存标志决定是否需要保存
    void saveIfNeeded();

public:
    // 构造函数
    explicit MemberManager(const std::string& filePath = "../data/members.csv");
    
    // CRUD操作 (增删查改)
    bool addMember(const Member& member);
    bool updateMember(const Member& member);
    bool deleteMember(const std::string& memberID);
    
    // 查找函数
    Member* findMemberByID(const std::string &memberID);
    std::vector<const Member*> findByName(const std::string& name, int matchMode = 0) const;
    std::vector<const Member*> findByPhoneNumber(const std::string& phoneNumber, int matchMode = 0) const;
    std::vector<const Member*> findByRegistrationDate(const std::string& registrationDate, int matchMode = 0) const;
    std::vector<const Member*> findByExpiryDate(const std::string& expiryDate, int matchMode = 0) const;
    std::vector<const Member*> findAdmins() const;

    // 验证
    Member* authenticateUser(const std::string& memberID, const std::string& password);

    // 获取器
    const std::vector<Member> &getAllMembers() const;
    int getTotalMembers() const;
    int getAdminCount() const;

    // 实用方法
    void reload();          // 重新加载文件
    void clearCache();      // 清除文件处理器缓存
    bool isMemberIDExists(const std::string& memberID) const;

    // 批量操作 (RAII)
    class BatchOperation {
    private:
        MemberManager* memberManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(MemberManager& mmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H
