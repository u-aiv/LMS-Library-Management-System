// ReservationManager.cpp 实现

#include "ReservationManager.h"
#include "MemberManager.h"
#include "BookManager.h"
#include "../utils/DateUtils.h"
#include "../utils/Validator.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>


// 构造函数
ReservationManager::ReservationManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "ReservationID,MemberID,ISBN,ReservationDate,IsActive"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
    buildQueues();
}

// 私有: 助手: 从文件加载预订数据
void ReservationManager::loadFromFile() {
    reservations.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // 跳过表头 (第一行)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                reservations.push_back(Reservation::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("加载预订文件失败: " + std::string(e.what()));
    }
}

// 私有: 助手: 将预订数据保存到文件
void ReservationManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("ReservationID,MemberID,ISBN,ReservationDate,IsActive");

    // 添加所有预订
    for (const auto& reservation : reservations) {
        lines.push_back(reservation.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("保存预订文件失败: " + std::string(e.what()));
    }
}

// 私有: 辅助: 检查自动保存标志决定是否需要保存
void ReservationManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool ReservationManager::isAutoSaveEnabled() const {
    return autoSave;
}

// 助手: 检查自动保存标志以决定是否需要保存
void ReservationManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// 从已加载的预订中建立队列
void ReservationManager::buildQueues() {
    reservationQueues.clear();

    // 按每个 ISBN 的日期排序预订
    std::map<std::string,std::vector<Reservation*> > isbnGroups;

    for (auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            isbnGroups[reservation.getISBN()].push_back(&reservation);
        }
    }

    // 为每个 ISBN 构建有序队列
    for (auto& pair : isbnGroups) {
        const auto& isbn = pair.first;
        auto& reservationGroup = pair.second;

        // Sort by reservation date (FIFO)
        std::sort(reservationGroup.begin(), reservationGroup.end(),
            [](const Reservation* a, const Reservation* b) {
                return a->getReservationDate() < b->getReservationDate();
            });

        // Add to queue in order
        for (const auto& reservation : reservationGroup) {
            reservationQueues[isbn].push_back(reservation->getReservationID());
        }
    }
}

// 将预订添加到其 ISBN 的队列中
void ReservationManager::addToQueue(const std::string& isbn, const std::string& reservationID) {
    reservationQueues[isbn].push_back(reservationID);
}

// 从队列中移除预订
bool ReservationManager::removeFromQueue(const std::string &isbn, const std::string &reservationID) {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return false;
    }

    auto& queue = it->second;
    auto queueIt = std::find(queue.begin(), queue.end(), reservationID);

    if (queueIt != queue.end()) {
        queue.erase(queueIt);

        // 删除 ISBN 条目若队列现在为空
        if (queue.empty()) {
            reservationQueues.erase(it);
        }
        return true;
    }
    return false;
}

// 为特定 ISBN 重建队列（在批量操作之后）
void ReservationManager::rebuildQueueForISBN(const std::string &isbn) {
    reservationQueues.erase(isbn);

    // 收集此 ISBN 的活跃预订
    std::vector<Reservation*> isbnReservations;
    for (auto& reservation : reservations) {
        if (reservation.getISBN() == isbn && reservation.getIsActive()) {
            isbnReservations.push_back(&reservation);
        }
    }

    // 按日期排序
    std::sort(isbnReservations.begin(), isbnReservations.end(),
        [](const Reservation* a, const Reservation* b) {
            return a->getReservationDate() < b->getReservationDate();
        });

    // 重建队列
    for (const auto* reservation : isbnReservations) {
        reservationQueues[isbn].push_back(reservation->getReservationID());
    }
}

// 处理队列中的下一个预约当书籍可用时
std::string ReservationManager::processNextReservation(const std::string& isbn) {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end() || it->second.empty()) {
        return "";      // 队列中无预订
    }

    std::string nextReservationID = it->second.front();
    return nextReservationID;
}

// 获取队列中的下一个预约 ID
std::string ReservationManager::getNextInQueue(const std::string& isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end() || it->second.empty()) {
        return "";
    }
    return it->second.front();
}

