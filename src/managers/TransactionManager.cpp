// TransactionManager.cpp 实现

#include "TransactionManager.h"
#include "BookManager.h"
#include "MemberManager.h"
#include "../config/Config.h"
#include "../utils/DateUtils.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

// 构造函数
TransactionManager::TransactionManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "TransactionID,MemberID,ISBN,BorrowDate,DueDate,ReturnDate,RenewCount,Fine,IsReturned"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// 私有: 助手: 从文件加载交易数据
void TransactionManager::loadFromFile() {
    transactions.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // 跳过表头(第一行)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                transactions.push_back(Transaction::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load transactions file: " + std::string(e.what()));
    }
}

// 私有: 助手: 将交易数据保存到文件
void TransactionManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("TransactionID,MemberID,ISBN,BorrowDate,DueDate,ReturnDate,RenewCount,Fine,IsReturned");

    // 添加所有交易
    for (const auto& transaction : transactions) {
        lines.push_back(transaction.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save transactions file: " + std::string(e.what()));
    }
}

// 私有: 辅助: 设置自动保存标志
void TransactionManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool TransactionManager::isAutoSaveEnabled() const {
    return autoSave;
}

// 助手: 检查自动保存标志决定是否需要保存
void TransactionManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// 私有: 辅助: 生成交易 ID
std::string TransactionManager::generateTransactionID() const {
    std::string currentDate = DateUtils::getCurrentDate();
    std::string year = currentDate.substr(0, 4);
    int month = std::stoi(currentDate.substr(5, 2));
    int season = (month - 1) / 3 + 1;

    std::string prefix = "T" + year + std::to_string(season);
    int maxSeq = 0;
    for (const auto& transaction : transactions) {
        const std::string& id = transaction.getTransactionID();
        if (id.rfind(prefix, 0) != 0) {
            continue;
        }
        if (id.size() < prefix.size() + 5) {
            continue;
        }

        std::string suffix = id.substr(prefix.size());
        bool allDigits = std::all_of(suffix.begin(), suffix.end(),
            [](unsigned char c) { return std::isdigit(c); });
        if (!allDigits) {
            continue;
        }
        int seq = std::stoi(suffix);
        if (seq > maxSeq) {
            maxSeq = seq;
        }
    }

    std::ostringstream oss;
    oss << prefix << std::setw(5) << std::setfill('0') << (maxSeq + 1);
    return oss.str();
}

int TransactionManager::getActiveCountForMember(const std::string& memberID) const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID && !transaction.haveReturned()) {
            count++;
        }
    }
    return count;
}

// 新增一条交易
bool TransactionManager::addTransaction(const Transaction& transaction) {
    // 检查交易ID是否已存在
    if (isTransactionIDExists(transaction.getTransactionID())) {
        return false;
    }
    transactions.push_back(transaction);
    saveIfNeeded();
    return true;
}

// 更新现有交易
bool TransactionManager::updateTransaction(const Transaction& transaction) {
    Transaction *existingTransaction = findByTransactionID(transaction.getTransactionID());

    if (existingTransaction == nullptr) {
        return false;
    }
    *existingTransaction = transaction;
    saveIfNeeded();
    return true;
}

