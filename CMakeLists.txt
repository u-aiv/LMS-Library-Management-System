cmake_minimum_required(VERSION 4.1)
project(Library_Management_System)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenSSL REQUIRED)

add_executable(Library_Management_System
        src/main.cpp
        src/authentication/auth.cpp
        src/models/Book.cpp
        src/models/Member.cpp
        src/models/Transaction.cpp
        src/managers/BookManager.cpp
        src/managers/MemberManager.cpp
        src/managers/TransactionManager.cpp
        src/ui/UI.cpp
        src/ui/MenuHandler.cpp
        src/utils/FileHandler.cpp
        src/utils/DateUtils.cpp
        src/config/Config.cpp
        src/models/Reservation.cpp
        src/managers/ReservationManager.cpp
        src/managers/RecommendationManager.cpp
        src/managers/ReportManager.cpp
        src/managers/BackupManager.cpp
)

target_link_libraries(Library_Management_System
        PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
)