// 获取预订在其队列中的位置 (1-indexed)
int ReservationManager::getQueuePosition(const std::string& reservationID) const {
    const Reservation* res = nullptr;
    for (const auto& reservation : reservations) {
        if (reservation.getReservationID() == reservationID) {
            res = &reservation;
            break;
        }
    }

    if (res == nullptr || !res->getIsActive()) {
        return -1;      // 未找到或无效
    }

    // 在队列中查找位置
    auto it = reservationQueues.find(res->getISBN());
    if (it == reservationQueues.end()) {
        return -1;
    }

    const auto& queue = it->second;
    auto queueIt = std::find(queue.begin(), queue.end(), reservationID);

    if (queueIt == queue.end()) {
        return -1;
    }

    return static_cast<int>(std::distance(queue.begin(), queueIt)) + 1;
}

// 获取特定 ISBN 的队列长度
int ReservationManager::getQueueLength(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return 0;
    }
    return static_cast<int>(it->second.size());
}

// 获取某个 ISBN 的排队预订 ID
std::vector<std::string> ReservationManager::getQueueForISBN(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return {};
    }
    return {it->second.begin(), it->second.end()};
}

// 检查某个 ISBN 是否有有效预订
bool ReservationManager::hasActiveReservations(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    return it != reservationQueues.end() && !it->second.empty();
}

// 新增一条预订
bool ReservationManager::addReservation(const Reservation& reservation) {
    // 检查 ReservationID 是否已存在
    if (isReservationIDExists(reservation.getReservationID())) {
        return false;
    }
    reservations.push_back(reservation);

    if (reservation.getIsActive()) {
        addToQueue(reservation.getISBN(), reservation.getReservationID());
    }

    if (autoSave) {
        saveToFile();
    }
    return true;
}

// 更新现有预订
bool ReservationManager::updateReservation(const Reservation& reservation) {
    Reservation *existingReservation = findByReservationID(reservation.getReservationID());

    if (existingReservation == nullptr) {
        return false;
    }

    bool wasActive = existingReservation->getIsActive();
    bool isActive = reservation.getIsActive();
    std::string isbn = existingReservation->getISBN();

    *existingReservation = reservation;

    // 根据状态变化更新队列
    if (wasActive && !isActive) {
        removeFromQueue(isbn, reservation.getReservationID());
    } else if (!wasActive && isActive) {
        addToQueue(isbn, reservation.getReservationID());
    }

    if (autoSave) {
        saveToFile();
    }
    return true;
}

// 删除预订
bool ReservationManager::deleteReservation(const Reservation& reservation) {
    auto it = std::find_if(reservations.begin(), reservations.end(),
        [&](const Reservation& res) { return res.getReservationID() == reservation.getReservationID(); });

    if (it != reservations.end()) {
        if (it->getIsActive()) {
            removeFromQueue(it->getISBN(), reservation.getReservationID());
        }

        reservations.erase(it);
        if (autoSave) {
            saveToFile();
        }
        return true;
    }
    return false;
}

// 以预订 ID 查找预订
Reservation* ReservationManager::findByReservationID(const std::string &reservationID) {
    for (auto& reservation : reservations) {
        if (reservation.getReservationID() == reservationID) {
            return &reservation;
        }
    }
    return nullptr;
}

