// Member.h 实现

#include <sstream>
#include "Member.h"
#include "../utils/DateUtils.h"
#include "../authentication/auth.h"

std::string Member::getMemberID() const {
    return memberID;
}
std::string Member::getName() const{
    return name;
}
std::string Member::getPhoneNumber() const {
    return phoneNumber;
}
std::vector<std::string> Member::getPreference() const {
    return preference;
}
std::string Member::getRegistrationDate() const {
    return registrationDate;
}
std::string Member::getExpiryDate() const {
    return expiryDate;
}
std::string Member::getPasswordHash() const {
    return passwordHash;
}
bool Member::isExpired() const {
    time_t nowTime = DateUtils::getCurrentTimestamp();
    time_t expiryTime = DateUtils::dateToTimestamp(expiryDate);
    return nowTime > expiryTime;
}

bool Member::getAdmin() const {
    return isAdmin;
}

int Member::getMaxBooksAllowed() const {
    return maxBooksAllowed;
}

bool Member::authenticate(const std::string& password) const {
    return auth::verifyPassword(password, passwordHash);
}

std::string Member::toCSV() const{
    std::string preferenceList;
    for (size_t i = 0; i < preference.size(); ++i) {
        preferenceList += preference[i];
        if (i < preference.size() - 1) {
            preferenceList += ';';
        }
    }
    if (preferenceList.empty()) {
        preferenceList = "None";
    }

    return memberID + "," +
           name + "," +
           phoneNumber + "," +
           preferenceList + "," +
           registrationDate + "," +
           expiryDate + "," +
           std::to_string(maxBooksAllowed) + "," +
           (isAdmin ? "1" : "0") + "," +
           passwordHash;
}

Member Member::fromCSV(const std::string& csvLine) {
    Member member;
    std::istringstream iss(csvLine);

    getline(iss, member.memberID, ',');
    getline(iss, member.name, ',');
    getline(iss, member.phoneNumber, ',');

    std::string preferenceList;
    getline(iss, preferenceList, ',');

    if (preferenceList != "None") {
        std::istringstream prefStream(preferenceList);
        std::string pref;
        while (getline(prefStream, pref, ';')) {
            if (!pref.empty()) {
                member.preference.push_back(pref);
            }
        }
    }

    getline(iss, member.registrationDate, ',');
    getline(iss, member.expiryDate, ',');
    iss.ignore(1);
    iss >> member.maxBooksAllowed;
    iss.ignore(1);

    int boolInt;
    iss >> boolInt;
    member.isAdmin = (boolInt != 0);

    iss.ignore(1);
    getline(iss, member.passwordHash);

    return member;
}
