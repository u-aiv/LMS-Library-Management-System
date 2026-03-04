// Reservation.h 实现

#include "Reservation.h"
#include <sstream>

std::string Reservation::getReservationID() const {
    return reservationID;
}
std::string Reservation::getMemberID() const {
    return memberID;
}
std::string Reservation::getISBN() const {
    return isbn;
}
std::string Reservation::getReservationDate() const {
    return reservationDate;
}
bool Reservation::getIsActive() const {
    return isActive;
}

void Reservation::cancelReservation() {
    if (!isActive) return;
    isActive = false;
}

std::string Reservation::toCSV() const {
    return reservationID + "," +
    memberID + "," +
    isbn + "," +
    reservationDate + "," +
    (isActive ? "1" : "0");
}

Reservation Reservation::fromCSV(const std::string &csvLine) {
    Reservation reservation;
    std::istringstream iss(csvLine);

    std::getline(iss, reservation.reservationID, ',');
    std::getline(iss, reservation.memberID, ',');
    std::getline(iss, reservation.isbn, ',');
    std::getline(iss, reservation.reservationDate, ',');

    int boolInt;
    iss >> boolInt;
    reservation.isActive = (boolInt != 0);

    return reservation;
}
