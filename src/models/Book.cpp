// Book.h 实现

#include "Book.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string Book::getISBN() const {
    return isbn;
}
std::string Book::getTitle() const {
    return title;
}
std::string Book::getAuthor() const {
    return author;
}
std::string Book::getPublisher() const {
    return publisher;
}
std::string Book::getGenre() const {
    return genre;
}
int Book::getTotalCopies() const {
    return totalCopies;
}
int Book::getAvailableCopies() const {
    return availableCopies;
}
bool Book::canBorrow() const {
    return !isReserved && availableCopies > 0;
}

void Book::borrowBook() {
    if (availableCopies <= 0) {
        std::cout << "错误：没有可借阅的副本!" << std::endl;
        return;
    }
    availableCopies--;
}

void Book::returnBook() {
    if (availableCopies >= totalCopies) {
        std::cout << "错误：归还的书籍数量不能超过总库存!" << std::endl;
        return;
    }
    availableCopies++;
}

void Book::setReserved(bool status){
    isReserved = status;
}

std::string Book::toCSV() const {
    return getISBN() + "," +
    getTitle() + "," +
    getAuthor() + "," +
    getPublisher() + "," +
    getGenre() + "," +
    std::to_string(getTotalCopies()) + "," +
    std::to_string(getAvailableCopies()) + "," +
    std::to_string(isReserved ? 1 : 0);
}

Book Book::fromCSV(const std::string& csvLine) {
    Book book;
    std::stringstream ss(csvLine);
    getline(ss, book.isbn, ',');
    getline(ss, book.title, ',');
    getline(ss, book.author, ',');
    getline(ss, book.publisher, ',');
    getline(ss, book.genre, ',');
    ss >> book.totalCopies;
    ss.ignore(1);
    ss >> book.availableCopies;
    ss.ignore(1);

    int boolInt;
    ss >> boolInt;
    book.isReserved = (boolInt != 0);

    return book;
}
