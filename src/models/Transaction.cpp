// Transaction.h 实现

#include "Transaction.h"
#include "../utils/DateUtils.h"
#include "../config/Config.h"
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

std::string Transaction::getTransactionID() const {
    return transactionID;
}
std::string Transaction::getUserID() const {
    return userID;
}
std::string Transaction::getISBN() const {
    return isbn;
}
std::string Transaction::getBorrowDate() const {
    return borrowDate;
}
std::string Transaction::getDueDate() const {
    return dueDate;
}
std::string Transaction::getReturnDate() const {
    return returnDate;
}
int Transaction::getRenewCount() const {
    return renewCount;
}
double Transaction::getFine() const {
    return fine;
}
bool Transaction::haveReturned() const {
    return isReturned;
}

bool Transaction::isOverdue() const {
    time_t currentDateTimestamp = std::time(nullptr);
    time_t dueDateTimestamp = DateUtils::dateToTimestamp(dueDate);
    return currentDateTimestamp > dueDateTimestamp;
}

double Transaction::calculateFine() const {
    if (!Transaction::isOverdue()) {
        return 0.0;
    }
    unsigned int increment = DateUtils::daysBetween(DateUtils::dateToTimestamp(dueDate),
                                                    DateUtils::getCurrentTimestamp());
    double amount = increment * (Config::getInstance().getFinePerDay());
    double maxFine = Config::getInstance().getMaxFine();
    if (amount > maxFine) amount = maxFine;
    return amount;
}

bool Transaction::canRenew() const {
    if (isReturned) return false;
    unsigned int totalDays = DateUtils::daysBetween(DateUtils::dateToTimestamp(borrowDate),
                                                     DateUtils::dateToTimestamp(dueDate));

    return  (totalDays + 7 <= 30);
}

void Transaction::renewBook() {
    if (!canRenew()) return;
    dueDate = DateUtils::addDays(dueDate, 7);
    renewCount++;
}

void Transaction::returnBook() {
    if (isReturned) return;
    fine = calculateFine();
    returnDate = DateUtils::getCurrentDate();
    isReturned = true;
    if (fine > 0) {
        std::cout << "您已逾期! 请支付罚款: $" << fine << std::endl;
    }
}

std::string Transaction::toCSV() const {
    return transactionID + "," +
           userID + "," +
           isbn + "," +
           borrowDate + "," +
           dueDate + "," +
           returnDate + "," +
           std::to_string(renewCount) + "," +
           std::to_string(fine) + "," +
           (isReturned ? "1" : "0");
}

Transaction Transaction::fromCSV(const std::string& csvLine) {
    Transaction transaction;
    std::istringstream iss(csvLine);

    std::getline(iss, transaction.transactionID, ',');
    std::getline(iss, transaction.userID, ',');
    std::getline(iss, transaction.isbn, ',');
    std::getline(iss, transaction.borrowDate, ',');
    std::getline(iss, transaction.dueDate, ',');
    std::getline(iss, transaction.returnDate, ',');

    iss >> transaction.renewCount;
    iss.ignore(1);
    iss >> transaction.fine;
    iss.ignore(1);

    int boolInt;
    iss >> boolInt;
    transaction.isReturned = (boolInt != 0);

    return transaction;
}
