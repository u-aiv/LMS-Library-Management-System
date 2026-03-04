#ifndef LIBRARY_MANAGEMENT_SYSTEM_RECOMMENDATIONMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_RECOMMENDATIONMANAGER_H

#include "BookManager.h"
#include "MemberManager.h"
#include "TransactionManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class RecommendationManager {
private:
    BookManager bookManager;
    MemberManager memberManager;
    TransactionManager transactionManager;

    std::unordered_map<std::string, size_t> buildGenreIndex(const std::vector<Book>& books) const;
    std::vector<double> buildMemberVector(
        const Member& member,
        const std::vector<Transaction>& transactions,
        std::unordered_map<std::string, Book>& bookByISBN,
        std::unordered_map<std::string, size_t>& genreIndex) const;

    // KNN
    static double cosineSimilarity(const std::vector<double>& lhs, const std::vector<double>& rhs);

    std::unordered_set<std::string> getMemberBorrowedISBNs(const std::string& memberID, const std::vector<Transaction>& transactions) const;

    std::unordered_map<std::string, int> buildISBNPopularity(const std::vector<Transaction>& transactions) const;

public:
    explicit RecommendationManager(
        const std::string& bookPath = "../data/books.csv",
        const std::string& memberPath = "../data/members.csv",
        const std::string& transactionPath = "../data/transactions.csv");

    // 基于KNN的推荐(协同过滤), 配有内容/热门度回退机制
    std::vector<Book> recommendForMember(
        const std::string& memberID,
        int topN = 5,
        int kNeighbors = 5,
        bool availableOnly = true);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_RECOMMENDATIONMANAGER_H
