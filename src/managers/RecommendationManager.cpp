// RecommendationManager.cpp 实现

#include "RecommendationManager.h"
#include <algorithm>
#include <cmath>

// 构造函数
RecommendationManager::RecommendationManager(
    const std::string &bookPath,
    const std::string &memberPath,
    const std::string &transactionPath) :
        bookManager(bookPath),
        memberManager(memberPath),
        transactionManager(transactionPath) {}

// 类型 -> 索引 映射
std::unordered_map<std::string, size_t> RecommendationManager::buildGenreIndex(const std::vector<Book>& books) const {
    std::unordered_map<std::string, size_t> index;
    size_t next = 0;
    for (const auto& book : books) {
        const std::string& genre = book.getGenre();
        if (index.find(genre) == index.end()) {
            index[genre] = next++;
        }
    }
    return index;
}

// 建立会员 偏好/历史 向量
std::vector<double> RecommendationManager::buildMemberVector(
    const Member& member,
    const std::vector<Transaction>& transactions,
    std::unordered_map<std::string, Book>& bookByISBN,
    std::unordered_map<std::string, size_t>& genreIndex) const {
    std::vector<double> vec(genreIndex.size(), 0.0);

    // 偏好为冷启动用户提供强的信号
    // 对明确偏好的高权重
    const std::vector<std::string> preferences = member.getPreference();
    for (const auto& pref : preferences) {
        auto it = genreIndex.find(pref);
        if (it != genreIndex.end()) {
            vec[it->second] += 2.0;
        }
    }

    // 借阅历史提供协作信号
    // 对过去的借款增加权重
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() != member.getMemberID()) {      // 跳过其他会员
            continue;
        }
        auto bookIt = bookByISBN.find(transaction.getISBN());
        if (bookIt == bookByISBN.end()) {
            continue;
        }
        const std::string& genre = bookIt->second.getGenre();
        auto indexIt = genreIndex.find(genre);
        if (indexIt != genreIndex.end()) {
            vec[indexIt->second] += 1.0;
        }
    }
    return vec;
}

// 用于 KNN 的余弦相似度
double RecommendationManager::cosineSimilarity(
    const std::vector<double> &lhs,
    const std::vector<double> &rhs) {
    if (lhs.empty() || rhs.empty() || lhs.size() != rhs.size()) {
        return 0.0;
    }

    double dot = 0.0;       // 点积
    double normLhs = 0.0;   // 左操作数的范数
    double normRhs = 0.0;   // 右操作数的范数
    for (size_t i = 0; i < lhs.size(); ++i) {
        dot += lhs[i] * rhs[i];
        normLhs += lhs[i] * lhs[i];
        normRhs += rhs[i] * rhs[i];
    }
    if (normLhs <= 0 || normRhs <= 0) {
        return 0.0;
    }

    return dot / (std::sqrt(normLhs) * std::sqrt(normRhs));
}

// 收集已阅图书的 ISBN
std::unordered_set<std::string> RecommendationManager::getMemberBorrowedISBNs(
    const std::string& memberID,
    const std::vector<Transaction>& transactions) const {
    std::unordered_set<std::string> borrowed;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID) {
            borrowed.insert(transaction.getISBN());
        }
    }
    return borrowed;
}

// 计算 ISBN 受欢迎程度
std::unordered_map<std::string, int> RecommendationManager::buildISBNPopularity(
    const std::vector<Transaction>& transactions) const {
    std::unordered_map<std::string, int> countByISBN;
    for (const auto& transaction : transactions) {
        countByISBN[transaction.getISBN()]++;
    }
    return countByISBN;
}

