#ifndef LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H

#include <string>
#include <vector>

// 前向声明
class BookManager;
class MemberManager;
class TransactionManager;
class ReservationManager;
class RecommendationManager;
class BackupManager;
class ReportManager;
class Member;
class UI;

class MenuHandler {
private:
    // 管理器引用
    BookManager& bookManager;
    MemberManager& memberManager;
    TransactionManager& transactionManager;
    ReservationManager& reservationManager;
    RecommendationManager& recommendationManager;
    UI& ui;

    // 当前用户会话
    Member* currentUser;
    bool isRunning;

    // 菜单显示助手
    void displayWelcomeScreen();
    void displayMemberMenu();
    void displayAdminMenu();

    // 会员菜单处理器
    void handleSearchBooks();
    void handleBorrowBook();
    void handleReturnBook();
    void handleRenewBook();
    void handleReserveBook();
    void handleViewBorrowedBooks();
    void handleViewBorrowingHistory();
    void handleViewReservations();
    void handleViewProfile();

    // 管理员菜单处理器
    void handleManageBooks();
    void handleManageMembers();
    void handleManageTransactions();
    void handleViewOverdueBooks();
    void handleManageReservations();
    void handleGenerateReports();
    void handleBackupRestore();
    void handleSystemSettings();

    // 管理员子菜单处理 - 书籍
    void handleAddBook();
    void handleUpdateBook();
    void handleDeleteBook();
    void handleViewAllBooks();

    // 管理员子菜单处理 - 会员
    void handleAddMember();
    void handleUpdateMember();
    void handleSearchReservations();
    void handleDeleteMember();
    void handleViewAllMembers();

    // 管理员子菜单处理 - 交易
    void handleViewAllTransactions();
    void handleViewActiveTransactions();
    void handleManualReturn();

    // 管理员子菜单处理 - 预约
    void handleAddReservation();
    void handleUpdateReservation();
    void handleDeleteReservation();
    void handleViewAllReservation();
    void handleViewActiveReservation();

    // 管理员子菜单处理 - 推荐
    void handleViewRecommendations();

    // 管理员子菜单处理 - 报告
    void handleGenerateInventoryReport();
    void handleGenerateMemberReport();
    void handleGenerateTransactionReport();
    void handleGenerateOverdueReport();
    void handleGenerateStatisticsReport();

    // 管理员子菜单处理 - 备份/恢复
    void handleBackupData();
    void handleRestoreData();

    // 管理员子菜单处理 - 系统设置
    void handleChangeUIMode();
    void handleChangeBorrowPeriod();
    void handleChangeFineSettings();

    // 实用方法
    std::string promptForInput(const std::string& prompt, bool canCancel = true);
    int promptForInt(const std::string& prompt, int min, int max, bool canCancel = true);
    double promptForDouble(const std::string& prompt, bool canCancel = true);
    bool confirmAction(const std::string& message);
    void displayMessage(const std::string& message, const std::string& type = "info");
    void pauseScreen();
    void clearScreen();

    // 搜索助手
    void performBookSearch(const std::string& searchType);
    void displaySearchResults(const std::vector<const class Book*>& results);
    void displayBookDetails(Book* book);

public:
    // 构造函数
    MenuHandler(BookManager& bm, MemberManager& mm,
                TransactionManager& tm, ReservationManager& rsm,
                RecommendationManager& rcm, UI& ui);

    // 析构函数
    ~MenuHandler();

    bool login();
    bool logout();
    void run();
    void exit();
    bool isApplicationRunning() const;

    // 获取当前登录用户
    Member* getCurrentUser() const;
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H
