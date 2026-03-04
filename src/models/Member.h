// Member header

#ifndef LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H

#include <string>
#include <vector>
#include <utility>

class Member {
private:
    std::string memberID;                       // 会员 ID
    std::string name;                           // 会员姓名
    std::string phoneNumber;                    // 会员手机号码
    std::vector<std::string> preference;        // 会员偏好类别
    std::string registrationDate;               // 会员注册日 (YYYY-MM-DD)
    std::string expiryDate;                     // 会员过期日 (YYYY-MM-DD)
    int maxBooksAllowed{};                      // 会员最多可借阅的书籍数量(默认 2 本)
    bool isAdmin{};                             // true = 管理员, false = 会员
    std::string passwordHash;                   // 用于验证

public:
    // 构造函数
    Member(std::string memberID, std::string name, std::string phoneNumber,
        std::vector<std::string> preference, std::string registrationDate, std::string expiryDate,
        int maxBooksAllowed = 2, bool isAdmin = false, std::string passwordHash = ""):
        memberID(std::move(memberID)), name(std::move(name)), phoneNumber(std::move(phoneNumber)),
        preference(std::move(preference)), registrationDate(std::move(registrationDate)),
        expiryDate(std::move(expiryDate)), maxBooksAllowed(maxBooksAllowed),
        isAdmin(isAdmin), passwordHash(std::move(passwordHash)) {}
    Member() = default;

    // 获取器和设置器
    std::string getMemberID() const;
    std::string getName() const;
    std::string getPhoneNumber() const;
    std::vector<std::string> getPreference() const;
    std::string getRegistrationDate() const;
    std::string getExpiryDate() const;
    std::string getPasswordHash() const;
    bool isExpired() const;
    bool getAdmin() const;
    int getMaxBooksAllowed() const;
    bool authenticate(const std::string& password) const;

    // CSV 函数
    std::string toCSV() const;
    static Member fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H
