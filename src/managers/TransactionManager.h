#ifndef LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H

#include "../models/Transaction.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>

// 前向声明
class MemberManager;
class BookManager;

class TransactionManager {
private:
    std::vector<Transaction> transactions;
    std::string filePath;
    FileHandler fileHandler;

    // 数据持久化
    void loadFromFile();
    void saveToFile();

    // 用于批量操作
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // 助手: 检查自动保存标志决定是否需要保存
    void saveIfNeeded();

    // 助手: 生成交易 ID
    std::string generateTransactionID() const;

    // 助手: 对一位会员计算其活跃交易数
    int getActiveCountForMember(const std::string& memberID) const;

public:
    // 构造函数
    explicit TransactionManager(const std::string& filePath = "../data/transactions.csv");

    // CRUD 操作 (增删查改)
    bool addTransaction(const Transaction& transaction);
    bool updateTransaction(const Transaction& transaction);
    bool deleteTransaction(const std::string& transactionID);

    // 搜索函数
    Transaction* findByTransactionID(const std::string& transactionID);
    std::vector<const Transaction*> findByMemberID(const std::string& memberID);
    std::vector<const Transaction*> findByISBN(const std::string& isbn);
    std::vector<const Transaction*> findByBorrowDate(const std::string& borrowDate);
    std::vector<const Transaction*> findByDueDate(const std::string& dueDate);
    std::vector<const Transaction*> findByReturnDate(const std::string& returnDate);
    std::vector<const Transaction*> findActiveTransactions();
    std::vector<const Transaction*> findOverdueTransactions();

    // 交易函数
    std::string borrowBook(const std::string& memberID, const std::string& isbn);
    std::string borrowBook(MemberManager& memberManager, BookManager& bookManager,
                           const std::string& memberID, const std::string& isbn);
    bool returnBook(const std::string& transactionID);
    bool returnBook(BookManager& bookManager, const std::string& transactionID);
    bool returnBook(const std::string& memberID, const std::string& isbn);
    bool returnBook(BookManager& bookManager, const std::string& memberID, const std::string& isbn);
    bool renewBook(const std::string& transactionID);
    bool renewBook(const std::string& memberID, const std::string& isbn);

    // 历史 & 报告
    std::vector<const Transaction*> getMemberHistory(const std::string& memberID);
    std::vector<const Transaction*> getActiveTransactions(const std::string& memberID);
    std::vector<const Transaction*> getOverdueTransactions();

    // 获取器
    const std::vector<Transaction>& getAllTransactions() const;
    int getTotalTransactions() const;
    int getActiveTransactionsCount() const;
    int getOverdueTransactionsCount() const;

    // 实用方法
    void reload();          // 重新加载文件
    void clearCache();      // 清除文件处理器缓存
    bool isTransactionIDExists(const std::string& transactionID) const;

    // 批量操作 (RAII)
    class BatchOperation {
    private:
        TransactionManager* transactionManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(TransactionManager& tmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H
