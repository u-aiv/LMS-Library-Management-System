#include "UI.h"
#include "../utils/DateUtils.h"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <thread>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

// 构造函数
UI::UI(DisplayMode mode) : currentMode(mode) {}

// 显示模式设置器
void UI::setDisplayMode(DisplayMode mode) {
    currentMode = mode;
}

// 显示模式获取器
UI::DisplayMode UI::getDisplayMode() {
    return currentMode;
}

// 切换显示模式
void UI::toggleDisplayMode() {
    currentMode = (currentMode == DisplayMode::SIMPLE) ? DisplayMode::ADVANCED : DisplayMode::SIMPLE;
}

// 清屏
void UI::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// 暂停并等待用户输入
void UI::pause(const std::string &message) const {
    std::cout << "\n" << message;
    #ifdef _WIN32
        _getch();
    #else
        struct termios oldSettings, newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
        getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif
    std::cout << std::endl;
}

// 根据当前模式获取框字符
UI::BoxChars UI::getBoxChars() const {
    if (currentMode == DisplayMode::ADVANCED) {
        return {
            // UTF-8 框绘图字符
            "\xE2\x95\x94",  // ╔ top-left
            "\xE2\x95\x97",  // ╗ top-right
            "\xE2\x95\x9A",  // ╚ bottom-left
            "\xE2\x95\x9D",  // ╝ bottom-right
            "\xE2\x95\x90",  // ═ horizontal
            "\xE2\x95\x91",  // ║ vertical
            "\xE2\x95\xA6",  // ╦ tee-down
            "\xE2\x95\xA9",  // ╩ tee-up
            "\xE2\x95\xA0",  // ╠ tee-right
            "\xE2\x95\xA3",  // ╣ tee-left
            "\xE2\x95\xAC"   // ╬ cross
            };
        } else {
            return {
            "+",  // top-left
            "+",  // top-right
            "+",  // bottom-left
            "+",  // bottom-right
            "-",  // horizontal
            "|",  // vertical
            "+",  // tee-down
            "+",  // tee-up
            "+",  // tee-right
            "+",  // tee-left
            "+"   // cross
            };
    }
}

// 显示主程序头
void UI::displayHeader(const std::string &title, const std::string &subtitle) const {
    BoxChars boxChars = getBoxChars();
    int width = DEFAULT_WIDTH;

    // 上边框
    std::cout << boxChars.topLeft;
    for (size_t i = 1; i < width - 1; i++) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.topRight << std::endl;

    // 标题
    std::cout << boxChars.vertical << centerText(title, width - 2) << boxChars.vertical << std::endl;

    // 子标题 (若有)
    if (!subtitle.empty()) {
        std::cout << boxChars.vertical << centerText(subtitle, width - 2) << boxChars.vertical << std::endl;
    }

    // 下边框
    std::cout << boxChars.bottomLeft;
    for (size_t i = 1; i < width - 1; i++) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.bottomRight << std::endl;
}

// 显示部分标题
void UI::displaySectionHeader(const std::string &sectionName) const {
    std::cout << '\n';
    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        std::cout << boxChars.teeDown << " " << sectionName << " ";
        for (size_t i = 0; i < DEFAULT_WIDTH - sectionName.length(); i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << std::endl;
    } else {
        std::cout << "=== " << sectionName << " ===" << std::endl;
    }
    std::cout << std::endl;
}

