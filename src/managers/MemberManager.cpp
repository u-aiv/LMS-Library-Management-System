// MemberManager.cpp 实现

#include "MemberManager.h"
#include "../utils/FileHandler.h"
#include "../authentication/auth.h"
#include <algorithm>
#include <iostream>

// 构造函数
MemberManager::MemberManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "MemberID,Name,PhoneNumber,Preference,RegistrationDate,ExpiryDate,MaxBooksAllowed,IsAdmin,PasswordHash"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// 私有: 助手: 从文件加载成员数据
void MemberManager::loadFromFile() {
    members.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                members.push_back(Member::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load members file: " + std::string(e.what()));
    }
}

// 私有: 助手: 将成员数据保存到文件
void MemberManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("MemberID,Name,PhoneNumber,Preference,RegistrationDate,ExpiryDate,MaxBooksAllowed,isAdmin,PasswordHash");

    // 添加所有会员
    for (const auto& member : members) {
        lines.push_back(member.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save members file: " + std::string(e.what()));
    }
}

// 私有: 辅助: 检查自动保存标志决定是否需要保存
void MemberManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// 新增一位会员
bool MemberManager::addMember(const Member& member) {
    // Check if MemberID already exists
    if (isMemberIDExists(member.getMemberID())) {
        return false;
    }
    members.push_back(member);
    saveIfNeeded();
    return true;
}

// 以 MemberID 删除一位会员
bool MemberManager::deleteMember(const std::string& MemberID) {
    auto it = std::find_if(members.begin(), members.end(),
        [&](const Member& member) { return member.getMemberID() == MemberID; });

    if (it != members.end()) {
        members.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// 更新现有会员
bool MemberManager::updateMember(const Member& member) {
    Member *existingMember = findMemberByID(member.getMemberID());

    if (existingMember == nullptr) {
        return false;
    }
    *existingMember = member;
    saveIfNeeded();
    return true;
}

// 以 MemberID 查找一位会员
Member* MemberManager::findMemberByID(const std::string &MemberID) {
    for (auto& member : members) {
        if (member.getMemberID() == MemberID) {
            return &member;
        }
    }
    return nullptr;
}

// 会员查找模板
// matchMode = 0 --> 精确匹配 (区分大小写) (默认)
// matchMode = 1 --> 模糊匹配 (统一大小写)
template<typename Getter>
std::vector<const Member*> findByField(
    const std::vector<Member>& members,
    const std::string& key,
    Getter getter,
    int matchMode = 0
    ) {
        if (matchMode != 0 && matchMode != 1) {
            throw std::runtime_error("匹配码无效");
        }

        std::vector<const Member*> results;

        if (matchMode == 0) {
            for (const auto& member : members) {
                if ((member.*getter)() == key) {
                    results.push_back(&member);
                }
            }
        }
        else {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

            for (const auto& member : members) {
                std::string memberKey = (member.*getter)();
                std::transform(memberKey.begin(), memberKey.end(), memberKey.begin(), ::tolower);

                if (memberKey.find(lowerKey) != std::string::npos)
                    results.push_back(&member);
            }
        }
        return results;
}

// 以姓名查找一位会员
std::vector<const Member*> MemberManager::findByName(const std::string& name, int matchMode) const {
    return findByField(members, name, &Member::getName , matchMode);
}

// 以手机号码查找一位会员
std::vector<const Member*> MemberManager::findByPhoneNumber(const std::string& phoneNumber, int matchMode) const {
    return findByField(members, phoneNumber, &Member::getPhoneNumber, matchMode);
}

// 以注册日期查找一位会员
std::vector<const Member*> MemberManager::findByRegistrationDate(const std::string& registrationDate, int matchMode) const {
    return findByField(members, registrationDate, &Member::getRegistrationDate, matchMode);
}

// 以过期日期查找一位会员
std::vector<const Member*> MemberManager::findByExpiryDate(const std::string& expiryDate, int matchMode) const {
    return findByField(members, expiryDate, &Member::getExpiryDate, matchMode);
}

// 查找管理员
std::vector<const Member*> MemberManager::findAdmins() const {
    std::vector<const Member*> results;

    for (const auto& member : members) {
        if (member.getAdmin()) {
            results.push_back(&member);
        }
    }
    return results;
}

// 验证
Member* MemberManager::authenticateUser(const std::string& memberID, const std::string& password) {
    Member *toBeVerifiedMember = findMemberByID(memberID);

    if (toBeVerifiedMember == nullptr) {
        return nullptr;
    }

    if (auth::verifyPassword(password, toBeVerifiedMember->getPasswordHash())) {
        return toBeVerifiedMember;
    }
    return nullptr;
}

// 获取所有会员
const std::vector<Member>& MemberManager::getAllMembers() const {
    return members;
}

// 获取会员总数
int MemberManager::getTotalMembers() const {
    return static_cast<int>(members.size());
}

// 获取管理员总数
int MemberManager::getAdminCount() const {
    int count = 0;
    for (const auto& member : members) {
        if (member.getAdmin()) {
            count++;
        }
    }
    return count;
}

// 重新加载文件
void MemberManager::reload() {
    loadFromFile();
}

// 清除文件处理器缓存
void MemberManager::clearCache() {
    fileHandler.clearCache();
}

// 检查 MemberID 是否存在
bool MemberManager::isMemberIDExists(const std::string& memberID) const {
    return std::find_if(members.begin(), members.end(),
        [&](const Member& member) { return member.getMemberID() == memberID; }) != members.end();
}

// 批量操作 (RAII)
void MemberManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool MemberManager::isAutoSaveEnabled() const {
    return autoSave;
}

MemberManager::BatchOperation::BatchOperation(MemberManager& mmgr) :
                    memberManager(&mmgr), originalAutoSave(mmgr.autoSave), active(true) {
    mmgr.setAutoSave(false);
}

MemberManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    memberManager(other.memberManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

MemberManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        memberManager->saveToFile();
        memberManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "在批量操作中尝试保存会员时出错" << std::endl;
    }
}

MemberManager::BatchOperation MemberManager::beginBatch() {
    return BatchOperation(*this);
}
