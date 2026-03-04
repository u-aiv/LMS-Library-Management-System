#ifndef LIBRARY_MANAGEMENT_SYSTEM_AUTH_H
#define LIBRARY_MANAGEMENT_SYSTEM_AUTH_H

#include <string>

namespace auth {
// 密码最大长度
constexpr size_t MAX_PASSWORD_LENGTH = 64;

// 哈希加密密码以安全存储
std::string hashPassword(const std::string& password);

// 验证密码
bool verifyPassword(const std::string& password, const std::string& storedHash);
}

#endif //LIBRARY_MANAGEMENT_SYSTEM_AUTH_H
