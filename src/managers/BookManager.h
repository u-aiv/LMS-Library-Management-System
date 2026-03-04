#ifndef LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H

#include "../models/Book.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>

class BookManager {
private:
    std::vector<Book> books;
    std::string filePath;
    FileHandler fileHandler;

    // 数据持久化
    // 助手：从文件中加载书籍数据
    void loadFromFile();

    // 助手：向文件中保存书籍数据
    void saveToFile();

    // 用于批量操作
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // 助手：检查自动保存标志决定是否需要保存
    void saveIfNeeded();


public:
    // 构造函数
    explicit BookManager(const std::string& filePath = "../data/books.csv");

    // CRUD 操作 (增删查改)
    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(const std::string& isbn);

    // 搜索函数
    Book* findBookByISBN(const std::string &isbn);
    std::vector<const Book*> findByTitle(const std::string& title, int matchMode = 0) const;
    std::vector<const Book*> findByAuthor(const std::string& author, int matchMode = 0) const;
    std::vector<const Book*> findByPublisher(const std::string& publisher, int matchMode = 0) const;
    std::vector<const Book*> findByGenre(const std::string& genre, int matchMode = 0) const;
    std::vector<const Book*> findAvailableBooks() const;

    // 借/还操作
    bool borrowBook(const std::string& isbn);
    bool returnBook(const std::string& isbn);

    // 获取器
    std::vector<Book> getAllBooks() const;
    int getTotalBooks() const;
    int getAvailableCount() const;
    // std::vector<Book*> getAvailableBooks() const;

    // 实用方法
    void reload();          // 重新加载文件
    void clearCache();      // 清除文件处理器缓存
    bool isISBNExists(const std::string& isbn) const;

    // 批量操作 (RAII)
    class BatchOperation {
    private:
        BookManager* bookManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(BookManager& bmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H
