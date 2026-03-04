// ReportManager.cpp 实现

#include "ReportManager.h"
#include "../utils/DateUtils.h"
#include "../utils/FileHandler.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <utility>

// 构造函数
ReportManager::ReportManager(const std::string& bookPath,
                             const std::string& memberPath,
                             const std::string& transactionPath,
                             const std::string& reservationPath,
                             const std::string& reportsDirectory)
    : bookManager(bookPath),
      memberManager(memberPath),
      transactionManager(transactionPath),
      reservationManager(reservationPath),
      reportsDir(reportsDirectory) {
    reloadAll();
}

// 重新加载所有数据
void ReportManager::reloadAll() {
    bookManager.reload();
    memberManager.reload();
    transactionManager.reload();
    reservationManager.reload();
}

// 拼接路径
std::string ReportManager::joinPath(const std::string& dir, const std::string& file) const {
    if (dir.empty()) {
        return file;
    }

    // 处理 "/" 和 "\"
    std::string result = dir;
    if (result.back() != '/' && result.back() != '\\') {
        result += '/';
    }
    return result + file;
}

// 构造报告文件名
std::string ReportManager::buildReportFileName(const std::string& prefix) const {
    // 以时间戳生成文件名: prefix_YYYYMMDD_HHMMSS.txt
    time_t currentTime = DateUtils::getCurrentTimestamp();
    std::string currentDate = DateUtils::getCurrentDate();
    tm * timeInfo = localtime(&currentTime);

    std::ostringstream oss;
    oss << prefix << "_"
        << std::setfill('0') << currentDate << "_"
        << std::setw(2) << timeInfo->tm_hour
        << std::setw(2) << timeInfo->tm_min
        << std::setw(2) << timeInfo->tm_sec
        << ".txt";

    return oss.str();
}

// 写入 CSV 行
bool ReportManager::writeLines(const std::string& filePath, const std::vector<std::string>& lines) const {
    try {
        FileHandler fileHandler;
        fileHandler.writeCSV(filePath, lines);
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("写入报告失败" + std::string(e.what()));
    }
}

std::vector<std::string> ReportManager::buildSummaryReport() const {
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  总   结   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("报告生成: " + currentDate);
    lines.emplace_back("");

    // 书目统计
    lines.emplace_back("--- 书目统计 --- ");
    lines.emplace_back("馆藏书总数: " + std::to_string(bookManager.getTotalBooks()));
    lines.emplace_back("可用数: " + std::to_string(bookManager.getAvailableCount()));
    lines.emplace_back("借出数: " + std::to_string(bookManager.getTotalBooks() - bookManager.getAvailableCount()));
    lines.emplace_back("");

    // 会员统计
    lines.emplace_back("--- 会员统计 ---");
    lines.emplace_back("馆内会员总数: " + std::to_string(memberManager.getTotalMembers()));
    lines.emplace_back("管理员数: " + std::to_string(memberManager.getAdminCount()));
    lines.emplace_back("常规用户: " + std::to_string(memberManager.getTotalMembers() - memberManager.getAdminCount()));
    lines.emplace_back("");

    // 交易统计
    lines.emplace_back("--- 交易统计 ---");
    lines.emplace_back("馆内交易总数: " + std::to_string(transactionManager.getTotalTransactions()));
    lines.emplace_back("有效交易数: " + std::to_string(transactionManager.getActiveTransactionsCount()));
    lines.emplace_back("逾期交易数: " + std::to_string(transactionManager.getOverdueTransactionsCount()));
    lines.emplace_back("");

    // 预约统计
    lines.emplace_back("--- 预约统计 ---");
    lines.emplace_back("总预约数: " + std::to_string(reservationManager.getTotalReservations()));
    lines.emplace_back("有效预约数: " + std::to_string(reservationManager.getActiveReservations()));
    lines.emplace_back("");

    lines.emplace_back("================================================");

    return lines;
}

std::vector<std::string> ReportManager::buildInventoryReport() const{
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  库   存   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("报告生成: " + currentDate);
    lines.emplace_back("");

    // 表头
    lines.emplace_back("ISBN          | Title                    | Author          | Total | Available");
    lines.emplace_back("--------------|--------------------------|-----------------|-------|----------|");

    // 书籍详细信息
    auto allBooks = bookManager.getAllBooks();
    for (const auto& book : allBooks) {
        std::string truncatedTitle = book.getTitle();
        if (truncatedTitle.length() > 24) {
            truncatedTitle = truncatedTitle.substr(0, 21) + "...";
        }

        std::ostringstream oss;
        oss << std::left
            << std::setw(14) << book.getISBN() << "|"
            << std::setw(26) << truncatedTitle << "|"
            << std::setw(17) << book.getAuthor().substr(0, 16) << "|"
            << std::setw(7) << book.getTotalCopies() << "|"
            << std::setw(10) << book.getAvailableCopies() << "|";
        lines.emplace_back(oss.str());
    }

    lines.emplace_back("");
    lines.emplace_back("总数: " + std::to_string(bookManager.getTotalBooks()));
    lines.emplace_back("可用数: " + std::to_string(bookManager.getAvailableCount()));
    lines.emplace_back("");
    lines.emplace_back("================================================");

    return lines;
}