// 以交易 ID 删除交易
bool TransactionManager::deleteTransaction(const std::string& transactionID) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&](const Transaction& trans) { return trans.getTransactionID() == transactionID; });

    if (it != transactions.end()) {
        transactions.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// 以交易 ID 查找交易
Transaction* TransactionManager::findByTransactionID(const std::string& transactionID) {
    for (auto& transaction : transactions) {
        if (transaction.getTransactionID() == transactionID) {
            return &transaction;
        }
    }
    return nullptr;
}

// 以会员 ID 查找交易
std::vector<const Transaction*> TransactionManager::findByMemberID(const std::string& memberID) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 以 ISBN 查找交易
std::vector<const Transaction*> TransactionManager::findByISBN(const std::string& isbn) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getISBN() == isbn) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 以借出日查找交易
std::vector<const Transaction*> TransactionManager::findByBorrowDate(const std::string& borrowDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getBorrowDate() == borrowDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 以到期日查找交易
std::vector<const Transaction*> TransactionManager::findByDueDate(const std::string& dueDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getDueDate() == dueDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 以归还日查找交易
std::vector<const Transaction*> TransactionManager::findByReturnDate(const std::string& returnDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getReturnDate() == returnDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 查找有效交易
std::vector<const Transaction*> TransactionManager::findActiveTransactions() {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 查找逾期交易
std::vector<const Transaction*> TransactionManager::findOverdueTransactions() {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned() && transaction.isOverdue()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 借一本书
std::string TransactionManager::borrowBook(const std::string& memberID, const std::string& isbn) {
    MemberManager memberManager(Config::MEMBERS_FILE);
    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr || member->isExpired()) {
        return "0";
    }
    if (getActiveCountForMember(memberID) >= member->getMaxBooksAllowed()) {
        return "0";
    }

    BookManager bookManager(Config::BOOKS_FILE);
    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr || !book->canBorrow()) {
        return "0";
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string dueDate = DateUtils::addDays(currentDate, 14);
    std::string transactionID = generateTransactionID();

    Transaction transaction(transactionID, memberID, isbn, currentDate, dueDate, "", 0, 0.0, false);
    if (!addTransaction(transaction)) {
        return "0";
    }

    if (!bookManager.borrowBook(isbn)) {
        deleteTransaction(transactionID);
        return "0";
    }

    return transactionID;
}

std::string TransactionManager::borrowBook(MemberManager& memberManager, BookManager& bookManager,
                                           const std::string& memberID, const std::string& isbn) {
    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr || member->isExpired()) {
        return "0";
    }
    if (getActiveCountForMember(memberID) >= member->getMaxBooksAllowed()) {
        return "0";
    }

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr || !book->canBorrow()) {
        return "0";
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string dueDate = DateUtils::addDays(currentDate, 14);
    std::string transactionID = generateTransactionID();

    Transaction transaction(transactionID, memberID, isbn, currentDate, dueDate, "", 0, 0.0, false);
    if (!addTransaction(transaction)) {
        return "0";
    }

    if (!bookManager.borrowBook(isbn)) {
        deleteTransaction(transactionID);
        return "0";
    }

    return transactionID;
}

// 以交易 ID 归还一本书
bool TransactionManager::returnBook(const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    BookManager bookManager(Config::BOOKS_FILE);
    if (!bookManager.returnBook(transaction->getISBN())) {
        return false;
    }

    transaction->returnBook();
    saveIfNeeded();
    return true;
}

bool TransactionManager::returnBook(BookManager& bookManager, const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    if (!bookManager.returnBook(transaction->getISBN())) {
        return false;
    }

    transaction->returnBook();
    saveIfNeeded();
    return true;
}

// 以会员 ID 和 ISBN 归还一本书
bool TransactionManager::returnBook(const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
        }
    }

    if (transaction == nullptr) {
        return false;
    }

    return returnBook(transaction->getTransactionID());
}

bool TransactionManager::returnBook(BookManager& bookManager, const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
        }
    }

    if (transaction == nullptr) {
        return false;
    }

    return returnBook(bookManager, transaction->getTransactionID());
}

// 以交易 ID 续约一本书
bool TransactionManager::renewBook(const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    if (!transaction->canRenew()) {
        return false;
    }

    transaction->renewBook();
    saveIfNeeded();
    return true;
}

bool TransactionManager::renewBook(const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
             }
    }

    if (transaction == nullptr) {
        return false;
    }

    return renewBook(transaction->getTransactionID());
}

// 获取会员交易历史
std::vector<const Transaction*> TransactionManager::getMemberHistory(const std::string& memberID) {
    return findByMemberID(memberID);
}

// 获取一位会员的有效交易
std::vector<const Transaction*> TransactionManager::getActiveTransactions(const std::string& memberID) {
    std::vector<const Transaction*> results;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID && !transaction.haveReturned()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// 获取所有逾期交易
std::vector<const Transaction*> TransactionManager::getOverdueTransactions() {
    return findOverdueTransactions();
}

// 获取所有交易
const std::vector<Transaction>& TransactionManager::getAllTransactions() const {
    return transactions;
}

// 获取交易总数
int TransactionManager::getTotalTransactions() const {
    return static_cast<int>(transactions.size());
}

// 获取活跃交易数
int TransactionManager::getActiveTransactionsCount() const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned()) {
            count++;
        }
    }
    return count;
}

// 获取逾期交易数
int TransactionManager::getOverdueTransactionsCount() const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned() && transaction.isOverdue()) {
            count++;
        }
    }
    return count;
}

// 重新加载文件
void TransactionManager::reload() {
    loadFromFile();
}

// 清除文件处理器缓存
void TransactionManager::clearCache() {
    fileHandler.clearCache();
}

// 检查交易 ID 是否存在
bool TransactionManager::isTransactionIDExists(const std::string& transactionID) const {
    return std::find_if(transactions.begin(), transactions.end(),
        [&](const Transaction& transaction) { return transaction.getTransactionID() == transactionID; }) != transactions.end();
}

// 批量操作 (RAII)
TransactionManager::BatchOperation::BatchOperation(TransactionManager& tmgr) :
                    transactionManager(&tmgr), originalAutoSave(tmgr.autoSave), active(true) {
    tmgr.setAutoSave(false);
}

TransactionManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    transactionManager(other.transactionManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

TransactionManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        transactionManager->saveToFile();
        transactionManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "在批量操作期间尝试保存交易时出错" << std::endl;
    }
}

TransactionManager::BatchOperation TransactionManager::beginBatch() {
    return BatchOperation(*this);
}