// 显示目录
void UI::displayMenu(const std::string &title, const std::vector<std::string> &options, bool showExit) const {
    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        int width = DEFAULT_WIDTH;

        // 上边框
        std::cout << boxChars.topLeft;
        for (size_t i = 1; i < width - 1; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.topRight << std::endl;

        // 标题
        std::cout << boxChars.vertical << centerText(title, width - 2) << std::endl;

        // 中间分隔符
        std::cout << boxChars.teeLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.teeRight << std::endl;

        // 菜单选项
        for (size_t i = 0; i < options.size(); i++) {
            std::ostringstream oss;
            oss << std::setw(3) << std::right << (i + 1) << ". " << options[i] << std::endl;
            std::cout << boxChars.vertical << " " << std::left << std::setw(width - 3) << oss.str() << boxChars.vertical <<std::endl;
        }
        std::cout << std::endl;

        // 用于高级显示模式的退出选项
        if (showExit) {
            std::ostringstream oss;
            oss << std::setw(3) << std::right << "0" << ". " << "退出/回退";
            std::cout << boxChars.vertical << " " << std::left << std::setw(width - 4)
                      << oss.str() << " " << boxChars.vertical << std::endl;
        }

        // 下边框
        std::cout << boxChars.bottomLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.bottomRight << std::endl;
    } else {
        // 简单模式
        std::cout << "\n" << title << "\n";
        for (size_t i = 0; i < title.length(); i++) {
            std::cout << "=";
        }
        std::cout << "\n\n";

        for (size_t i = 0; i < options.size(); i++) {
            std::cout << std::setw(3) << std::right << (i + 1) << ". " << options[i];
        }

        // 用于简单显示模式的退出选项
        if (showExit) {
            std::cout << std::setw(3) << std::right << "0" << ". " << "退出/回退" << std::endl;
        }
        std::cout << std::endl;
    }
}

// 显示消息
// 助手: 基于类型获取消息前缀
std::string UI::getMessagePrefix(MessageType type) const {
    switch (type) {
        case MessageType::SUCCESS:
            return "[SUCCESS]";
        case MessageType::WARNING:
            return "[WARNING]";
        case MessageType::ERR:
            return "[ERROR]";
        case MessageType::INFO:
        default:
            return "[INFO]";
    }
}

// 显示消息
void UI::displayMessage(const std::string &message, MessageType type) const {
    std::string prefix = getMessagePrefix(type);

    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        int width = DEFAULT_WIDTH;

        std::cout << "\n" << boxChars.topLeft;
        for (size_t i = 1; i < width - 1; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.topRight << std::endl;

        std::cout << boxChars.vertical << " " << std::left
                  << std::setw(width - 3) << (prefix + message) << boxChars.vertical << std::endl;

        std::cout << boxChars.bottomLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.bottomRight << std::endl;
    } else {
        std::cout << "\n" << (prefix + message) << "\n" << std::endl;
    }
}

