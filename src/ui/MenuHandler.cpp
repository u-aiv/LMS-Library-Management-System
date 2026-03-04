// MenuHandler.cpp 实现

#include "MenuHandler.h"
#include "../managers/BookManager.h"
#include "../managers/MemberManager.h"
#include "../managers/TransactionManager.h"
#include "../managers/ReservationManager.h"
#include "../managers/RecommendationManager.h"
#include "../managers/BackupManager.h"
#include "../managers/ReportManager.h"
#include "../models/Book.h"
#include "../models/Member.h"
#include "../authentication/auth.h"
#include "../config/Config.h"
#include "UI.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <sstream>
#include <map>

// 构造函数
MenuHandler::MenuHandler(BookManager &bm, MemberManager &mm,
    TransactionManager &tm, ReservationManager& rsm, RecommendationManager& rcm, UI &ui)
    : bookManager(bm), memberManager(mm), transactionManager(tm), reservationManager(rsm), recommendationManager(rcm), ui(ui),
      currentUser(nullptr), isRunning(false) {}

// 析构函数
MenuHandler::~MenuHandler() {
    if (currentUser != nullptr) {
        logout();
    }
}

// 公有方法

bool MenuHandler::login() {
    clearScreen();
    displayWelcomeScreen();

    std::string memberID = promptForInput("输入您的 ID (按 q 以退出): ", false);
    if (memberID.empty() || memberID == "q" || memberID == "Q") {
        return false;
    }

    std::string password = promptForInput("请输入密码: ", false);
    if (password.empty()) {
        return false;
    }

    currentUser = memberManager.authenticateUser(memberID, password);

    if (currentUser == nullptr) {
        displayMessage("凭证无效, 请再试一次!", "error");
        pauseScreen();
        return false;
    }

    if (currentUser->isExpired()) {
        displayMessage("您的账户已过期, 请联系管理员", "error");
        currentUser = nullptr;
        pauseScreen();
        return false;
    }

    displayMessage("已登录. 欢迎使用, " + currentUser->getName() + " !", "success");
    return true;
}

bool MenuHandler::logout() {
    if (currentUser != nullptr) {
        displayMessage("正在注销... 再见, " + currentUser->getName() + " !", "info");
        currentUser = nullptr;
        pauseScreen();
        return true;
    }
    return false;
}

void MenuHandler::run() {
    isRunning = true;

    while (isRunning) {
        if (currentUser == nullptr) {
            if (!login()) {
                isRunning = false;
                break;
            }
        }
        clearScreen();
        if (currentUser->getAdmin()) {
            displayAdminMenu();
        } else {
            displayMemberMenu();
        }
    }
}

void MenuHandler::exit() {
    isRunning = false;
}

bool MenuHandler::isApplicationRunning() const {
    return isRunning;
}

Member* MenuHandler::getCurrentUser() const {
    return currentUser;
}

// 菜单显示方法