// 主推荐函数
std::vector<Book> RecommendationManager::recommendForMember(
    const std::string& memberID,
    int topN,
    int kNeighbors,
    bool availableOnly) {
    if (topN <= 0) {        // 处理无效 topN
        return {};
    }


    Member* targetMember = memberManager.findMemberByID(memberID);        // 查找会员
    if (targetMember == nullptr) {
        return {};
    }

    // 加载所有书籍、交易和会员
    const std::vector<Book> allBooks = bookManager.getAllBooks();
    const std::vector<Transaction>& allTransactions = transactionManager.getAllTransactions();
    const std::vector<Member>& allMembers = memberManager.getAllMembers();

    std::unordered_map<std::string, Book> bookByISBN;       // ISBN -> 书籍 映射
    bookByISBN.reserve(allBooks.size());
    for (const auto& book : allBooks) {
        bookByISBN[book.getISBN()] = book;
    }

    std::unordered_map<std::string, size_t> genreIndex = buildGenreIndex(allBooks);
    if (genreIndex.empty()) {
        return {};
    }

    const std::vector<double> targetVec = buildMemberVector(
        *targetMember, allTransactions, bookByISBN, genreIndex);

    // 邻居 ID 及与其相似度
    std::vector<std::pair<std::string, double> > neighborScore;
    neighborScore.reserve(allMembers.size());
    for (const auto& member : allMembers) {
        if (member.getMemberID() == targetMember->getMemberID()) {
            continue;
        }

        const std::vector<double> vec = buildMemberVector(
            member, allTransactions, bookByISBN, genreIndex);
        double similarity = cosineSimilarity(vec, targetVec);
        if (similarity > 0.0) {
            neighborScore.emplace_back(member.getMemberID(), similarity);
        }
    }

    // 按照相似度排序
    std::sort(neighborScore.begin(), neighborScore.end(),
    [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
    {return lhs.second > rhs.second;});

    if (kNeighbors > 0 && static_cast<size_t>(kNeighbors) < neighborScore.size()) {
        neighborScore.resize(static_cast<size_t>(kNeighbors));      // 保留前 k 个邻居
    }

    const std::unordered_set<std::string> borrowedByTarget = getMemberBorrowedISBNs(memberID, allTransactions);
    const std::unordered_map<std::string, int> bookPopularity = buildISBNPopularity(allTransactions);

    std::unordered_map<std::string, double> candidateScores;

    for (const auto& neighbor : neighborScore) {
        const std::string& neighborID = neighbor.first;
        double neighborSimilarity = neighbor.second;
        for (const auto& transaction : allTransactions) {
            if (transaction.getUserID() != neighborID) {
                continue;
            }
            const std::string& neighborISBN = transaction.getISBN();
            if (borrowedByTarget.find(neighborISBN) != borrowedByTarget.end()) {
                continue;
            }
            candidateScores[transaction.getISBN()] += neighborSimilarity;
        }
    }

    // 回退：如果没有邻居, 则使用目标向量进行基于内容的评分
    if (candidateScores.empty()) {      // 没有候选人
        for (const auto& book : allBooks) {
            const std::string& isbn = book.getISBN();
            if (borrowedByTarget.find(isbn) != borrowedByTarget.end()) {
                continue;
            }
            auto idxIt = genreIndex.find(book.getGenre());
            if (idxIt == genreIndex.end()) {
                continue;
            }

            double base = targetVec[idxIt->second];     // 基于偏好的基础分
            int pop = 0;                                // 受欢迎程度统计
            auto popIt = bookPopularity.find(isbn);
            if (popIt != bookPopularity.end()) {
                pop = popIt->second;
            }
            candidateScores[isbn] = base + (0.1 * static_cast<double>(pop));
        }
    } else {                                            // 有候选人
        for (auto& entry: candidateScores) {
            auto popIt = bookPopularity.find(entry.first);
            if (popIt != bookPopularity.end()) {
                entry.second += 0.05 * static_cast<double>(popIt->second);
            }
        }
    }

    std::vector<std::pair<std::string, double> > scoredISBNs;       // 已评分的 ISBN 向量
    scoredISBNs.reserve(candidateScores.size());
    for (const auto& entry: candidateScores) {
        scoredISBNs.emplace_back(entry);
    }

    std::sort(scoredISBNs.begin(), scoredISBNs.end(),               // 按分数排序候选人
        [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs) {
        if (lhs.second != rhs.second) {
            return lhs.second > rhs.second;
        }
            return lhs.first < rhs.first; });       // 按 ISBN 排序

    std::vector<Book> resultBooks;
    for (const auto& score : scoredISBNs) {
        if (static_cast<int>(resultBooks.size()) >= topN) {
            break;
        }
        auto bookIt = bookByISBN.find(score.first);
        if (bookIt == bookByISBN.end()) {
            continue;
        }
        if (availableOnly && !bookIt->second.canBorrow()) {
            continue;
        }
        resultBooks.emplace_back(bookIt->second);
    }
    return resultBooks;
}
