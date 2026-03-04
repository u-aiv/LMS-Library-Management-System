#ifndef LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H
#define LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H

#include <string>
#include <ctime>

namespace DateUtils{
// 转换 YYYY-MM-DD 到 UNIX 时间戳
time_t dateToTimestamp(const std::string& dateString);

// 转换 UNIX 时间戳 到 YYYY-MM-DD
std::string timestampToDate(time_t timestamp);

// 获得现在日期 YYYY-MM-DD
std::string getCurrentDate();

// 获得现在精准时间 YYYY-MM-DD h:m:s
std::string getCurrentDateTime();

// 获得现在 UNIX 时间戳
time_t getCurrentTimestamp();

// 计算天数间隔
unsigned int daysBetween(time_t startTime, time_t endTime);

// 天数增量
std::string addDays(const std::string& dateString, int addition);
}

#endif //LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H
