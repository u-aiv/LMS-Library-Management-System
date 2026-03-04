#ifndef LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H
#define LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H

#include <string>
#include <unordered_map>

class Config {
public:
    // 单例方法
    static Config& getInstance();

    // 禁止复制
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 文件路径
    static const std::string DATA_DIR;
    static const std::string BOOKS_FILE;
    static const std::string MEMBERS_FILE;
    static const std::string TRANSACTIONS_FILE;
    static const std::string RESERVATIONS_FILE;
    static const std::string SETTINGS_FILE;
    static const std::string REPORTS_DIR;

    // 书目种类
    static const std::string GENRES[5];
    static constexpr int GENRES_COUNT = 5;

    // 借阅规则
    static constexpr int DEFAULT_BORROW_DAYS = 14;
    static constexpr int RENEWAL_DAYS = 7;
    static constexpr int MAX_RENEWALS = 2;      // 最大共 30 天
    static constexpr int MAX_TOTAL_BORROW_DAYS = 30;

    // 成员设置
    static constexpr int DEFAULT_MAX_BOOKS = 2;
    static constexpr int ADMIN_MAX_RENEWALS = 10;
    static constexpr int MEMBERSHIP_DURATION_DAYS = 365 * 4;  // 默认 4 年期限
    static constexpr int MIN_MAX_BOOKS = 1;
    static constexpr int MAX_MAX_BOOKS = 10;

    // 罚款设置
    static const double FINE_PER_DAY;
    static const double MAX_FINE;

    // 验证规则
    static constexpr int ISBN_LENGTH = 13;
    static constexpr int PHONE_LENGTH = 10;
    static constexpr int MIN_TITLE_LENGTH = 1;
    static constexpr int MAX_TITLE_LENGTH = 200;
    static constexpr int MIN_NAME_LENGTH = 1;
    static constexpr int MAX_NAME_LENGTH = 100;
    static constexpr int MIN_AUTHOR_LENGTH = 1;
    static constexpr int MAX_AUTHOR_LENGTH = 100;
    static constexpr int MIN_PUBLISHER_LENGTH = 1;
    static constexpr int MAX_PUBLISHER_LENGTH = 100;

    // ID 前缀
    static const std::string MEMBER_ID_PREFIX;
    static const std::string ADMIN_ID_PREFIX;
    static const std::string TRANSACTION_ID_PREFIX;
    static const std::string RESERVATION_ID_PREFIX;

    // ID 格式
    static constexpr int MEMBER_ID_NUMBER_WIDTH = 8;  // e.g. M20261001
    static constexpr int TRANSACTION_ID_NUMBER_WIDTH = 10;  // e.g. T2026100001
    static constexpr int RESERVATION_ID_NUMBER_WIDTH = 10;  // e.g. R2026100001

    // 日期格式
    static const std::string DATE_FORMAT;  // YYYY-MM-DD

    // CSV 设置
    static constexpr char CSV_DELIMITER = ',';
    static constexpr char CSV_LIST_DELIMITER = ';';  // 用于字段内的列表

    // UI 设置
    static constexpr int UI_BOX_WIDTH = 50;
    static constexpr char UI_HORIZONTAL_CHAR = '=';
    static constexpr char UI_VERTICAL_CHAR = '|';
    static constexpr char UI_CORNER_CHAR = '+';

    // 新用户的默认密码
    static const std::string DEFAULT_PASSWORD;

    // 系统消息
    static const std::string SUCCESS_MESSAGE_PREFIX;
    static const std::string ERROR_MESSAGE_PREFIX;
    static const std::string WARNING_MESSAGE_PREFIX;
    static const std::string INFO_MESSAGE_PREFIX;

    // 加载和保存设置
    void loadSettings();
    void saveSettings();

    // 可配置的设置获取器 (可在运行时被修改)
    bool isAdvancedUIMode() const;
    int getBorrowPeriodDays() const;
    double getFinePerDay() const;
    double getMaxFine() const;
    int getDefaultMaxBooks() const;

    // 可配置的设置设置器
    void setAdvancedUIMode(bool enabled);
    void setBorrowPeriodDays(int days);
    void setFinePerDay(double fine);
    void setMaxFine(double fine);
    void setDefaultMaxBooks(int maxBooks);

    // 实用方法
    static bool isValidGenre(const std::string& genre);
    static std::string formatMemberID(int number);
    static std::string formatTransactionID(int number);
    static std::string formatReservationID(int number);

private:
    // 单例的私有构造函数
    Config();

    // 运行时可配置设置
    bool advancedUIMode;
    int borrowPeriodDays;
    double finePerDay;
    double maxFine;
    int defaultMaxBooks;

    // 设置存储
    std::unordered_map<std::string, std::string> settings;

    // 辅助方法
    void initializeDefaults();

    void parseSettingsLine(const std::string &line);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H
