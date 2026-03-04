#ifndef LIBRARY_MANAGEMENT_SYSTEM_REPORTMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_REPORTMANAGER_H

#include "BookManager.h"
#include "MemberManager.h"
#include "TransactionManager.h"
#include "ReservationManager.h"
#include <string>
#include <vector>

class ReportManager {
private:
    BookManager bookManager;
    MemberManager memberManager;
    TransactionManager transactionManager;
    ReservationManager reservationManager;
    std::string reportsDir;                 // 报告输出目录

    // 拼接路径
    std::string joinPath(const std::string& dir, const std::string& file) const;
    // 构造报告文件名
    std::string buildReportFileName(const std::string& prefix) const;
    // 写入文本行
    bool writeLines(const std::string& filePath, const std::vector<std::string>& lines) const;

    // 构建器
    std::vector<std::string> buildSummaryReport() const;
    std::vector<std::string> buildInventoryReport() const;
    std::vector<std::string> buildMemberReport() const;
    std::vector<std::string> buildTransactionReport(int topN) const;
    std::vector<std::string> buildReservationReport() const;
    std::vector<std::string> buildTopBorrowedBooksReport(int topN) const;

public:
    explicit ReportManager(
        const std::string& bookPath = "../data/books.csv",
        const std::string& memberPath = "../data/members.csv",
        const std::string& transactionPath = "../data/transactions.csv",
        const std::string& reservationPath = "../data/reservations.csv",
        const std::string& reportsDirectory = "../reports");

    // 重新加载所有数据
    void reloadAll();

    // 生成器
    bool generateSummaryReport(bool reload = true);
    bool generateInventoryReport(bool reload = true);
    bool generateMemberReport(bool reload = true);
    bool generateTransactionReport(int topN = 10, bool reload = true);
    bool generateReservationReport(bool reload = true);
    bool generateTopBorrowedBooksReport(int topN = 10, bool reload = true);
    bool generateAllReports(int topN = 10, bool reload = true);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_REPORTMANAGER_H