void MenuHandler::displayWelcomeScreen() {
    ui.displayHeader("Library Management System");
    std::cout << "\n";
    std::cout << "  ╔════════════════════════════════════════════╗\n";
    std::cout << "  ║                    图书馆管理系统                 ║";
    std::cout << "  ║                                            ║\n";
    std::cout << "  ║            您 的 知 识 与 学 习 门 户               ║\n";
    std::cout << "  ╚════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void MenuHandler::displayMemberMenu() {
    ui.displayHeader("会员菜单: " + currentUser->getName());

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│   书籍操作                                  │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  1. 搜索图书                                │\n";
    std::cout << "│  2. 借阅图书                            │\n";
    std::cout << "│  3. 归还图书                            │\n";
    std::cout << "│  4. 续约图书                             │\n";
    std::cout << "│  5. 预约图书                           │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│   账户操作                                  │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  6. 查看借书                            │\n";
    std::cout << "│  7. 查看借阅历史                              │\n";
    std::cout << "│  8. 查看预订                            │\n";
    std::cout << "│  9. 查看推荐                            │\n";
    std::cout << "│ 10. 个人资料                            │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  0. 注销                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 11);
    switch (choice) {
        case 1: handleSearchBooks(); break;
        case 2: handleBorrowBook(); break;
        case 3: handleReturnBook(); break;
        case 4: handleRenewBook(); break;
        case 5: handleReserveBook(); break;
        case 6: handleViewBorrowedBooks(); break;
        case 7: handleViewBorrowingHistory(); break;
        case 8: handleViewReservations(); break;
        case 9: handleViewRecommendations(); break;
        case 10: handleViewProfile(); break;
        case 0: logout(); break;
        default: {
            displayMessage("无效选择, 请重试", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::displayAdminMenu() {
    ui.displayHeader("管理员菜单: " + currentUser->getName());

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│                  管理员                   │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  1. 管理图书                            │\n";
    std::cout << "│  2. 管理会员                            │\n";
    std::cout << "│  3. 管理交易                            │\n";
    std::cout << "│  4. 查看逾期                            │\n";
    std::cout << "│  5. 生成报告                            │\n";
    std::cout << "│  6. 备份还原                            │\n";
    std::cout << "│  7. 系统设置                            │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  0. 注销                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 7);
    switch (choice) {
        case 1: handleManageBooks(); break;
        case 2: handleManageMembers(); break;
        case 3: handleManageTransactions(); break;
        case 4: handleViewOverdueBooks(); break;
        case 5: handleGenerateReports(); break;
        case 6: handleBackupRestore(); break;
        case 7: handleSystemSettings(); break;
        case 0: logout(); break;
        default: {
            displayMessage("无效选择, 请重试", "error");
            pauseScreen();
            break;
        }
    }
}

// 会员目录处理器

void MenuHandler::handleSearchBooks() {
    clearScreen();
    ui.displayHeader("搜索图书: ");

    std::cout << "\n";
    std::cout << "搜索项:\n";
    std::cout << "  1. 书名\n";
    std::cout << "  2. 作者\n";
    std::cout << "  3. ISBN\n";
    std::cout << "  4. 类型\n";
    std::cout << "  5. 出版社\n";
    std::cout << "  0. 回退\n\n";

    int choice = promptForInt("请输入你的选择: ", 0, 5);
    switch (choice) {
        case 1: performBookSearch("title"); break;
        case 2: performBookSearch("author"); break;
        case 3: performBookSearch("isbn"); break;
        case 4: performBookSearch("genre"); break;
        case 5: performBookSearch("publisher"); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleBorrowBook() {
    clearScreen();
    ui.displayHeader("借阅图书: ");

    std::string isbn = promptForInput("请输入欲借阅图书的 ISBN: ");
    if (isbn.empty()) {
        return;
    }

    Book* book = bookManager.findBookByISBN(isbn);
    if (!book) {
        displayMessage("查无此书, 请再试一次", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    if (!confirmAction("您想借阅这本书吗?")) {
        return;
    }

    if (transactionManager.borrowBook(memberManager, bookManager, currentUser->getMemberID(), isbn) != "0") {
        displayMessage("成功借阅!", "success");
    } else {
        displayMessage("借书失败, 请检查图书是否可借或您的借阅额度", "error");
    }
    pauseScreen();
}

void MenuHandler::handleReturnBook() {
    clearScreen();
    ui.displayHeader("归还图书: ");

    // Display current borrowed books
    auto borrowedBook = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBook.empty()) {
        displayMessage("您没有要还的书", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n已借图书:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date" << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& transaction : borrowedBook) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(35) << book->getTitle()
                      << std::setw(15) << transaction->getBorrowDate()
                      << std::setw(15) << transaction->getDueDate() << "\n";
        }
    }
    std::cout << std::string(80, '-') << "\n\n";

    std::string isbn = promptForInput("请输入欲还图书的 ISBN: ");
    if (isbn.empty()) {
        return;
    }

    if (transactionManager.returnBook(bookManager, currentUser->getMemberID(), isbn)) {
        displayMessage("成功还书!", "success");
    } else {
        displayMessage("归还书籍失败, 请检查输入的 ISBN", "error");
    }
    pauseScreen();
}

void MenuHandler::handleRenewBook() {
    clearScreen();
    ui.displayHeader("续约图书");

    auto borrowedBook = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBook.empty()) {
        displayMessage("您没有可续借的书", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n";
    std::cout << "已借图书:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(15) << "Due Date"
              << std::setw(15) << "Renewals" << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& transaction : borrowedBook) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(35) << book->getTitle()
                      << std::setw(15) << transaction->getDueDate()
                      << std::setw(15) << transaction->getRenewCount() << "\n";
        }
    }
    std::cout << std::string(80, '-') << "\n\n";

    std::string isbn = promptForInput("请输入欲续约图书的 ISBN: ");
    if (isbn.empty()) return;

    if (transactionManager.renewBook(currentUser->getMemberID(), isbn)) {
        displayMessage("成功续约图书!", "success");
    } else {
        displayMessage("续借图书失败, 您可能已达到最大续借次数", "error");
    }
    pauseScreen();
}

void MenuHandler::handleReserveBook() {
    clearScreen();
    ui.displayHeader("预定图书");

    std::string isbn = promptForInput("请输入欲预订图书的 ISBN: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("查无此书", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    int currentQueueLength = reservationManager.getQueueLength(isbn);
    if (currentQueueLength > 0) {
        std::cout << "\n";
        std::cout << "⚠ 预定队列信息:\n";
        std::cout << "  当前队长: " << currentQueueLength << " 人等待中\n";
        std::cout << "  您将位于: " << (currentQueueLength + 1) << "\n\n";
    }

    if (book->canBorrow()) {
        displayMessage("这本书目前有库存, 您可以直接借阅, 无需预约", "info");
        if (!confirmAction("您还想预订这本书吗?")) {
            return;
        }
    }

    // 检查该会员是否已有此书的有效预约
    auto memberReservations = reservationManager.findByMemberID(currentUser->getMemberID());
    for (const auto* reservation : memberReservations) {
        if (reservation->getISBN() == isbn && reservation->getIsActive()) {
            displayMessage("您已经有这本书的有效预订", "error");

            int position = reservationManager.getQueuePosition(reservation->getReservationID());
            if (position > 0) {
                std::cout << "您当前位于: " << position << "\n";
            }
            pauseScreen();
            return;
        }
    }

    if (!confirmAction("您想预订这本书吗?")) {
        return;
    }

    std::string reservationID = reservationManager.reserveBook(currentUser->getMemberID(), isbn);

    if (reservationID == "0") {
        displayMessage("预订图书失败, 请重试或联系管理员", "error");
    } else {
        displayMessage("图书预订成功! 预订 ID: " + reservationID, "success");

        // Show queue information
        int position = reservationManager.getQueuePosition(reservationID);
        int queueLength = reservationManager.getQueueLength(isbn);

        std::cout << "\n预订详细信息:\n";
        std::cout << "  您位于: " << position << " / " << queueLength << "\n";

        if (position == 1) {
            std::cout << "   您排在队首!\n";
        } else {
            std::cout << "  预计等待: " << (position - 1) << " 个人\n";
        }
    }

    pauseScreen();
}

void MenuHandler::handleViewBorrowedBooks() {
    clearScreen();
    ui.displayHeader("当前已借阅图书");

    auto borrowedBooks = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBooks.empty()) {
        displayMessage("您没有已借阅图书", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(30) << "Title"
              << std::setw(20) << "Author"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date"
              << std::setw(5) << "Fine" << "\n";
    std::cout << std::string(100, '=') << "\n";

    double totalFine = 0.0;
    for (const auto& transaction : borrowedBooks) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            double fine = transaction->calculateFine();
            totalFine += fine;

            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(30) << book->getTitle().substr(0, 28)
                      << std::setw(20) << book->getAuthor().substr(0, 18)
                      << std::setw(15) << transaction->getBorrowDate()
                      << std::setw(15) << transaction->getDueDate()
                      << std::setw(5) << std::fixed << std::setprecision(2) << fine << "\n";
        }
    }
    std::cout << std::string(100, '=') << "\n";
    std::cout << "共计罚款: $" << std::fixed << std::setprecision(2) << totalFine << "\n";
    std::cout << std::string(100, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewBorrowingHistory() {
    clearScreen();
    ui.displayHeader("借阅历史");

    auto history = transactionManager.getMemberHistory(currentUser->getMemberID());

    if (history.empty()) {
        displayMessage("无借阅历史", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(110, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(30) << "Title"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Return Date"
              << std::setw(10) << "Status"
              << std::setw(10) << "Fine" << "\n";
    std::cout << std::string(110, '=') << "\n";

    for (const auto& transaction : history) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        std::string title = book ? book->getTitle().substr(0, 28) : "未知";

        std::cout << std::left << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << title
                  << std::setw(15) << transaction->getBorrowDate()
                  << std::setw(15) << (transaction->getReturnDate().empty() ? "N/A" : transaction->getReturnDate())
                  << std::setw(10) << (transaction->haveReturned() ? "已归还" : "活跃")
                  << std::setw(10) << std::fixed << std::setprecision(2) << transaction->getFine() << "\n";
    }
    std::cout << std::string(110, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewReservations() {
    clearScreen();
    ui.displayHeader("我的预约");

    auto myReservations = reservationManager.findByMemberID(currentUser->getMemberID());

    if (myReservations.empty()) {
        displayMessage("您没有预约", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(140, '=') << "\n";
    std::cout << std::left << std::setw(15) << "预约 ID"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(18) << "预定日期"
              << std::setw(12) << "状态"
              << std::setw(10) << "位置"
              << std::setw(10) << "队列"
              << std::setw(15) << "可用"
              << std::setw(15) << "行动" << "\n";
    std::cout << std::string(140, '=') << "\n";

    int activeCount = 0;
    int nextInLineCount = 0;

    for (const auto* reservation : myReservations) {
        Book* book = bookManager.findBookByISBN(reservation->getISBN());
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";
        std::string status = reservation->getIsActive() ? "活跃" : "已取消";

        // 获得队列信息
        std::string positionStr = "-";
        std::string queueLengthStr = "-";
        std::string availableStr = "-";

        if (reservation->getIsActive()) {
            activeCount++;

            // 获得队列位置
            int position = reservationManager.getQueuePosition(reservation->getReservationID());
            int queueLength = reservationManager.getQueueLength(reservation->getISBN());

            if (position > 0) {
                if (position == 1) {
                    positionStr = "→ 下一个!";
                    nextInLineCount++;
                } else {
                    positionStr = std::to_string(position);
                }
                queueLengthStr = std::to_string(queueLength);
            }

            // Check if book is available
            if (book) {
                availableStr = std::to_string(book->getAvailableCopies()) + "/" + std::to_string(book->getTotalCopies());
            }
        }

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate()
                  << std::setw(12) << status
                  << std::setw(10) << positionStr
                  << std::setw(10) << queueLengthStr
                  << std::setw(15) << availableStr
                  << std::setw(15) << (reservation->getIsActive() ? "可取消" : "-") << "\n";
    }
    std::cout << std::string(140, '=') << "\n";
    std::cout << "总预约: " << myReservations.size()
              << " (有效: " << activeCount;

    if (nextInLineCount > 0) {
        std::cout << ", 下一个: " << nextInLineCount;
    }
    std::cout << ")\n";
    std::cout << std::string(140, '=') << "\n\n";

    // Helpful information
    if (activeCount > 0) {
        std::cout << "队列信息指引:\n";
        std::cout << "   • 位置 1(您是下一个!)= 书籍可用时将为你保留\n";
        std::cout << "   • 位置越高 = 排队前的人越多\n";
        // std::cout << "   • You'll be notified when you become next in line\n\n";
    }

    // 取消预约选项
    if (activeCount > 0) {
        if (confirmAction("您想要取消预约吗?")) {
            std::string reservationID = promptForInput("输入预订ID以取消: ");
            if (!reservationID.empty()) {
                std::string result = reservationManager.cancelReservation(reservationID);
                if (result != "0") {
                    displayMessage("预订已成功取消! 其他人的排队位置已更新", "success");
                } else {
                    displayMessage("取消预订失败, 请检查预订ID", "error");
                }
            }
        }
    }

    pauseScreen();
}

void MenuHandler::handleViewRecommendations() {
    clearScreen();
    ui.displayHeader("Book Recommendations");

    std::cout << "\nPersonalized Book Recommendations for " << currentUser->getName() << "\n\n";

    // 推荐算法 KNN 配置项
    const int TOP_N = 5;               // 显示的推荐数量
    const int K_NEIGHBORS = 5;         // 要考虑的相似用户数量
    const bool AVAILABLE_ONLY = false; // 显示所有书籍 (用户可以预约不可用的书籍)

    std::cout << "正在分析您的阅读偏好并寻找相似的读者...\n";
    std::cout << "   使用协同过滤与 " << K_NEIGHBORS << " 临近邻居\n\n";

    // Use the KNN algorithm
    std::vector<Book> recommendations = recommendationManager.recommendForMember(
        currentUser->getMemberID(),
        TOP_N,
        K_NEIGHBORS,
        AVAILABLE_ONLY
    );

    if (recommendations.empty()) {
        displayMessage("暂时没有可用的推荐", "info");
        std::cout << "\n这也许因为:\n";
        std::cout << "  • 您是新会员(开始借书以获得个性化推荐)\n";
        std::cout << "  • 未发现相似的阅读模式\n";
        std::cout << "  • 所有推荐的书籍您都已经借过了\n\n";

        std::cout << "提示: 在您的个人资料中更新阅读偏好, 以获得更好的推荐!\n";
        pauseScreen();
        return;
    }

    std::cout << "为你推荐 " << recommendations.size() << " 本书:\n\n";
    std::cout << std::string(125, '=') << "\n";
    std::cout << std::left << std::setw(5) << "#"
              << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(25) << "Author"
              << std::setw(20) << "Genre"
              << std::setw(12) << "Status"
              << std::setw(8) << "Copies"
              << std::setw(5) << "⭐" << "\n";
    std::cout << std::string(125, '=') << "\n";

    int rank = 1;
    for (const auto& book : recommendations) {
        std::string status;
        std::string copiesInfo;

        if (book.canBorrow()) {
            status = " Available";
            copiesInfo = std::to_string(book.getAvailableCopies()) + "/" +
                        std::to_string(book.getTotalCopies());
        } else {
            status = " Reserved";
            int queueLength = reservationManager.getQueueLength(book.getISBN());
            copiesInfo = "Queue:" + std::to_string(queueLength);
        }

        // Recommendation strength indicator (top 5 get stars)
        std::string stars = (rank <= 5) ? "⭐" : "";

        std::cout << std::left << std::setw(5) << rank
                  << std::setw(15) << book.getISBN()
                  << std::setw(35) << book.getTitle().substr(0, 33)
                  << std::setw(25) << book.getAuthor().substr(0, 23)
                  << std::setw(20) << book.getGenre().substr(0, 18)
                  << std::setw(12) << status
                  << std::setw(8) << copiesInfo
                  << std::setw(5) << stars << "\n";
        rank++;
    }
    std::cout << std::string(125, '=') << "\n";

    std::cout << "\n⭐ = 高度推荐 (top 5)\n";
    std::cout << " = 可立即借阅\n";
    std::cout << " = 可以预订(您将被加入队列)\n\n";

    // 显示推荐统计
    int availableCount = 0;
    int reservedCount = 0;
    std::map<std::string, int> genreCount;

    for (const auto& book : recommendations) {
        if (book.canBorrow()) {
            availableCount++;
        } else {
            reservedCount++;
        }
        genreCount[book.getGenre()]++;
    }

    std::cout << "推荐细分:\n";
    std::cout << "   总推荐: " << recommendations.size() << "\n";
    std::cout << "   现已提供:        " << availableCount << "\n";
    std::cout << "   需要预约: " << reservedCount << "\n";

    if (!genreCount.empty()) {
        std::cout << "   推荐的热门类型: ";
        int shown = 0;
        for (const auto& pair : genreCount) {
            if (shown > 0) std::cout << ", ";
            std::cout << pair.first << " (" << pair.second << ")";
            if (++shown >= 3) break;
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    pauseScreen();
}

void MenuHandler::handleViewProfile() {
    clearScreen();
    ui.displayHeader("个人资料");

    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "会员 ID:       " << currentUser->getMemberID() << "\n";
    std::cout << "Name:            " << currentUser->getName() << "\n";
    std::cout << "Member Type:     " << (currentUser->getAdmin() ? "管理员" : "会员") << "\n";
    std::cout << "Account Status:  " << (currentUser->isExpired() ? "已过期" : "活跃") << "\n";
    std::cout << "Registration:    " << currentUser->getRegistrationDate() << "\n";
    std::cout << std::string(50, '=') << "\n";

    // 显示借阅统计
    auto activeTransactions = transactionManager.getActiveTransactions(currentUser->getMemberID());
    auto history = transactionManager.getMemberHistory(currentUser->getMemberID());

    std::cout << "\n借阅统计:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "当前已借阅: " << activeTransactions.size() << "\n";
    std::cout << "总借阅数:     " << history.size() << "\n";
    std::cout << std::string(50, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleManageBooks() {
    clearScreen();
    ui.displayHeader("管理图书");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 新增图书                                │\n";
    std::cout << "│  2. 更新图书                                │\n";
    std::cout << "│  3. 删除图书                                │\n";
    std::cout << "│  4. 查看所有图书                              │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 4);

    switch (choice) {
        case 1: handleAddBook(); break;
        case 2: handleUpdateBook(); break;
        case 3: handleDeleteBook(); break;
        case 4: handleViewAllBooks(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleManageMembers() {
    clearScreen();
    ui.displayHeader("会员管理");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 新增会员                                │\n";
    std::cout << "│  2. 更新会员                                │\n";
    std::cout << "│  3. 删除会员                                │\n";
    std::cout << "│  4. 查看所有会员                              │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 4);

    switch (choice) {
        case 1: handleAddMember(); break;
        case 2: handleUpdateMember(); break;
        case 3: handleDeleteMember(); break;
        case 4: handleViewAllMembers(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleManageTransactions() {
    clearScreen();
    ui.displayHeader("Manage Transactions");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 查看所有交易                              │\n";
    std::cout << "│  2. 查看活跃交易                              │\n";
    std::cout << "│  3. 手动归还                                │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 3);

    switch (choice) {
        case 1: handleViewAllTransactions(); break;
        case 2: handleViewActiveTransactions(); break;
        case 3: handleManualReturn(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleViewOverdueBooks() {
    clearScreen();
    ui.displayHeader("逾期图书");

    auto overdueTransactions = transactionManager.getOverdueTransactions();

    if (overdueTransactions.empty()) {
        displayMessage("未找到逾期图书", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(12) << "会员 ID"
              << std::setw(20) << "会员姓名"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(15) << "Due Date"
              << std::setw(12) << "Days Late"
              << std::setw(10) << "Fine" << "\n";
    std::cout << std::string(120, '=') << "\n";

    double totalFines = 0.0;
    for (const auto& transaction : overdueTransactions) {
        Member* member = memberManager.findMemberByID(transaction->getUserID());
        Book* book = bookManager.findBookByISBN(transaction->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "未知";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";
        double fine = transaction->calculateFine();
        totalFines += fine;

        std::cout << std::left << std::setw(12) << transaction->getUserID()
                  << std::setw(20) << memberName
                  << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(15) << transaction->getDueDate()
                  << "$" << std::setw(9) << std::fixed << std::setprecision(2) << fine << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "逾期图书总数: " << overdueTransactions.size() << "\n";
    std::cout << "总罚款: $" << std::fixed << std::setprecision(2) << totalFines << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleGenerateReports() {
    clearScreen();
    ui.displayHeader("生成报告");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 库存报告                                │\n";
    std::cout << "│  2. 会员报告                                │\n";
    std::cout << "│  3. 交易报告                                │\n";
    std::cout << "│  4. 统计报告                                │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 5);

    switch (choice) {
        case 1: handleGenerateInventoryReport(); break;
        case 2: handleGenerateMemberReport(); break;
        case 3: handleGenerateTransactionReport(); break;
        case 4: handleGenerateStatisticsReport(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleBackupRestore() {
    clearScreen();
    ui.displayHeader("备份 & 恢复");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 备份数据                                │\n";
    std::cout << "│  2. 还原数据                                │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 2);

    switch (choice) {
        case 1: handleBackupData(); break;
        case 2: handleRestoreData(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleSystemSettings() {
    clearScreen();
    ui.displayHeader("系统设置");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 变更UI模式                              │\n";
    std::cout << "│  2. 变更借阅期限                              │\n";
    std::cout << "│  3. 变更罚款设定                              │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 3);

    switch (choice) {
        case 1: handleChangeUIMode(); break;
        case 2: handleChangeBorrowPeriod(); break;
        case 3: handleChangeFineSettings(); break;
        case 0: return;
        default: {
            displayMessage("无效选择", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleAddBook() {
    clearScreen();
    ui.displayHeader("添加新书");

    std::string isbn = promptForInput("输入 ISBN: ");
    if (isbn.empty()) return;

    // Check if book already exists
    if (bookManager.findBookByISBN(isbn) != nullptr) {
        displayMessage("具有此 ISBN 的书已经存在", "error");
        pauseScreen();
        return;
    }

    std::string title = promptForInput("输入书名: ");
    if (title.empty()) return;

    std::string author = promptForInput("输入作者: ");
    if (author.empty()) return;

    std::string publisher = promptForInput("输入出版社: ");
    if (publisher.empty()) return;

    std::string genre = promptForInput("输入类型: ");
    if (genre.empty()) return;

    int totalCopies = promptForInt("输入总册数: ", 1, 1000);

    // 使用构造函数参数创建新书
    Book* newBook = new Book(isbn, title, author, publisher, genre, totalCopies, totalCopies, false);

    if (bookManager.addBook(*newBook)) {
        displayMessage("图书添加成功!", "success");
    } else {
        displayMessage("图书添加失败", "error");
        delete newBook;
    }

    pauseScreen();
}

void MenuHandler::handleUpdateBook() {
    clearScreen();
    ui.displayHeader("更新图书信息");

    std::string isbn = promptForInput("输入图书 ISBN 以更新: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("查无此书", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    std::cout << "\n输入新值 (按下 Enter 保留原设置):\n\n";

    // 获取新值或保留现有值
    std::string newTitle = promptForInput("书名 [" + book->getTitle() + "]: ");
    std::string title = newTitle.empty() ? book->getTitle() : newTitle;

    std::string newAuthor = promptForInput("作者 [" + book->getAuthor() + "]: ");
    std::string author = newAuthor.empty() ? book->getAuthor() : newAuthor;

    std::string newPublisher = promptForInput("出版社 [" + book->getPublisher() + "]: ");
    std::string publisher = newPublisher.empty() ? book->getPublisher() : newPublisher;

    std::string newGenre = promptForInput("类型 [" + book->getGenre() + "]: ");
    std::string genre = newGenre.empty() ? book->getGenre() : newGenre;

    Book updatedBook(
        book->getISBN(),
        title,
        author,
        publisher,
        genre,
        book->getTotalCopies(),
        book->getAvailableCopies(),
        false
    );

    if (bookManager.updateBook(updatedBook)) {
        displayMessage("图书更新成功!", "success");
        // refresh memory and cache
        bookManager.clearCache();
        bookManager.reload();
    } else {
        displayMessage("图书更新失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteBook() {
    clearScreen();
    ui.displayHeader("删除图书");

    std::string isbn = promptForInput("输入图书 ISBN 以删除: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("查无此书", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    if (!confirmAction("您确定要删除这本书吗?")) {
        return;
    }

    if (bookManager.deleteBook(isbn)) {
        displayMessage("图书删除成功!", "success");
    } else {
        displayMessage("删除书籍失败, 该书可能有正在借阅的记录", "error");
    }

    pauseScreen();
}

void MenuHandler::handleViewAllBooks() {
    clearScreen();
    ui.displayHeader("馆藏所有图书");

    // 如果用户在程序外手动修改了 CSV
    bookManager.clearCache();
    bookManager.reload();
    auto allBooks = bookManager.getAllBooks();

    if (allBooks.empty()) {
        displayMessage("馆藏无书", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(25) << "Author"
              << std::setw(20) << "Genre"
              << std::setw(10) << "Total"
              << std::setw(10) << "Available" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& book : allBooks) {
        std::cout << std::left << std::setw(15) << book.getISBN()
                  << std::setw(35) << book.getTitle().substr(0, 33)
                  << std::setw(25) << book.getAuthor().substr(0, 23)
                  << std::setw(20) << book.getGenre().substr(0, 18)
                  << std::setw(10) << book.getTotalCopies()
                  << std::setw(10) << book.getAvailableCopies() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "图书总数: " << allBooks.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleAddMember() {
    clearScreen();
    ui.displayHeader("新增会员");

    std::string name = promptForInput("输入姓名: ");
    if (name.empty()) return;

    std::string phone = promptForInput("输入手机号码: ");
    if (phone.empty()) return;

    std::string password = promptForInput("输入密码: ");
    if (password.empty()) return;

    // 获取类型偏好
    std::vector<std::string> preferences;
    std::cout << "\n输入阅读偏好 (类型):\n";
    std::cout << "可用类型: Fiction, Non-Fiction, Science, History, Biography\n";
    std::cout << "留空并按下 Enter 以结束输入\n\n";

    for (int i = 0; i < 5; i++) {
        std::string pref = promptForInput("类型 " + std::to_string(i + 1) + ": ");
        if (pref.empty()) break;
        preferences.push_back(pref);
    }

    std::cout << "\n会员类型:\n";
    std::cout << "  1. 普通会员\n";
    std::cout << "  2. 管理员\n";
    int typeChoice = promptForInt("输入选择: ", 1, 2);
    bool isAdmin = (typeChoice == 2);

    auto generateNextMemberID = [&](bool adminFlag) -> std::string {
        const std::string prefix = adminFlag ? Config::ADMIN_ID_PREFIX : Config::MEMBER_ID_PREFIX;
        const std::string currentDate = DateUtils::getCurrentDate();
        const std::string year = currentDate.substr(0, 4);
        int month = std::stoi(currentDate.substr(5, 2));
        int season = (month - 1) / 3 + 1;
        int maxSeq = 0;

        const auto& allMembers = memberManager.getAllMembers();
        for (const auto& m : allMembers) {
            const std::string id = m.getMemberID();
            if (id.empty() || id[0] != prefix[0]) {
                continue;
            }

            if (id.size() != 9) {
                continue;
            }
            if (id.substr(1, 4) != year) {
                continue;
            }
            if (!std::isdigit(static_cast<unsigned char>(id[5]))) {
                continue;
            }
            int idSeason = id[5] - '0';
            if (idSeason != season) {
                continue;
            }
            bool seqDigits = true;
            for (size_t i = 6; i < id.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(id[i]))) {
                    seqDigits = false;
                    break;
                }
            }
            if (!seqDigits) {
                continue;
            }
            int seq = std::stoi(id.substr(6));
            if (seq > maxSeq) {
                maxSeq = seq;
            }
        }

        std::ostringstream oss;
        oss << prefix << year << season
            << std::setw(3) << std::setfill('0') << (maxSeq + 1);
        return oss.str();
    };

    std::string memberID = generateNextMemberID(isAdmin);

    // 生成数据
    std::string registrationDate = DateUtils::getCurrentDate();
    std::string expiryDate = DateUtils::addDays(registrationDate, 365); // 1 year membership

    // 对密码进行哈希处理
    std::string passwordHash = auth::hashPassword(password);

    // 获取允许的最大书籍数量
    Config& config = Config::getInstance();
    int maxBooksAllowed = config.getDefaultMaxBooks();

    Member* newMember = new Member(
        memberID,
        name,
        phone,
        preferences,
        registrationDate,
        expiryDate,
        maxBooksAllowed,
        isAdmin,
        passwordHash
    );

    if (memberManager.addMember(*newMember)) {
        displayMessage("会员添加成功!", "success");
        std::cout << "\n会员详情:\n";
        std::cout << "  会员 ID:        " << memberID << "\n";
        std::cout << "  姓名:             " << name << "\n";
        std::cout << "  类型:             " << (isAdmin ? "管理员" : "普通用户") << "\n";
        std::cout << "  注册:     " << registrationDate << "\n";
        std::cout << "  到期:           " << expiryDate << "\n";
        std::cout << "  最大可借书数量:        " << maxBooksAllowed << "\n";
        std::cout << "  偏好:      ";
        if (preferences.empty()) {
            std::cout << "None\n";
        } else {
            for (size_t i = 0; i < preferences.size(); i++) {
                std::cout << preferences[i];
                if (i < preferences.size() - 1) std::cout << ", ";
            }
            std::cout << "\n";
        }
    } else {
        displayMessage("会员添加失败", "error");
        delete newMember;
    }

    pauseScreen();
}

void MenuHandler::handleUpdateMember() {
    clearScreen();
    ui.displayHeader("更新会员信息");

    std::string memberID = promptForInput("输入会员 ID 以更新: ");
    if (memberID.empty()) return;

    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr) {
        displayMessage("查无此人", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nCurrent Information:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "会员 ID: " << member->getMemberID() << "\n";
    std::cout << "姓名:      " << member->getName() << "\n";
    std::cout << "手机号码:     " << member->getPhoneNumber() << "\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << "输入新值 (按下 Enter 以保留当前值):\n\n";

    // 获取新值或保留现有值
    std::string newName = promptForInput("Name [" + member->getName() + "]: ");
    std::string name = newName.empty() ? member->getName() : newName;

    std::string newPhone = promptForInput("Phone [" + member->getPhoneNumber() + "]: ");
    std::string phone = newPhone.empty() ? member->getPhoneNumber() : newPhone;


    Member updatedMember(
        member->getMemberID(),
        name,
        phone,
        member->getPreference(),
        member->getRegistrationDate(),
        member->getExpiryDate(),
        member->getMaxBooksAllowed(),
        member->getAdmin(),
        member->getPasswordHash()
    );

    if (memberManager.updateMember(updatedMember)) {
        displayMessage("会员更新成功!", "success");
    } else {
        displayMessage("会员更新失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteMember() {
    clearScreen();
    ui.displayHeader("删除会员");

    std::string memberId = promptForInput("输入会员 ID 以删除: ");
    if (memberId.empty()) return;

    Member* member = memberManager.findMemberByID(memberId);
    if (member == nullptr) {
        displayMessage("查无此人", "error");
        pauseScreen();
        return;
    }

    std::cout << "\n会员信息:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "会员 ID: " << member->getMemberID() << "\n";
    std::cout << "姓名:      " << member->getName() << "\n";
    std::cout << "手机号码:     " << member->getPhoneNumber() << "\n";
    std::cout << std::string(50, '-') << "\n\n";

    if (!confirmAction("您确定要删除这位会员吗?")) {
        return;
    }

    if (memberManager.deleteMember(memberId)) {
        displayMessage("会员删除成功!", "success");
    } else {
        displayMessage("会员删除失败, 会员可能仍持有有效的交易", "error");
    }

    pauseScreen();
}

void MenuHandler::handleViewAllMembers() {
    clearScreen();
    ui.displayHeader("所有会员");

    auto allMembers = memberManager.getAllMembers();

    if (allMembers.empty()) {
        displayMessage("查无会员", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << std::left << std::setw(12) << "会员 ID"
              << std::setw(25) << "姓名"
              << std::setw(15) << "手机号码"
              << std::setw(10) << "类型"
              << std::setw(8) << "状态" << "\n";
    std::cout << std::string(100, '=') << "\n";

    for (const auto& member : allMembers) {
        std::cout << std::left << std::setw(12) << member.getMemberID()
                  << std::setw(25) << member.getName().substr(0, 23)
                  << std::setw(15) << member.getPhoneNumber()
                  << std::setw(10) << (member.getAdmin() ? "管理员" : "会员")
                  << std::setw(8) << (member.isExpired() ? "已过期" : "有效") << "\n";
    }
    std::cout << std::string(100, '=') << "\n";
    std::cout << "会员总数: " << allMembers.size() << "\n";
    std::cout << std::string(100, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewAllTransactions() {
    clearScreen();
    ui.displayHeader("所有交易");

    auto allTransactions = transactionManager.getAllTransactions();

    if (allTransactions.empty()) {
        displayMessage("查无交易", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "交易 ID"
              << std::setw(12) << "会员 ID"
              << std::setw(15) << "ISBN"
              << std::setw(15) << "借阅日期"
              << std::setw(15) << "应还日期"
              << std::setw(15) << "归还日期"
              << std::setw(10) << "状态"
              << std::setw(8) << "罚款" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& transaction : allTransactions) {
        std::cout << std::left << std::setw(15) << transaction.getTransactionID()
                  << std::setw(12) << transaction.getUserID()
                  << std::setw(15) << transaction.getISBN()
                  << std::setw(15) << transaction.getBorrowDate()
                  << std::setw(15) << transaction.getDueDate()
                  << std::setw(15) << (transaction.getReturnDate().empty() ? "N/A" : transaction.getReturnDate())
                  << std::setw(10) << (transaction.haveReturned() ? "已归还" : "活跃")
                  << "$" << std::setw(7) << std::fixed << std::setprecision(2) << transaction.getFine() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "交易总数: " << allTransactions.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewActiveTransactions() {
    clearScreen();
    ui.displayHeader("活跃交易");

    auto activeTransactions = transactionManager.findActiveTransactions();
    if (activeTransactions.empty()) {
        displayMessage("查无活跃交易", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(110, '=') << "\n";
    std::cout << std::left << std::setw(12) << "会员 ID"
              << std::setw(20) << "会员姓名"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(15) << "借阅日期"
              << std::setw(15) << "应还日期"
              << std::setw(8) << "罚款" << "\n";
    std::cout << std::string(110, '=') << "\n";

    for (const auto& transaction : activeTransactions) {
        Member* memberByTransaction = memberManager.findMemberByID(transaction->getUserID());
        Book* book = bookManager.findBookByISBN(transaction->getISBN());

        std::string memberName = memberByTransaction ? memberByTransaction->getName().substr(0, 18) : "未知";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";

        std::cout << std::left << std::setw(12) << transaction->getUserID()
                  << std::setw(20) << memberName
                  << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(15) << transaction->getBorrowDate()
                  << std::setw(15) << transaction->getDueDate()
                  << "$" << std::setw(7) << std::fixed << std::setprecision(2) << transaction->calculateFine() << "\n";
    }
    std::cout << std::string(110, '=') << "\n";
    std::cout << "活跃交易总数: " << activeTransactions.size() << "\n";
    std::cout << std::string(110, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleManualReturn() {
    clearScreen();
    ui.displayHeader("手动归还");

    std::string memberID = promptForInput("输入会员 ID: ");
    if (memberID.empty()) return;

    std::string isbn = promptForInput("输入 ISBN: ");
    if (isbn.empty()) return;

    if (confirmAction("开始为 " + memberID + " 归还图书 " + isbn + "?")) {
        if (transactionManager.returnBook(memberID, isbn)) {
            displayMessage("图书归还成功", "success");
        } else {
            displayMessage("处理还书失败, 请核实详情", "error");
        }
    }

    pauseScreen();
}

void MenuHandler::handleGenerateInventoryReport() {
    clearScreen();
    ui.displayHeader("生成库存报告");

    std::cout << "\n生成库存报告中...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateInventoryReport(true)) {
        displayMessage("库存报告生成成功!", "success");
        std::cout << "\n✓ 报告保存至: " << Config::REPORTS_DIR << "\n";
        std::cout << "  检查报表目录以获取详细的库存报告\n";
    } else {
        displayMessage("库存报告生成失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleGenerateMemberReport() {
    clearScreen();
    ui.displayHeader("生成会员报告");

    std::cout << "\n生成会员报告...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateMemberReport(true)) {
        displayMessage("会员报告生成成功!", "success");
        std::cout << "\n✓ 报告保存至: " << Config::REPORTS_DIR << "\n";
        std::cout << "  检查报表目录以获取详细的库存报告\n";
    } else {
        displayMessage("会员报告生成失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleGenerateTransactionReport() {
    clearScreen();
    ui.displayHeader("生成交易报告");

    std::cout << "\n生成交易报告中...\n";

    int topN = promptForInt("要包含多少个最新交易? (10-100): ", 10, 100);
    if (topN == -1) return;

    std::cout << "\n生成最新的 " << topN << " 个交易中...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateTransactionReport(topN, true)) {
        displayMessage("交易报告生成成功!", "success");
        std::cout << "\n✓ 报告保存至: " << Config::REPORTS_DIR << "\n";
        std::cout << "  包含了最新的 " << topN << " 个交易\n";
    } else {
        displayMessage("交易报告生成失败", "error");
    }

    pauseScreen();
}


void MenuHandler::handleGenerateStatisticsReport() {
    clearScreen();
    ui.displayHeader("生成统计报告");

    std::cout << "\n生成综合统计报告中...\n";
    std::cout << "   这包括所有报告: 摘要, 库存, 会员，\n";
    std::cout << "   交易, 预约, 以及热书.\n\n";

    int topN = promptForInt("热书数量 (10-50): ", 10, 50);
    if (topN == -1) return;

    std::cout << "\n生成所有报告中...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateAllReports(topN, true)) {
        displayMessage("所有统计报告生成成功!", "success");
        std::cout << "\n✓ 报告保存至: " << Config::REPORTS_DIR << "\n";
        std::cout << "\n生成的报告:\n";
        std::cout << "  • 摘要报告\n";
        std::cout << "  • 库存报告\n";
        std::cout << "  • 会员报告\n";
        std::cout << "  • 交易报告 (top " << topN << ")\n";
        std::cout << "  • 预订报告\n";
        std::cout << "  • 热书报告 (top " << topN << ")\n";
    } else {
        displayMessage("生成部分或全部报告失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleBackupData() {
    clearScreen();
    ui.displayHeader("备份系统数据");

    std::string description = promptForInput("输入备份描述 (可选): ");
    if (description.empty()) {
        description = "手动备份";
    }

    std::cout << "\n创建备份中...\n";
    std::cout << "   描述: " << description << "\n\n";

    BackupManager backupManager;

    if (backupManager.backupData(description)) {
        BackupInfo latestBackup = backupManager.getLatestBackup();

        displayMessage("备份创建成功!", "success");
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════╗\n";
        std::cout << "║                   备份完成                         ║\n";
        std::cout << "╠════════════════════════════════════════════════╣\n";
        std::cout << "║  备份 ID:   " << std::left << std::setw(33) << latestBackup.backupID << "║\n";
        std::cout << "║  时间:        " << std::left << std::setw(33) << latestBackup.backupTime << "║\n";
        std::cout << "║  描述: " << std::left << std::setw(33) << latestBackup.description.substr(0, 32) << "║\n";
        std::cout << "║  状态:      " << std::left << std::setw(35) << (latestBackup.isValid ? "✓ Valid" : "✗ Invalid") << "║\n";
        std::cout << "╚════════════════════════════════════════════════╝\n";
    } else {
        displayMessage("备份创建失败, 请检查上面的错误信息", "error");
    }

    pauseScreen();
}

void MenuHandler::handleRestoreData() {
    clearScreen();
    ui.displayHeader("恢复系统数据");

    BackupManager backupManager;

    if (!backupManager.hasValidBackups()) {
        displayMessage("没有可用的有效备份进行恢复", "warning");
        pauseScreen();
        return;
    }

    auto backups = backupManager.listBackups();

    std::cout << "\n可用备份:\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(5) << "#"
              << std::setw(25) << "备份 ID"
              << std::setw(22) << "时间"
              << std::setw(40) << "描述"
              << std::setw(10) << "状态" << "\n";
    std::cout << std::string(120, '=') << "\n";

    int index = 1;
    for (const auto& backup : backups) {
        std::cout << std::left << std::setw(5) << index++
                  << std::setw(25) << backup.backupID
                  << std::setw(22) << backup.backupTime
                  << std::setw(40) << backup.description.substr(0, 38)
                  << std::setw(10) << (backup.isValid ? "✓ 有效" : "✗ 无效") << "\n";
    }
    std::cout << std::string(120, '=') << "\n\n";

    std::cout << "警 告: 恢复将 覆 盖 所有当前数据!\n";
    std::cout << "   在恢复之前, 请确保备份当前数据\n\n";

    if (!confirmAction("您想继续进行恢复吗?")) {
        return;
    }

    std::string backupId = promptForInput("输入备份 ID 以恢复: ");
    if (backupId.empty()) return;

    // 验证备份是否存在且有效
    BackupInfo selectedBackup = backupManager.getBackupInfo(backupId);
    if (selectedBackup.backupID.empty()) {
        displayMessage("备份 ID 未找到", "error");
        pauseScreen();
        return;
    }

    if (!selectedBackup.isValid) {
        displayMessage("所选的备份无效或已损坏", "error");
        pauseScreen();
        return;
    }

    std::cout << "\n恢复备份: " << selectedBackup.description << "\n";
    std::cout << "从: " << selectedBackup.backupTime << "\n\n";

    if (!confirmAction("最终确认 - 恢复这项备份?")) {
        return;
    }

    std::cout << "\n恢复数据中...\n";

    if (backupManager.restoreData(backupId)) {
        displayMessage("数据恢复成功!", "success");
        std::cout << "重要: 请重启应用程序以使更改生效\n\n";
    } else {
        displayMessage("数据恢复失败, 请检查上方的错误信息", "error");
    }

    pauseScreen();
}

void MenuHandler::handleChangeUIMode() {
    clearScreen();
    ui.displayHeader("变更 UI 模式");

    Config& config = Config::getInstance();

    std::cout << "\n当前 UI 模式: " << (config.isAdvancedUIMode() ? "高级" : "简单") << "\n\n";

    std::cout << "可选模式:\n";
    std::cout << "  1. 简单模式  - 基本菜单和简约界面\n";
    std::cout << "  2. 高级模式 - 增强界面功能更多\n";
    std::cout << "  0. 取消\n\n";

    int choice = promptForInt("选择 UI 模式: ", 0, 2);

    if (choice == 0) return;

    bool newMode = (choice == 2);

    if (newMode == config.isAdvancedUIMode()) {
        displayMessage("UI 模式已被设定为这个选项", "info");
        pauseScreen();
        return;
    }

    config.setAdvancedUIMode(newMode);
    config.saveSettings();

    displayMessage("UI 模式改变成功!", "success");
    std::cout << "新模式: " << (newMode ? "高级" : "简单") << "\n";
    std::cout << "注意: 某些更改可能需要重启应用程序\n\n";

    pauseScreen();
}

void MenuHandler::handleChangeBorrowPeriod() {
    clearScreen();
    ui.displayHeader("变更借阅期限");

    Config& config = Config::getInstance();

    std::cout << "\n当前借阅期限: " << config.getBorrowPeriodDays() << " 天\n\n";

    std::cout << "输入新的借阅期限 (7-90 天): ";
    int newPeriod = promptForInt("天数: ", 7, 90);

    if (newPeriod == -1) return;  // 用户取消

    if (newPeriod == config.getBorrowPeriodDays()) {
        displayMessage("借阅周期已被设定为这个值", "info");
        pauseScreen();
        return;
    }

    if (!confirmAction("变更借阅期限至 " + std::to_string(newPeriod) + " 天?")) {
        return;
    }

    config.setBorrowPeriodDays(newPeriod);
    config.saveSettings();

    displayMessage("借阅期限变更成功!", "success");
    std::cout << "新期限: " << newPeriod << " 天\n";
    std::cout << "注意: 这只影响新的借阅, 现有借阅不变\n\n";

    pauseScreen();
}

void MenuHandler::handleChangeFineSettings() {
    clearScreen();
    ui.displayHeader("变更罚款设置");

    Config& config = Config::getInstance();

    std::cout << "\n当前罚款设置:\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << "每日罚款: $" << std::fixed << std::setprecision(2) << config.getFinePerDay() << "\n";
    std::cout << "最大罚款: $" << std::fixed << std::setprecision(2) << config.getMaxFine() << "\n";
    std::cout << std::string(40, '-') << "\n\n";

    std::cout << "What would you like to change?\n";
    std::cout << "  1. 每日罚款\n";
    std::cout << "  2. 最大罚款\n";
    std::cout << "  3. 二者\n";
    std::cout << "  0. 取消\n\n";

    int choice = promptForInt("输入选项: ", 0, 3);

    if (choice == 0) return;

    double newFinePerDay = config.getFinePerDay();
    double newMaxFine = config.getMaxFine();

    if (choice == 1 || choice == 3) {
        newFinePerDay = promptForDouble("输入新的每日罚款 ($0.00-$50.00): ");
        if (newFinePerDay < 0 || newFinePerDay > 50.0) {
            displayMessage("罚款金额无效, 必须在 $0.00 到 $50.00 之间", "error");
            pauseScreen();
            return;
        }
    }

    if (choice == 2 || choice == 3) {
        newMaxFine = promptForDouble("输入新的最大罚款 ($0.00-$500.00): ");
        if (newMaxFine < 0 || newMaxFine > 500.0) {
            displayMessage("罚款金额无效, 必须在 $0.00 到 $500.00 之间", "error");
            pauseScreen();
            return;
        }
    }

    // 验证: 最高罚款应大于等于每日罚款
    if (newMaxFine < newFinePerDay) {
        displayMessage("最高罚款不得低于每日罚款", "error");
        pauseScreen();
        return;
    }

    std::cout << "\n新设置:\n";
    std::cout << "  每日罚款: $" << std::fixed << std::setprecision(2) << newFinePerDay << "\n";
    std::cout << "  最大罚款: $" << std::fixed << std::setprecision(2) << newMaxFine << "\n\n";

    if (!confirmAction("应用这些罚款设置?")) {
        return;
    }

    config.setFinePerDay(newFinePerDay);
    config.setMaxFine(newMaxFine);
    config.saveSettings();

    displayMessage("罚款设置更新成功!", "success");
    std::cout << "注意: 新的罚款将适用于未来的逾期计算\n\n";

    pauseScreen();
}

// 预订管理处理器

void MenuHandler::handleManageReservations() {
    clearScreen();
    ui.displayHeader("管理预订");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. 查看所有预订                              │\n";
    std::cout << "│  2. 查看有效预订                              │\n";
    std::cout << "│  3. 添加预订                                │\n";
    std::cout << "│  4. 更新预订                                │\n";
    std::cout << "│  5. 取消预订                                │\n";
    std::cout << "│  6. 搜索预订                                │\n";
    std::cout << "│  0. 回退                                  │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 6);

    switch (choice) {
        case 1: handleViewAllReservation(); break;
        case 2: handleViewActiveReservation(); break;
        case 3: handleAddReservation(); break;
        case 4: handleUpdateReservation(); break;
        case 5: handleDeleteReservation(); break;
        case 6: handleSearchReservations(); break;
        case 0: return;
        default: displayMessage("无效选择", "error"); pauseScreen(); break;
    }
}

void MenuHandler::handleViewAllReservation() {
    clearScreen();
    ui.displayHeader("所有预订");

    const auto& allReservations = reservationManager.getAllReservations();

    if (allReservations.empty()) {
        displayMessage("未找到预订", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "预订 ID"
              << std::setw(12) << "会员 ID"
              << std::setw(20) << "会员姓名"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(18) << "预约日期"
              << std::setw(10) << "状态" << "\n";
    std::cout << std::string(120, '=') << "\n";

    int activeCount = 0;
    for (const auto& reservation : allReservations) {
        Member* member = memberManager.findMemberByID(reservation.getMemberID());
        Book* book = bookManager.findBookByISBN(reservation.getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "未知";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";

        if (reservation.getIsActive()) {
            activeCount++;
        }

        std::cout << std::left << std::setw(15) << reservation.getReservationID()
                  << std::setw(12) << reservation.getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation.getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation.getReservationDate()
                  << std::setw(10) << (reservation.getIsActive() ? "活跃" : "已取消") << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "预订总数: " << allReservations.size()
              << " (活跃: " << activeCount << ", 已取消: " << (allReservations.size() - activeCount) << ")\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewActiveReservation() {
    clearScreen();
    ui.displayHeader("有效预约");

    auto activeReservations = reservationManager.findActiveReservations();

    if (activeReservations.empty()) {
        displayMessage("未找到有效预约", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "预订 ID"
              << std::setw(12) << "会员 ID"
              << std::setw(20) << "会员姓名"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(18) << "预约日期";
    std::cout << std::string(120, '=') << "\n";

    for (const auto* reservation : activeReservations) {
        Member* member = memberManager.findMemberByID(reservation->getMemberID());
        Book* book = bookManager.findBookByISBN(reservation->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "未知";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(12) << reservation->getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate();
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "有效预约总数: " << activeReservations.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleAddReservation() {
    clearScreen();
    ui.displayHeader("添加预约");

    std::string memberID = promptForInput("输入 会员 ID: ");
    if (memberID.empty()) return;

    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr) {
        displayMessage("查无此人", "error");
        pauseScreen();
        return;
    }

    std::cout << "\n会员: " << member->getName() << " (" << memberID << ")\n\n";

    std::string isbn = promptForInput("输入 ISBN: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("查无此书", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    // Check for existing active reservation
    auto existingReservations = reservationManager.findByMemberID(memberID);
    for (const auto* reservation : existingReservations) {
        if (reservation->getISBN() == isbn && reservation->getIsActive()) {
            displayMessage("该会员已经为这本书有一个有效的预约", "error");
            pauseScreen();
            return;
        }
    }

    if (!confirmAction("为该会员创建预订?")) {
        return;
    }

    std::string reservationID = reservationManager.reserveBook(memberID, isbn);

    if (reservationID == "0") {
        displayMessage("创建预订失败", "error");
    } else {
        displayMessage("预订创建成功! ID: " + reservationID, "success");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteReservation() {
    clearScreen();
    ui.displayHeader("取消预约");

    std::string reservationID = promptForInput("输入预订 ID 以取消: ");
    if (reservationID.empty()) return;

    Reservation* reservation = reservationManager.findByReservationID(reservationID);
    if (reservation == nullptr) {
        displayMessage("预订未找到", "error");
        pauseScreen();
        return;
    }

    // 显示预订详细信息
    Member* member = memberManager.findMemberByID(reservation->getMemberID());
    Book* book = bookManager.findBookByISBN(reservation->getISBN());

    std::cout << "\n预订详细信息:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << "预订 ID:   " << reservation->getReservationID() << "\n";
    std::cout << "会员:           " << (member ? member->getName() : "未知")
              << " (" << reservation->getMemberID() << ")\n";
    std::cout << "图书:             " << (book ? book->getTitle() : "未知") << "\n";
    std::cout << "ISBN:             " << reservation->getISBN() << "\n";
    std::cout << "预约日期: " << reservation->getReservationDate() << "\n";
    std::cout << "状态:           " << (reservation->getIsActive() ? "活跃" : "已取消") << "\n";
    std::cout << std::string(60, '-') << "\n\n";

    if (!reservation->getIsActive()) {
        displayMessage("该预订已被取消", "warning");
        pauseScreen();
        return;
    }

    if (!confirmAction("取消预订1?")) {
        return;
    }

    std::string result = reservationManager.cancelReservation(reservationID);

    if (result != "0") {
        displayMessage("预订取消成功!", "success");
    } else {
        displayMessage("预订取消失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleUpdateReservation() {
    clearScreen();
    ui.displayHeader("更新预订");

    std::string reservationID = promptForInput("输入预订 ID 以更新: ");
    if (reservationID.empty()) return;

    Reservation* reservation = reservationManager.findByReservationID(reservationID);
    if (reservation == nullptr) {
        displayMessage("未找到预订", "error");
        pauseScreen();
        return;
    }

    // Display current reservation details
    Member* member = memberManager.findMemberByID(reservation->getMemberID());
    Book* book = bookManager.findBookByISBN(reservation->getISBN());

    std::cout << "\n";
    std::cout << "当前预订详细信息:\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "预订 ID:   " << reservation->getReservationID() << "\n";
    std::cout << "会员:           " << (member ? member->getName() : "未知")
              << " (" << reservation->getMemberID() << ")\n";
    std::cout << "图书:             " << (book ? book->getTitle() : "未知") << "\n";
    std::cout << "ISBN:             " << reservation->getISBN() << "\n";
    std::cout << "预约日期: " << reservation->getReservationDate() << "\n";
    std::cout << "状态:           " << (reservation->getIsActive() ? "活跃" : "已取消") << "\n";

    // 显示队列位置若有效
    if (reservation->getIsActive()) {
        int position = reservationManager.getQueuePosition(reservationID);
        int queueLength = reservationManager.getQueueLength(reservation->getISBN());
        std::cout << "队列位置:   " << position << " / " << queueLength << "\n";
    }
    std::cout << std::string(60, '=') << "\n\n";

    // 更新选项
    std::cout << "您想要如何更新?\n";
    std::cout << "  1. 变更会员 ID\n";
    std::cout << "  2. 变更图书 (ISBN)\n";
    std::cout << "  3. 切换状态 (有效/已取消)\n";
    std::cout << "  0. 取消\n\n";

    int choice = promptForInt("输入选项: ", 0, 3);

    if (choice == 0) return;

    std::string newMemberID = reservation->getMemberID();
    std::string newISBN = reservation->getISBN();
    bool newIsActive = reservation->getIsActive();

    switch (choice) {
        case 1: {
            // 变更会员
            std::string inputMemberID = promptForInput("输入新会员 ID: ");
            if (inputMemberID.empty()) return;

            Member* newMember = memberManager.findMemberByID(inputMemberID);
            if (newMember == nullptr) {
                displayMessage("查无此人", "error");
                pauseScreen();
                return;
            }

            newMemberID = inputMemberID;
            std::cout << "\n✓ 将会将会员改变为: " << newMember->getName() << "\n";
            break;
        }
        case 2: {
            // Change book
            std::string inputISBN = promptForInput("输入新ISBN: ");
            if (inputISBN.empty()) return;

            Book* newBook = bookManager.findBookByISBN(inputISBN);
            if (newBook == nullptr) {
                displayMessage("查无此书", "error");
                pauseScreen();
                return;
            }

            newISBN = inputISBN;
            std::cout << "\n✓ 将会将图书改变为: " << newBook->getTitle() << "\n";
            break;
        }
        case 3: {
            // Toggle status
            newIsActive = !reservation->getIsActive();
            std::cout << "\n✓ 将会将状态改变为: " << (newIsActive ? "活跃" : "已取消") << "\n";
            break;
        }
        default:
            return;
    }

    if (!confirmAction("\n应用这些变更?")) {
        return;
    }

    // 创建更新的预订 (预订不可更改)
    Reservation updatedReservation(
        reservation->getReservationID(),
        newMemberID,
        newISBN,
        reservation->getReservationDate(),
        newIsActive
    );

    if (reservationManager.updateReservation(updatedReservation)) {
        displayMessage("预订更新成功!", "success");

        // Show what changed
        std::cout << "\n已应用变更:\n";
        if (newMemberID != reservation->getMemberID()) {
            std::cout << "  • 会员 ID: " << reservation->getMemberID() << " → " << newMemberID << "\n";
        }
        if (newISBN != reservation->getISBN()) {
            std::cout << "  • ISBN: " << reservation->getISBN() << " → " << newISBN << "\n";
        }
        if (newIsActive != reservation->getIsActive()) {
            std::cout << "  • 状态: " << (reservation->getIsActive() ? "活跃" : "已取消")
                      << " → " << (newIsActive ? "活跃" : "已取消") << "\n";
        }
    } else {
        displayMessage("预订更新失败", "error");
    }

    pauseScreen();
}

void MenuHandler::handleSearchReservations() {
    clearScreen();
    ui.displayHeader("搜索预订");

    std::cout << "\n搜索项:\n";
    std::cout << "  1. 预订 ID\n";
    std::cout << "  2. 会员 ID\n";
    std::cout << "  3. ISBN\n";
    std::cout << "  4. 预约日期\n";
    std::cout << "  0. 回退  \n\n";

    int choice = promptForInt("请输入您的选择: ", 0, 4);

    std::vector<const Reservation*> results;

    switch (choice) {
        case 1: {
            std::string reservationID = promptForInput("输入 预订 ID: ");
            if (!reservationID.empty()) {
                Reservation* res = reservationManager.findByReservationID(reservationID);
                if (res != nullptr) {
                    results.push_back(res);
                }
            }
            break;
        }
        case 2: {
            std::string memberId = promptForInput("输入会员 ID: ");
            if (!memberId.empty()) {
                results = reservationManager.findByMemberID(memberId);
            }
            break;
        }
        case 3: {
            std::string isbn = promptForInput("输入 ISBN: ");
            if (!isbn.empty()) {
                results = reservationManager.findByISBN(isbn);
            }
            break;
        }
        case 4: {
            std::string date = promptForInput("输入预约日期 (YYYY-MM-DD): ");
            if (!date.empty()) {
                results = reservationManager.findByReservationDate(date);
            }
            break;
        }
        case 0:
            return;
        default:
            displayMessage("无效选择", "error");
            pauseScreen();
            return;
    }

    if (results.empty()) {
        displayMessage("未找到预订", "info");
        pauseScreen();
        return;
    }

    clearScreen();
    ui.displayHeader("搜索结果");

    std::cout << "\n找到 " << results.size() << " 个预订:\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "预订 ID"
              << std::setw(12) << "会员 ID"
              << std::setw(20) << "会员姓名"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "书名"
              << std::setw(18) << "预约日期"
              << std::setw(10) << "状态" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto* reservation : results) {
        Member* member = memberManager.findMemberByID(reservation->getMemberID());
        Book* book = bookManager.findBookByISBN(reservation->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "未知";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "未知";

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(12) << reservation->getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate()
                  << std::setw(10) << (reservation->getIsActive() ? "活跃" : "已取消") << "\n";
    }
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

// 实用函数

std::string MenuHandler::promptForInput(const std::string& prompt, bool canCancel) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);

    // 修剪空白
    input.erase(0, input.find_first_not_of(" \t\n\r"));
    input.erase(input.find_last_not_of(" \t\n\r") + 1);

    if (canCancel && (input == "q" || input == "Q" || input == "cancel")) {
        return "";
    }

    return input;
}

int MenuHandler::promptForInt(const std::string& prompt, int min, int max, bool canCancel) {
    while (true) {
        std::string input = promptForInput(prompt, canCancel);

        if (input.empty() && canCancel) {
            return -1;
        }

        try {
            int value = std::stoi(input);
            if (value >= min && value <= max) {
                return value;
            }
            displayMessage("请输入一个介于 " + std::to_string(min) +
                         " 和 " + std::to_string(max) + "的数", "error");
        } catch (...) {
            displayMessage("输入无效, 请输入一个有效的数字", "error");
        }
    }
}

double MenuHandler::promptForDouble(const std::string& prompt, bool canCancel) {
    while (true) {
        std::string input = promptForInput(prompt, canCancel);

        if (input.empty() && canCancel) {
            return -1.0;
        }

        try {
            double value = std::stod(input);
            if (value >= 0) {
                return value;
            }
            displayMessage("请输入一个正数", "error");
        } catch (...) {
            displayMessage("输入无效, 请输入一个有效的数字", "error");
        }
    }
}

bool MenuHandler::confirmAction(const std::string& message) {
    std::cout << "\n" << message << " (y/n): ";
    std::string input;
    std::getline(std::cin, input);

    return (input == "y" || input == "Y" || input == "yes" || input == "Yes" || input == "YES");
}

void MenuHandler::displayMessage(const std::string& message, const std::string& type) {
    std::cout << "\n";
    if (type == "success") {
        std::cout << "SUCCESS: " << message << "\n";
    } else if (type == "error") {
        std::cout << "ERROR: " << message << "\n";
    } else if (type == "warning") {
        std::cout << "WARNING: " << message << "\n";
    } else {
        std::cout << "INFO: " << message << "\n";
    }
    std::cout << "\n";
}

void MenuHandler::pauseScreen() {
    std::cout << "\n按下 Enter 以继续...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');     // 丢弃缓冲区中的所有字符
    // std::cin.get();
}

void MenuHandler::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// 搜索助手函数

void MenuHandler::performBookSearch(const std::string& searchType) {
    std::string query = promptForInput("输入搜索项: ");
    if (query.empty()) return;

    std::vector<const Book*> results;

    if (searchType == "title") {
        results = bookManager.findByTitle(query);
    } else if (searchType == "author") {
        results = bookManager.findByAuthor(query);
    } else if (searchType == "isbn") {
        Book* book = bookManager.findBookByISBN(query);
        if (book != nullptr) {
            results.push_back(book);
        }
    } else if (searchType == "genre") {
        results = bookManager.findByGenre(query);
    } else if (searchType == "publisher") {
        results = bookManager.findByPublisher(query);
    }

    displaySearchResults(results);
}

void MenuHandler::displaySearchResults(const std::vector<const class Book*>& results) {
    clearScreen();
    ui.displayHeader("搜索结果");

    if (results.empty()) {
        displayMessage("未找到符合您搜索条件的书籍", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n找到了 " << results.size() << " 本书:\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "书名"
              << std::setw(25) << "作者"
              << std::setw(20) << "类型"
              << std::setw(10) << "可用" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& book : results) {
        std::cout << std::left << std::setw(15) << book->getISBN()
                  << std::setw(35) << book->getTitle().substr(0, 33)
                  << std::setw(25) << book->getAuthor().substr(0, 23)
                  << std::setw(20) << book->getGenre().substr(0, 18)
                  << std::setw(10) << book->getAvailableCopies() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";

    std::cout << "\n输入 ISBN 以查看详细信息 (或按下 Enter 以回退): ";
    std::string isbn;
    std::getline(std::cin, isbn);

    if (!isbn.empty()) {
        Book* selectedBook = bookManager.findBookByISBN(isbn);
        if (selectedBook != nullptr) {
            displayBookDetails(selectedBook);
        }
    }
}

void MenuHandler::displayBookDetails(Book* book) {
    clearScreen();
    ui.displayHeader("图书详细信息");

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ISBN:           " << book->getISBN() << "\n";
    std::cout << "书名:          " << book->getTitle() << "\n";
    std::cout << "作者:         " << book->getAuthor() << "\n";
    std::cout << "出版社:      " << book->getPublisher() << "\n";
    std::cout << "类型:       " << book->getGenre() << "\n";
    std::cout << "总数量:   " << book->getTotalCopies() << "\n";
    std::cout << "可用:      " << book->getAvailableCopies() << "\n";
    std::cout << "状态:         " << (book->canBorrow() ? "可用" : "不可用") << "\n";

    // 显示预订队列信息
    int queueLength = reservationManager.getQueueLength(book->getISBN());
    if (queueLength > 0) {
        std::cout << "\n--- 预订 队列 ---\n";
        std::cout << "等待人数: " << queueLength << "\n";

        if (currentUser && !currentUser->getAdmin()) {
            auto userReservations = reservationManager.findByMemberID(currentUser->getMemberID());
            for (const auto* res : userReservations) {
                if (res->getISBN() == book->getISBN() && res->getIsActive()) {
                    int position = reservationManager.getQueuePosition(res->getReservationID());
                    std::cout << "您的位置:  " << position;
                    if (position == 1) {
                        std::cout << " (下一位!)";
                    }
                    std::cout << "\n";
                    break;
                }
            }
        }
    }

    std::cout << std::string(60, '=') << "\n";

    pauseScreen();
}
