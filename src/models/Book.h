#ifndef LIBRARY_MANAGEMENT_SYSTEM_BOOK_H
#define LIBRARY_MANAGEMENT_SYSTEM_BOOK_H

#include <string>
#include <utility>

class Book {
private:
    std::string isbn;                // ISBN 码, 键
    std::string title;               // 书名
    std::string author;              // 作者
    std::string publisher;           // 出版社
    std::string genre;               // 类别

    int totalCopies{};                // 图书总册数
    int availableCopies{};            // 图书有效册数
    bool isReserved{};                // true = 已被预订, false = 未被预订

public:
    // 构造函数
    Book(std::string isbn, std::string title, std::string author, std::string publisher, std::string genre,
     int totalCopies = 0, int availableCopies = 0, bool isReserved = false)
    : isbn(std::move(isbn)), title(std::move(title)), author(std::move(author)), publisher(std::move(publisher)),
      genre(std::move(genre)), totalCopies(totalCopies), availableCopies(availableCopies), isReserved(isReserved) {}
    Book() = default;

    // 获取器和设置器
    std::string getISBN() const;
    std::string getTitle() const;
    std::string getPublisher() const;
    std::string getAuthor() const;
    std::string getGenre() const;
    int getTotalCopies() const;
    int getAvailableCopies() const;
    bool canBorrow() const;

    void borrowBook();
    void returnBook();
    void setReserved(bool status);

    // CSV 函数
    std::string toCSV() const;
    static Book fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BOOK_H
