#ifndef LIBRARY_MANAGEMENT_SYSTEM_VALIDATOR_H
#define LIBRARY_MANAGEMENT_SYSTEM_VALIDATOR_H

#include <string>
#include <vector>
#include <algorithm>

namespace Validator {
    inline bool isValidISBN(const std::string& isbn) {
        if (isbn.size() == 13)
            return true;
        return false;
    }

    inline bool isValidPhoneNumber(const std::string& phoneNumber) {
        if (phoneNumber.size() == 11)
            return true;
        return false;
    }

    inline bool isValidGenre(const std::string& genre, const std::vector<std::string>& validGenres) {
       if (std::any_of(validGenres.cbegin(), validGenres.cend(),
           [&](const std::string& g) { return g == genre; })) {
            return true;
       }
       return false;

       // return std::find(validGenres.cbegin(),
       // validGenres.cend(),
       //     genre) != validGenres.cend();
    }

    inline bool isValidMemberID(const std::string& memberID) {
        std::string MemberType = memberID.substr(0, 1);
        std::string MemberSeason = memberID.substr(4, 5);
        std::string MemberNumber = memberID.substr(5);

        if ((MemberType != "A" && MemberType != "M") ||
            std::stoi(MemberSeason) < 1 ||
            std::stoi(MemberSeason) > 4 ||
            MemberNumber.length() != 3) {
            return false;
        }
        return true;
    }

    inline bool isValidTransactionID(const std::string& transactionID) {
        std::string transactionType = transactionID.substr(0, 1);
        std::string transactionSeason = transactionID.substr(4, 5);
        std::string transactionNumber = transactionID.substr(5);
        if (transactionType != "T" ||
            std::stoi(transactionSeason) < 1 ||
            std::stoi(transactionSeason) > 4 ||
            transactionNumber.length() != 5) {
            return false;
        }
        return true;
    }

    inline bool isValidReservationID(const std::string& reservationID) {
        std::string reservationType = reservationID.substr(0, 1);
        std::string reservationSeason = reservationID.substr(4, 5);
        std::string reservationNumber = reservationID.substr(5);
        if (reservationType != "R" ||
            std::stoi(reservationSeason) < 1 ||
            std::stoi(reservationSeason) > 4 ||
            reservationNumber.length() != 5) {
            return false;
        }
        return true;
    }
}

#endif //LIBRARY_MANAGEMENT_SYSTEM_VALIDATOR_H
