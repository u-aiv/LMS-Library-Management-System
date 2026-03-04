// main.cpp

#include "config/Config.h"
#include "authentication/auth.h"
#include "models/Book.h"
#include "models/Member.h"
#include "managers/BookManager.h"
#include "managers/MemberManager.h"
#include "managers/TransactionManager.h"
#include "managers/ReservationManager.h"
#include "managers/RecommendationManager.h"
#include "ui/UI.h"
#include "ui/MenuHandler.h"
#include "utils/DateUtils.h"
#include "utils/FileHandler.h"

#include <clocale>
#include <iostream>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace {

bool ensureWorkingDirHasData() {
    FileHandler fileHandler;
    // 找寻数据
    for (int i = 0; i < 4; ++i) {
        if (fileHandler.isFileExist(Config::BOOKS_FILE) &&
            fileHandler.isFileExist(Config::MEMBERS_FILE) &&
            fileHandler.isFileExist(Config::TRANSACTIONS_FILE) &&
            fileHandler.isFileExist(Config::RESERVATIONS_FILE)) {
            return true;
        }
    #ifdef _WIN32
            _chdir("..");
    #else
            chdir("..");
    #endif
    }
    return false;
}

void setupConsoleEnvironment() {
    // 采用 UTF-8
    std::setlocale(LC_ALL, "");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void ensureSeedMembers(MemberManager& memberManager) {
    if (memberManager.getTotalMembers() > 0) {
        return;
    }

    const std::string today = DateUtils::getCurrentDate();
    const std::string expiry = DateUtils::addDays(today, 365 * 4);

    // 首次启动时的默认管理员和成员
    const Member admin(
        "A20261001",
        "System Admin",
        "13000000000",
        std::vector<std::string>{"Science", "History"},
        today,
        expiry,
        10,
        true,
        auth::hashPassword("admin123")
    );

    const Member user(
        "M20261001",
        "Default User",
        "13100000000",
        std::vector<std::string>{"Fiction", "Biography"},
        today,
        expiry,
        2,
        false,
        auth::hashPassword("user123")
    );

    memberManager.addMember(admin);
    memberManager.addMember(user);
}

void ensureSeedBooks(BookManager& bookManager) {
    if (bookManager.getTotalBooks() > 0) {
        return;
    }

    // 测试样例用书
    const std::vector<Book> sampleBooks = {
        Book("9787020002207", "Dream of the Red Chamber", "Cao Xueqin", "People's Literature", "Fiction", 5, 5, false),
        Book("9787544253994", "The Three-Body Problem", "Liu Cixin", "Chongqing Press", "Science", 4, 4, false),
        Book("9787544771047", "Sapiens", "Yuval Noah Harari", "CITIC Press", "History", 3, 3, false),
        Book("9780307474278", "The Great Gatsby", "F. Scott Fitzgerald", "Scribner", "Fiction", 4, 4, false),
        Book("9780062315007", "Steve Jobs", "Walter Isaacson", "Simon & Schuster", "Biography", 2, 2, false)
    };

    for (const auto & sampleBook : sampleBooks) {
        bookManager.addBook(sampleBook);
    }
}

void bootstrapDefaultData(BookManager& bookManager, MemberManager& memberManager) {
    ensureSeedMembers(memberManager);
    ensureSeedBooks(bookManager);
}

} // namespace

int main() {
    try {
        setupConsoleEnvironment();

        // 确保工作目录指向 data/
        ensureWorkingDirHasData();

        Config& config = Config::getInstance();
        config.loadSettings();

        // 如果设置为空, 则写入默认配置
        config.saveSettings();

        BookManager bookManager(Config::BOOKS_FILE);
        MemberManager memberManager(Config::MEMBERS_FILE);
        TransactionManager transactionManager(Config::TRANSACTIONS_FILE);
        ReservationManager reservationManager(Config::RESERVATIONS_FILE);
        RecommendationManager recommendationManager(
            Config::BOOKS_FILE,
            Config::MEMBERS_FILE,
            Config::TRANSACTIONS_FILE
        );

        bootstrapDefaultData(bookManager, memberManager);

        UI::DisplayMode mode = config.isAdvancedUIMode()
            ? UI::DisplayMode::ADVANCED
            : UI::DisplayMode::SIMPLE;
        UI ui(mode);

        ui.displayHeader("图书馆管理系统", "C++11 控制台版本");
        ui.displayMessage("初始管理员: A20261001 / admin123", UI::MessageType::INFO);
        ui.displayMessage("初始会员: M20261001 / user123", UI::MessageType::INFO);
        ui.pause("按任意键继续...");

        MenuHandler menuHandler(
            bookManager,
            memberManager,
            transactionManager,
            reservationManager,
            recommendationManager,
            ui
        );

        menuHandler.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[FATAL] 遇到未知错误" << std::endl;
    }

    return 1;
}
