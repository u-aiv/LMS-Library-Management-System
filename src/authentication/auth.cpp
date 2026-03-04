// auth.cpp 实现
// Use PKCS5_PBKDF2

#include "auth.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <vector>

namespace auth {

std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    for (unsigned char byte : bytes){
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<unsigned char> hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        auto byte = static_cast<unsigned char>(
            std::strtol(byteStr.c_str(), nullptr, 16)
        );
        bytes.push_back(byte);
    }
    return bytes;
}

std::string hashPassword(const std::string& password) {
    if (password.length() > MAX_PASSWORD_LENGTH) {
        throw std::runtime_error("密码过长!(应少于 64 个字符)");
    }

    std::vector<unsigned char> salt(16);
    if (!RAND_bytes(salt.data(),
                    static_cast<int>(salt.size()))) {
        throw std::runtime_error("生成盐值失败!");
    }

    std::vector<unsigned char> hash(32);
    if (!PKCS5_PBKDF2_HMAC(password.data(),
                           static_cast<int>(password.length()),
                           salt.data(),
                           static_cast<int>(salt.size()),
                           100000,         // Number of iterations
                           EVP_sha256(),
                           static_cast<int>(hash.size()),
                           hash.data())) {
        throw std::runtime_error("密码哈希计算失败!");
    }

    return bytesToHex(salt) + bytesToHex(hash);
}

bool verifyPassword(const std::string& password, const std::string& storedHash) {
    if (password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    if (storedHash.length() != 96) {
        return false;
    }

    auto salt = hexToBytes(storedHash.substr(0, 32));

    std::vector<unsigned char> hash(32);
    if (!PKCS5_PBKDF2_HMAC(password.data(),
                           static_cast<int>(password.length()),
                           salt.data(),
                           static_cast<int>(salt.size()),
                           100000,         // 迭代次数
                           EVP_sha256(),
                           static_cast<int>(hash.size()),
                           hash.data())) {
        return false;
    }

    const auto computedHash = bytesToHex(hash);
    const auto expectedHash = storedHash.substr(32);

    return computedHash == expectedHash;
}

}
