// BookManager.h 实现

#include "BookManager.h"
#include "../utils/FileHandler.h"
#include <algorithm>
#include <iostream>
#include <utility>

// 构造函数
BookManager::BookManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "ISBN,Title,Author,Publisher,Genre,TotalCopies,AvailableCopies,IsReserved"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// 私有：助手：从文件加载书籍数据
void BookManager::loadFromFile() {
    books.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                books.push_back(Book::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load books file: " + std::string(e.what()));
    }
}

// 私有：助手：向文件保存书籍数据
void BookManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("ISBN,Title,Author,Publisher,Genre,TotalCopies,AvailableCopies,IsReserved");

    // 添加全部书目
    for (const auto& book : books) {
        lines.push_back(book.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save books file: " + std::string(e.what()));
    }
}

// 私有：辅助：检查自动保存标志决定是否需要保存
void BookManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// 新增一本书
bool BookManager::addBook(const Book& book) {
    // 检查 ISBN 是否已存在
    if (isISBNExists(book.getISBN())) {
        return false;
    }
    books.push_back(book);
    saveIfNeeded();
    return true;
}

// 以 ISBN 删除一本书
bool BookManager::deleteBook(const std::string& isbn) {
    auto it = std::find_if(books.begin(), books.end(),
        [&](const Book& book) { return book.getISBN() == isbn; });

    if (it != books.end()) {
        books.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// 更新现有图书
bool BookManager::updateBook(const Book& book) {
    Book *existingBook = findBookByISBN(book.getISBN());

    if (existingBook == nullptr) {
        return false;
    }
    *existingBook = book;
    saveIfNeeded();
    return true;
}

// 以 ISBN 查找一本书
Book* BookManager::findBookByISBN(const std::string &isbn) {
    for (auto& book : books) {
        if (book.getISBN() == isbn) {
            return &book;
        }
    }
    return nullptr;
}

// 书籍查找模板
// matchMode = 0 --> 精确查找 (区分大小写) (默认)
// matchMode = 1 --> 模糊查找 (统一大小写)
template<typename Getter>
std::vector<const Book*> findByField(
    const std::vector<Book>& books,
    const std::string& key,
    Getter getter,
    int matchMode = 0
    ) {
        if (matchMode != 0 && matchMode != 1) {
            throw std::runtime_error("无效匹配码");
        }

        std::vector<const Book*> results;

        if (matchMode == 0) {
            for (const auto& book : books) {
                if ((book.*getter)() == key) {
                    results.push_back(&book);
                }
            }
        }
        else {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

            for (const auto& book : books) {
                std::string bookKey = (book.*getter)();
                std::transform(bookKey.begin(), bookKey.end(), bookKey.begin(), ::tolower);

                if (bookKey.find(lowerKey) != std::string::npos)
                    results.push_back(&book);
            }
        }
        return results;
}

// 以标题查找书
std::vector<const Book*> BookManager::findByTitle(const std::string& title, int matchMode) const {
    return findByField(books, title,&Book::getTitle , matchMode);
}

// 以作者查找书
std::vector<const Book*> BookManager::findByAuthor(const std::string& author, int matchMode) const {
    return findByField(books, author, &Book::getAuthor, matchMode);
}

// 以出版社查找书
std::vector<const Book*> BookManager::findByPublisher(const std::string& publisher, int matchMode) const {
    return findByField(books, publisher, &Book::getPublisher, matchMode);
}

// 以类型查找书
std::vector<const Book*> BookManager::findByGenre(const std::string& genre, int matchMode) const {
    return findByField(books, genre, &Book::getGenre, matchMode);
}

// 查找可用书目
std::vector<const Book*> BookManager::findAvailableBooks() const {
    std::vector<const Book*> results;

    for (const auto& book : books) {
        if (book.canBorrow()) {
            results.push_back(&book);
        }
    }
    return results;
}

// 借一本书
bool BookManager::borrowBook(const std::string& isbn) {         // -> TransactionManager
    auto* book = findBookByISBN(isbn);

    if (book == nullptr) {
        return false;       // 书未找到
    }

    if (!book->canBorrow()) {
        return false;       // 书不可用
    }

    book->borrowBook();
    saveIfNeeded();
    return true;
}

// 还一本书
bool BookManager::returnBook(const std::string& isbn) {         // -> TransactionManager
    auto* book = findBookByISBN(isbn);

    if (book == nullptr) {
        return false;       // 书未找到
    }

    book->returnBook();
    saveIfNeeded();
    return true;
}

// 获取所有书目
std::vector<Book> BookManager::getAllBooks() const {
    return books;
}

// 获取书目总数量
int BookManager::getTotalBooks() const {
    return static_cast<int>(books.size());
}

// 获取可用书目数量
int BookManager::getAvailableCount() const {
    int count = 0;
    for (const auto& book : books) {
        if (book.canBorrow()) {
            count++;
        }
    }
    return count;
}

// 重新加载文件
void BookManager::reload() {
    loadFromFile();
}

// 清除文件处理者缓存
void BookManager::clearCache() {
    fileHandler.clearCache();
}

// 检查如果 ISBN 存在
bool BookManager::isISBNExists(const std::string& isbn) const {
    return std::find_if(books.begin(), books.end(),
        [&](const Book& book) { return book.getISBN() == isbn; }) != books.end();
}

// 批量操作 (RAII)
void BookManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool BookManager::isAutoSaveEnabled() const {
    return autoSave;
}

BookManager::BatchOperation::BatchOperation(BookManager& bmgr) :
                    bookManager(&bmgr), originalAutoSave(bmgr.autoSave), active(true) {
    bmgr.setAutoSave(false);
}

BookManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    bookManager(other.bookManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

BookManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        bookManager->saveToFile();
        bookManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "在批量操作过程中尝试保存书籍时出错" << std::endl;
    }
}

BookManager::BatchOperation BookManager::beginBatch() {
    return BatchOperation(*this);
}
