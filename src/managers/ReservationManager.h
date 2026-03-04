#ifndef LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H

#include "../models/Reservation.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>
#include <map>
#include <deque>

class ReservationManager {
private:
    std::vector<Reservation> reservations;
    std::string filePath;
    FileHandler fileHandler;

    // 每个 ISBN 的预约队列 (FIFO)
    std::map<std::string, std::deque<std::string> > reservationQueues;

    // 数据持久化
    // 助手：从文件中加载预订数据
    void loadFromFile();

    // 助手：将预订数据保存到文件
    void saveToFile();

    // 用于批量操作
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // 助手：检查自动保存标志决定是否需要保存
    void saveIfNeeded();

    // 队列管理助手
    void buildQueues();     // 从已加载队列中建立队列
    void addToQueue(const std::string& isbn, const std::string& reservationID);
    bool removeFromQueue(const std::string& isbn, const std::string& reservationID);
    void rebuildQueueForISBN(const std::string& isbn);

public:
    // 构造函数
    explicit ReservationManager(const std::string& filePath = "../data/reservations.csv");

    // CRUD 操作 (增删查改)
    bool addReservation(const Reservation& reservation);
    bool updateReservation(const Reservation& reservation);
    bool deleteReservation(const Reservation& reservation);

    // 搜索函数
    Reservation* findByReservationID(const std::string &reservationID);
    std::vector<const Reservation*> findByMemberID(const std::string& memberID);
    std::vector<const Reservation*> findByISBN(const std::string& isbn);
    std::vector<const Reservation*> findByReservationDate(const std::string& reservationDate);
    std::vector<const Reservation*> findActiveReservations();

    // 预订函数
    std::string reserveBook(const std::string& memberID, const std::string& isbn);
    std::string cancelReservation(const std::string& reservationID);

    // 队列管理函数
    std::string processNextReservation(const std::string& isbn);
    std::string getNextInQueue(const std::string& isbn) const;
    int getQueuePosition(const std::string& reservationID) const;
    int getQueueLength(const std::string& isbn) const;
    std::vector<std::string> getQueueForISBN(const std::string& isbn) const;
    bool hasActiveReservations(const std::string& isbn) const;

    // 获取器
    const std::vector<Reservation>& getAllReservations() const;
    int getTotalReservations() const;
    int getActiveReservations() const;

    // 实用方法
    void reload();          // 重新加载文件
    void clearCache();      // 清理文件处理器缓存
    bool isReservationIDExists(const std::string& reservationID) const;

    // 批量操作 (RAII)
    class BatchOperation {
    private:
        ReservationManager* reservationManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(ReservationManager& rmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H
