// Config.cpp 实现

#include "Config.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

// 初始化文件路径
const std::string Config::DATA_DIR = "../data/";
const std::string Config::BOOKS_FILE = "../data/books.csv";
const std::string Config::MEMBERS_FILE = "../data/members.csv";
const std::string Config::TRANSACTIONS_FILE = "../data/transactions.csv";
const std::string Config::RESERVATIONS_FILE = "../data/reservations.csv";
const std::string Config::SETTINGS_FILE = "../data/settings.csv";
const std::string Config::REPORTS_DIR = "../reports/";

// 书目类型
const std::string Config::GENRES[5] = {
    "Fiction",
    "Non-Fiction",
    "Science",
    "History",
    "Biography"
};

// 文件设置
const double Config::FINE_PER_DAY = 2.0;
const double Config::MAX_FINE = 14.0;

// ID 前缀
const std::string Config::MEMBER_ID_PREFIX = "M";
const std::string Config::ADMIN_ID_PREFIX = "A";
const std::string Config::TRANSACTION_ID_PREFIX = "T";
const std::string Config::RESERVATION_ID_PREFIX = "R";

// 日期格式
const std::string Config::DATE_FORMAT = "YYYY-MM-DD";

// 默认密码
const std::string Config::DEFAULT_PASSWORD = "defaultpassword";

// 系统消息
const std::string Config::SUCCESS_MESSAGE_PREFIX = "[SUCCESS] ";
const std::string Config::ERROR_MESSAGE_PREFIX = "[ERROR] ";
const std::string Config::WARNING_MESSAGE_PREFIX = "[WARNING] ";
const std::string Config::INFO_MESSAGE_PREFIX = "[INFO] ";

// 获取 Config 的单例实例
Config& Config::getInstance() {
    static Config instance;
    return instance;
}

// 构造函数
Config::Config() {
    initializeDefaults();
}

void Config::initializeDefaults() {
    advancedUIMode = false;         // 首次启动设置为简单 UI 样式
    borrowPeriodDays = DEFAULT_BORROW_DAYS;
    finePerDay = FINE_PER_DAY;
    maxFine = MAX_FINE;
    defaultMaxBooks = DEFAULT_MAX_BOOKS;
}

void Config::loadSettings() {
    std::ifstream ifs(SETTINGS_FILE);
    if (!ifs.is_open()) {       // 文件不存在则使用初始路径
        return;
    }

    std::string line;
    if (std::getline(ifs, line)) {      // 跳过头行
        while (std::getline(ifs, line)) {
            parseSettingsLine(line);
        }
    }
    ifs.close();
}

void Config::parseSettingsLine(const std::string& line) {
    if (line.empty() || line[0] == '#') {       // 跳过空行和注释
        return;
    }

    std::stringstream ss(line);
    std::string key, value;

    if (std::getline(ss, key, CSV_DELIMITER) && std::getline(ss, value)) {
        // 修剪空白
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        // 将值存储在设置中
        settings[key] = value;

        // 应用特定设置
        if (key == "AdvancedUIMode") {
            advancedUIMode = (value == "1" || value == "true" || value == "True");
        } else if (key == "BorrowPeriodDays") {
            try {
                int days = std::stoi(value);
                if (days > 0 && days <= MAX_TOTAL_BORROW_DAYS) {
                    borrowPeriodDays = days;
                }
            } catch (...){
                // 无效值返回默认
            }
        } else if (key == "FinePerDay") {
            try {
                double fine = std::stod(value);
                if (fine > 0.0) {
                    finePerDay = fine;
                }
            } catch (...) {
                // 无效值返回默认
            }
        } else if (key == "MaxFine") {
            try {
                double fine = std::stod(value);
                if (fine > 0.0) {
                    maxFine = fine;
                }
            } catch (...) {
                // 无效值返回默认
            }
        } else if (key == "DefaultMaxBooks") {
            try {
                int maxBooks = std::stoi(value);
                if (maxBooks >= MIN_MAX_BOOKS && maxBooks <= MAX_MAX_BOOKS) {
                    defaultMaxBooks = maxBooks;
                }
            } catch (...) {
                // 无效值返回默认
            }
        }
    }
}

void Config::saveSettings() {
    std::ofstream ofs(SETTINGS_FILE);

    if (!ofs.is_open()) {
        // 打开文件错误
        return;
    }

    // 写入文件头
    ofs << "Setting,Value" << std::endl;

    // 写入设置
    ofs << "AdvancedUIMode," << (advancedUIMode ? "1" : "0") << std::endl;
    ofs << "BorrowPeriodDays," << borrowPeriodDays << std::endl;
    ofs << "FinePerDay," << std::fixed << std::setprecision(2) << finePerDay << std::endl;
    ofs << "MaxFine," << std::fixed << std::setprecision(2) << maxFine << std::endl;
    ofs << "DefaultMaxBooks," << defaultMaxBooks << std::endl;

    ofs.close();
}

// 可配置设置的获取器
bool Config::isAdvancedUIMode() const {
    return advancedUIMode;
}

int Config::getBorrowPeriodDays() const {
    return borrowPeriodDays;
}

double Config::getFinePerDay() const {
    return finePerDay;
}

double Config::getMaxFine() const {
    return maxFine;
}

int Config::getDefaultMaxBooks() const {
    return defaultMaxBooks;
}

void Config::setAdvancedUIMode(bool enabled) {
    advancedUIMode = enabled;
}

void Config::setBorrowPeriodDays(int days) {
    if (days > 0 && days <= MAX_TOTAL_BORROW_DAYS) {
        borrowPeriodDays = days;
    }
}

void Config::setFinePerDay(double fine) {
    if (fine >= 0.0) {
        finePerDay = fine;
    }
}

void Config::setMaxFine(double fine) {
    if (fine >= 0.0) {
        maxFine = fine;
    }
}

void Config::setDefaultMaxBooks(int maxBooks) {
    if (maxBooks >= MIN_MAX_BOOKS && maxBooks <= MAX_MAX_BOOKS) {
        defaultMaxBooks = maxBooks;
    }
}

// 实用方法
bool Config::isValidGenre(const std::string &genre) {
    for (const auto & g : GENRES) {
        if (g == genre) {
            return true;
        }
    }
    return false;
}

std::string Config::formatMemberID(int number) {
    std::ostringstream oss;
    oss << MEMBER_ID_PREFIX << std::setfill('0') << std::setw(MEMBER_ID_NUMBER_WIDTH) << number;
    return oss.str();
}

std::string Config::formatTransactionID(int number) {
    std::ostringstream oss;
    oss << TRANSACTION_ID_PREFIX << std::setfill('0') << std::setw(TRANSACTION_ID_NUMBER_WIDTH) << number;
    return oss.str();
}

std::string Config::formatReservationID(int number) {
    std::stringstream oss;
    oss << RESERVATION_ID_PREFIX << std::setfill('0') << std::setw(RESERVATION_ID_NUMBER_WIDTH) << number;
    return oss.str();
}