// 以会员 ID 查找预订
std::vector<const Reservation*> ReservationManager::findByMemberID(const std::string& memberID) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getMemberID() == memberID) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// 以预订 ISBN 查找预订
std::vector<const Reservation*> ReservationManager::findByISBN(const std::string& isbn) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getISBN() == isbn) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// 以预订日期查找预订
std::vector<const Reservation*> ReservationManager::findByReservationDate(const std::string& reservationDate) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getReservationDate() == reservationDate) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// 查找有效预订
std::vector<const Reservation*> ReservationManager::findActiveReservations() {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// 预约图书
std::string ReservationManager::reserveBook(const std::string& memberID, const std::string& isbn) {
    MemberManager memberManager;
    Member* member = memberManager.findMemberByID(memberID);
    BookManager bookManager;
    Book* book = bookManager.findBookByISBN(isbn);

    if (!member || !book) {
        return "0";
    }
    if (member->isExpired()) {
        return "0";
    }

    // 避免为同一会员和 ISBN 创建重复预订
    for (const auto& reservation : reservations) {
        if (reservation.getIsActive() &&
            reservation.getMemberID() == memberID &&
            reservation.getISBN() == isbn) {
            return "0";
        }
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string year = currentDate.substr(0,4);
    int month = std::stoi(currentDate.substr(5,2));
    int season = (month - 1) / 3 + 1;

    std::string prefix = "R" + year + std::to_string(season);
    int maxSeq = 0;
    for (const auto& reservation : reservations) {
        std::string id = reservation.getReservationID();
        if (id.rfind(prefix, 0) != 0) {
            continue;
        }
        if (id.size() < prefix.size() + 5) {
            continue;
        }
        std::string suffix = id.substr(prefix.size());
        bool allDigits = std::all_of(suffix.begin(), suffix.end(),
            [](unsigned char c) { return std::isdigit(c); });
        if (!allDigits) {
            continue;
        }

        int seq = std::stoi(suffix);
        if (seq > maxSeq) {
            maxSeq = seq;
        }
    }

    std::ostringstream oss;
    oss << prefix << std::setw(5) << std::setfill('0') << (maxSeq + 1);
    std::string reservationID = oss.str();

    Reservation reservation(reservationID, memberID, isbn, currentDate, true);
    if (!addReservation(reservation)) {
        return "0";
    }

    // 标记书本为已预定
    Book updatedBook = *book;
    updatedBook.setReserved(true);
    if (!bookManager.updateBook(updatedBook)) {
        return "0";
    }

    return reservationID;
}

// 取消预订
std::string ReservationManager::cancelReservation(const std::string& reservationID) {
    Reservation* reservation = findByReservationID(reservationID);
    if (!reservation || !reservation->getIsActive()) {
        return "0";
    }

    std::string isbn = reservation->getISBN();
    removeFromQueue(isbn, reservationID);
    reservation->cancelReservation();
    saveIfNeeded();

    BookManager bookManager;
    Book* book = bookManager.findBookByISBN(isbn);
    if (!book) {
        return "0";
    }

    // 检查此 ISBN 是否仍有有效的预订
    bool hasActiveForISBN = hasActiveReservations(isbn);

    Book updatedBook = *book;
    updatedBook.setReserved(hasActiveForISBN);
    if (!bookManager.updateBook(updatedBook)) {
        return "0";
    }

    return reservationID;
}

// 获取所有预订
const std::vector<Reservation>& ReservationManager::getAllReservations() const {
    return reservations;
}

// 获取预订总数
int ReservationManager::getTotalReservations() const {
    return static_cast<int>(reservations.size());
}

// 获取有效预订数量
int ReservationManager::getActiveReservations() const {
    int count = 0;
    for (const auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            count++;
        }
    }
    return count;
}

// 重新加载文件
void ReservationManager::reload() {
    loadFromFile();
    buildQueues();
}

// 清除文件处理器缓存
void ReservationManager::clearCache() {
    fileHandler.clearCache();
}

// 检查预订 ID 是否存在
bool ReservationManager::isReservationIDExists(const std::string& reservationID) const {
    return std::find_if(reservations.begin(), reservations.end(),
        [&](const Reservation& reservation) { return reservation.getReservationID() == reservationID; }) != reservations.end();
}

// 批量操作 (RAII)
ReservationManager::BatchOperation::BatchOperation(ReservationManager& rmgr) :
                    reservationManager(&rmgr), originalAutoSave(rmgr.autoSave), active(true) {
    rmgr.setAutoSave(false);
}

ReservationManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    reservationManager(other.reservationManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

ReservationManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        reservationManager->saveToFile();
        reservationManager->buildQueues();
        reservationManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "在批量操作期间尝试保存预订时出错" << std::endl;
    }
}

ReservationManager::BatchOperation ReservationManager::beginBatch() {
    return BatchOperation(*this);
}