// 输入函数
// 获取整数输入
int UI::getIntInput(const std::string &prompt, int max, int min) const {
    int value;
    bool valid = false;

    while (!valid) {
        std::cout << prompt << ": ";
        if (std::cin >> value) {
            if (value >= min && value <= max) {
                valid = true;
            } else {
                displayMessage("请输入一个数介于 " +
                    std::to_string(min) + " 和 " + std::to_string(max),
                    MessageType::WARNING);
            }
        } else {
            displayMessage("无效输入, 请输入一个数", MessageType::ERR);
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }
    std::cin.ignore(10000, '\n');       // Clear trailing newline
    return value;
}

// 获取字符串输入
std::string UI::getStringInput(const std::string& prompt, bool allowEmpty) const {
    std::string input;
    bool valid = false;

    while (!valid) {
        std::cout << prompt << ": ";
        std::getline(std::cin, input);

        input = trim(input);

        if (!input.empty() || allowEmpty) {
            valid = true;
        } else {
            displayMessage("请输入一个有效的输入", MessageType::ERR);
        }
    }
    return input;
}

// 获取密码输入
std::string UI::getPasswordInput(const std::string& prompt) const {
    std::cout << prompt << ": ";
    std::string password;

    #ifdef _WIN32
        char ch;
        while ((ch = _getch()) != '\r') {  // 输入密码
            if (ch == '\b') {  // 退格
                if (!password.empty()) {
                    password.pop_back();
                    std::cout << "\b \b";
                }
            } else {
                password.push_back(ch);
                std::cout << '*';
            }
        }
    #else
        struct termios oldSettings, newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

        std::getline(std::cin, password);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    #endif

    std::cout << std::endl;
    return password;
}

// 获取 yes/no 确认
bool UI::getConfirmation(const std::string &prompt) const {
    std::string input;
    while (true) {
        std::cout << prompt << " (y/n) ";
        std::getline(std::cin, input);

        input = trim(toLower(input));

        if (input == "y" || input == "yes") {
            return true;
        } else if (input == "n" || input == "no") {
            return false;
        } else {
            displayMessage("请输入 'y' 或 'n'.", MessageType::WARNING);
        }
    }
}

// 绘框函数
// 画一条水平线
void UI::drawLine(int width, char lineChar) const {
    for (size_t i = 0; i < width; i++) {
        std::cout << lineChar;
    }
    std::cout << std::endl;
}

// 画一个带内容的框
void UI::drawBox(const std::string &content, int width) const {
    drawBox(std::vector<std::string>{content}, width);
}

// 画一个带多个内容的框
void UI::drawBox(const std::vector<std::string> &lines, int width) const {
    BoxChars boxChars = getBoxChars();

    // 自动计算宽度
    if (width == 0) {
        for (const auto& line : lines) {
            width = std::max(width, static_cast<int>(line.length()) + 4);
        }
    }

    // 上框线
    std::cout << boxChars.topLeft;
    for (size_t i = 1; i < width - 1; ++i) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.topRight << std::endl;

    // 内容
    for (const auto& line : lines) {
        std::cout << boxChars.vertical << " " << std::left << std::setw(width - 3)
                 << line << boxChars.vertical << std::endl;
    }

    // 下框线
    std::cout << boxChars.bottomLeft;
    for (size_t i = 1; i < width - 1; ++i) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.bottomRight << std::endl;
}

// 画一个进度条
void UI::drawProgressBar(int current, int total, const std::string& label, int width) const {
    double percentage = (total > 0) ? static_cast<double>(current) / total * 100.0 : 0.0;
    int filled = static_cast<int>(width * current / total);

    std::cout << "\r";
    if (!label.empty()) {
        std::cout << label << " ";
    }

    std::cout << "[";
    for (size_t i = 0; i < width; ++i) {
        if (i < filled) {
            std::cout << '=';
        } else if (i == filled) {
            std::cout << '>';
        } else {
            std::cout << ' ';
        }
    }
    std::cout << ']' << std::fixed << std::setprecision(1) << percentage << std::endl;
    std::cout.flush();

    if (current >= total) {
        std::cout << std::endl;
    }
}

// 显示一个带动画的进度信息
void UI::displayLoading(const std::string& message, int durationMs) const {
    const char spinChars[] = {'|', '/', '-', '\\'};
    int iterations = durationMs / 100;

    for (size_t i = 0; i < iterations; ++i) {
        std::cout << "\r" << message << " " << spinChars[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\r" << message << "Done." << std::endl;
}

// 显示当前日期和时间
void UI::displayDateTime() const {
    std::cout << DateUtils::getCurrentDateTime() << std::endl;
}

// 画一条水平分割线
void UI::displaySeparator() const {
    drawLine(DEFAULT_WIDTH, '-');
}

// 指定宽度的居中文本
std::string UI::centerText(const std::string& content, int width) const {
    int padding = (width - static_cast<int>(content.length())) / 2;
    if (padding < 0) {
        padding = 0;
    }

    std::string result(padding, ' ');
    result += content;
    result.resize(width, ' ');

    return result;
}

// 将文本填充到特定宽度
std::string UI::padText(const std::string& content, int width, Alignment align) const {
    if (static_cast<int>(content.length()) >= width) {
        return content.substr(0, width);
    }

    std::string result;
    int padding = width - static_cast<int>(content.length());

    switch (align) {
        case Alignment::LEFT:
            result = content + std::string(padding, ' ');
            break;
        case Alignment::RIGHT:
            result = std::string(padding, ' ') + content;
            break;
        default:
            result = centerText(content, width);
            break;
    }

    return result;
}

// 截断文本以适应宽度
std::string UI::truncatedText(const std::string& content, int width, bool addEllipsis) const {
    if (static_cast<int>(content.length()) <= width) {
        return content;
    }

    if (addEllipsis && width > 3) {
        return content.substr(0, width - 3) + "...";
    }

    return content.substr(0, width);
}

// 助手: 将字符串转换为大写
std::string UI::toUpper(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// 助手: 将字符串转换为小写
std::string UI::toLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// 助手: 修剪空白
std::string UI::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}