std::vector<std::string> ReportManager::buildMemberReport() const {
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  会   员   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("报告生成: " + currentDate);
    lines.emplace_back("");

    // 表头
    lines.emplace_back("Member ID | Name                 | Phone Number | Registration Date | Expiry Date");
    lines.emplace_back("----------|----------------------|--------------|-------------------|-----------");

    // 会员信息
    auto allMembers = memberManager.getAllMembers();
    for (const auto& member : allMembers) {
        std::ostringstream oss;
        oss << std::left
            << std::setw(10) << member.getMemberID() << "| "
            << std::setw(21) << member.getName() << "| "
            << std::setw(13) << member.getPhoneNumber() << "| "
            << std::setw(18) << member.getRegistrationDate() << "| "
            << member.getExpiryDate();
        lines.emplace_back(oss.str());
    }

    lines.emplace_back("");
    lines.emplace_back("总数: " + std::to_string(memberManager.getTotalMembers()));
    lines.emplace_back("管理员数: " + std::to_string(memberManager.getAdminCount()));
    lines.emplace_back("");
    lines.emplace_back("================================================");

    return lines;
}

std::vector<std::string> ReportManager::buildTransactionReport(int topN) const {
    if (topN <= 0){
        topN = 10;
    }
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  交   易   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("报告生成: " + currentDate);
    lines.emplace_back("");

    // 表头
    lines.emplace_back("Transaction ID | Member ID |     ISBN      | Borrow Date | Due Date   | Returned | Fine");
    lines.emplace_back("---------------|-----------|---------------|-------------|------------|----------|-----");

    // 交易信息
    auto allTransactions = transactionManager.getAllTransactions();

    // 按借阅日期（降序）部分排序以获取最近的交易
    std::vector<const Transaction*> transactionPtrs;
    transactionPtrs.reserve(allTransactions.size());
    for (const auto& trans : allTransactions) {
        transactionPtrs.emplace_back(&trans);
    }

    if (topN < transactionPtrs.size()){
        std::partial_sort(transactionPtrs.begin(),
                          transactionPtrs.begin() + topN,
                          transactionPtrs.end(),
                          [](const Transaction* a, const Transaction* b){
                            return a->getBorrowDate() > b->getBorrowDate();
                          });
        transactionPtrs.resize(topN);
    } else {
        std::sort(transactionPtrs.begin(), transactionPtrs.end(),
                  [](const Transaction* a, const Transaction* b) {
        return a->getBorrowDate() > b->getBorrowDate();
    });
    }

    int count = 0;
    for (const auto* trans : transactionPtrs) {
        if (count >= topN) break;

        std::ostringstream oss;
        oss << std::left
            << std::setw(15) << trans->getTransactionID() << "| "
            << std::setw(10) << trans->getUserID() << "| "
            << std::setw(14) << trans->getISBN() << "| "
            << std::setw(12) << trans->getBorrowDate() << "| "
            << std::setw(11) << trans->getDueDate() << "| "
            << std::setw(9) << (trans->haveReturned() ? "Yes" : "No") << "| "
            << std::fixed << std::setprecision(2) << trans->getFine();
        lines.emplace_back(oss.str());
        count++;
    }

    lines.emplace_back("");
    lines.emplace_back("总交易数: " + std::to_string(transactionManager.getTotalTransactions()));
    lines.emplace_back("活动交易数: " + std::to_string(transactionManager.getActiveTransactionsCount()));
    lines.emplace_back("逾期交易数: " + std::to_string(transactionManager.getOverdueTransactionsCount()));
    lines.emplace_back("");
    lines.emplace_back("================================================");

    return lines;
}

std::vector<std::string> ReportManager::buildReservationReport() const {
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  预   约   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("Report Generated: " + currentDate);
    lines.emplace_back("");

    // 表头
    lines.emplace_back("Reservation ID | Member ID |     ISBN     | Reservation Date | Status");
    lines.emplace_back("---------------|-----------|--------------|------------------|--------");

    // 预约信息
    auto allReservations = reservationManager.getAllReservations();
    for (const auto& res : allReservations) {
        std::ostringstream oss;
        oss << std::left
            << std::setw(15) << res.getReservationID() << "| "
            << std::setw(10) << res.getMemberID() << "| "
            << std::setw(14) << res.getISBN() << "| "
            << std::setw(17) << res.getReservationDate() << "| "
            << (res.getIsActive() ? "Active" : "Cancelled");
        lines.emplace_back(oss.str());
    }
    
    lines.emplace_back("");
    lines.emplace_back("总预约数: " + std::to_string(reservationManager.getTotalReservations()));
    lines.emplace_back("有效预约数: " + std::to_string(reservationManager.getActiveReservations()));
    lines.emplace_back("");
    lines.emplace_back("================================================");
    
    return lines;
}

