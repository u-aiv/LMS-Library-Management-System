#ifndef LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H
#define LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H

#include <string>

class Reservation {
private:
    std::string reservationID;          // 预约 ID
    std::string memberID;               // 预约者 ID
    std::string isbn;                   // 被预约书 ISBN
    std::string reservationDate;        // 预约日期
    bool isActive{};                    // 预约有效标记

public:
    // 构造函数
    Reservation(std::string reservationID, std::string memberID, std::string isbn,
                std::string reservationDate, bool isActive = true) :
                reservationID(std::move(reservationID)), memberID(std::move(memberID)),
                isbn(std::move(isbn)), reservationDate(std::move(reservationDate)),
                isActive(isActive){}
    Reservation() = default;

    // 获取器和设置器
    std::string getReservationID() const;
    std::string getMemberID() const;
    std::string getISBN() const;
    std::string getReservationDate() const;
    bool getIsActive() const;

    // 业务逻辑
    void cancelReservation();

    // 实用方法
    std::string toCSV() const;
    static Reservation fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H
