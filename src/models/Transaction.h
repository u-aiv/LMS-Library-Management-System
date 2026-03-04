#ifndef LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H
#define LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H

#include <string>

class Transaction {
private:
    std::string transactionID;      // 交易 ID
    std::string userID;             // 用户 ID
    std::string isbn;               // 书目 ISBN
    std::string borrowDate;         // 借阅日期 (YYYY-MM-DD)
    std::string dueDate;            // 逾期日期 (YYYY-MM-DD)
    std::string returnDate;         // 归还日期 (YYYY-MM-DD)
    int renewCount{};               // 续约次数
    double fine{};                  // 罚款金额
    bool isReturned{};              // true = 已归还, false = 未归还

public:
    // 构造函数
    Transaction(std::string transactionID, std::string userID, std::string isbn,
                std::string borrowDate, std::string dueDate, std::string returnDate,
                int renewCount = 0, double fine = 0.0, bool isReturned = false):
                transactionID(std::move(transactionID)), userID(std::move(userID)),
                isbn(std::move(isbn)), borrowDate(std::move(borrowDate)),
                dueDate(std::move(dueDate)), returnDate(std::move(returnDate)),
                renewCount(renewCount), fine(fine), isReturned(isReturned){}
    Transaction() = default;

    // 获取器和设置器
    std::string getTransactionID() const;
    std::string getUserID() const;
    std::string getISBN() const;
    std::string getBorrowDate() const;
    std::string getDueDate() const;
    std::string getReturnDate() const;
    int getRenewCount() const;
    double getFine() const;
    bool haveReturned() const;

    // 业务逻辑
    bool isOverdue() const;
    double calculateFine() const;
    bool canRenew() const;
    void renewBook();
    void returnBook();

    // 实用方法
    std::string toCSV() const;
    static Transaction fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H