std::vector<std::string> ReportManager::buildTopBorrowedBooksReport(int topN) const {
    if (topN <= 0){
        topN = 10;
    }
    std::vector<std::string> lines;

    // 标题
    lines.emplace_back("================================================");
    lines.emplace_back("                  图书馆管理系统报告                     ");
    lines.emplace_back("                  热   书   报   告                 ");
    lines.emplace_back("================================================");
    lines.emplace_back("");

    // 日期
    std::string currentDate = DateUtils::getCurrentDate();
    lines.emplace_back("Report Generated: " + currentDate);
    lines.emplace_back("");

    // 按 ISBN 统计借阅频率
    std::unordered_map<std::string, int> borrowCount;
    auto allTransactions = transactionManager.getAllTransactions();

    for (const auto& transaction : allTransactions) {
        borrowCount[transaction.getISBN()]++;
    }

    // 转换为向量并排序
    std::vector<std::pair<std::string, int> > sortedBorrows(borrowCount.begin(), borrowCount.end());
    std::sort(sortedBorrows.begin(), sortedBorrows.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });

    // 表头
    lines.emplace_back("Rank | ISBN       | Title                    | Author          | Borrow Count");
    lines.emplace_back("-----|------------|--------------------------|-----------------|-------------");

    int rank = 1;
    for (const auto& pair : sortedBorrows) {
        if (rank > topN) break;

        auto book = const_cast<BookManager&>(bookManager).findBookByISBN(pair.first);
        if (book) {
            // 检查书目标题是否被截断
            std::string truncatedTitle = book->getTitle();
            if (truncatedTitle.length() > 24){
                truncatedTitle = truncatedTitle.substr(0, 21) + "...";
            }
            std::ostringstream oss;
            oss << std::left
                << std::setw(5) << rank << "| "
                << std::setw(11) << book->getISBN() << "| "
                << std::setw(25) << truncatedTitle << "| "
                << std::setw(16) << book->getAuthor().substr(0, 15) << "| "
                << pair.second;
            lines.push_back(oss.str());
            rank++;
        }
    }

    lines.emplace_back("");
    lines.emplace_back("Total Books with Transactions: " + std::to_string(borrowCount.size()));
    lines.emplace_back("");
    lines.emplace_back("================================================");

    return lines;
}

bool ReportManager::generateSummaryReport(bool reload) {
    if (reload) {
        reloadAll();
    }

    std::string fileName = buildReportFileName("SummaryReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildSummaryReport();
    return writeLines(filePath, content);
}

bool ReportManager::generateInventoryReport(bool reload) {
    if (reload) {
        reloadAll();
    }

    std::string fileName = buildReportFileName("InventoryReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildInventoryReport();
    return writeLines(filePath, content);
}
bool ReportManager::generateMemberReport(bool reload) {
    if (reload) {
        reloadAll();
    }
    std::string fileName = buildReportFileName("MemberReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildMemberReport();
    return writeLines(filePath, content);
}

bool ReportManager::generateTransactionReport(int topN, bool reload) {
    if (reload) {
        reloadAll();
    }
    std::string fileName = buildReportFileName("TransactionReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildTransactionReport(topN);
    return writeLines(filePath, content);
}

bool ReportManager::generateReservationReport(bool reload) {
    if (reload) {
        reloadAll();
    }
    std::string fileName = buildReportFileName("ReservationReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildReservationReport();
    return writeLines(filePath, content);
}

bool ReportManager::generateTopBorrowedBooksReport(int topN, bool reload) {
    if (reload) {
        reloadAll();
    }
    std::string fileName = buildReportFileName("TopBorrowedBooksReport");
    std::string filePath = joinPath(reportsDir, fileName);
    std::vector<std::string> content = buildTopBorrowedBooksReport(topN);
    return writeLines(filePath, content);
}

bool ReportManager::generateAllReports(int topN, bool reload) {
    if (reload) {
        reloadAll();
    }

    bool summarySuccess = generateSummaryReport(false);
    bool inventorySuccess = generateInventoryReport(false);
    bool memberSuccess = generateMemberReport(false);
    bool transactionSuccess = generateTransactionReport(topN, false);
    bool reservationSuccess = generateReservationReport(false);
    bool topBooksSuccess = generateTopBorrowedBooksReport(topN, false);

    return summarySuccess && inventorySuccess && memberSuccess &&
    transactionSuccess && reservationSuccess && topBooksSuccess;
}
