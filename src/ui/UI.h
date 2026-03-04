#ifndef LIBRARY_MANAGEMENT_SYSTEM_UI_H
#define LIBRARY_MANAGEMENT_SYSTEM_UI_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

class UI {
public:
    enum class DisplayMode {
        SIMPLE,         // 简单的文本菜单
        ADVANCED,       // 带边框的框线字符
    };

    enum class MessageType {
        INFO,           // 信息提示
        SUCCESS,        // 成功确认
        WARNING,        // 警告信息
        ERR,            // 错误信息
    };

    enum class Alignment {      // 表格对齐选项
        LEFT,
        CENTER,
        RIGHT,
    };

    // 构造函数
    UI(DisplayMode = DisplayMode::ADVANCED);        // 初始显示模式: 高级

    // 显示模式设置器
    void setDisplayMode(DisplayMode mode);

    // 显示模式获取器
    DisplayMode getDisplayMode();

    // 切换显示模式
    void toggleDisplayMode();

    // 清屏
    void clearScreen();

    // 等待用户输入
    void pause(const std::string &message = "按任意键继续...") const;

    // 显示主程序头
    void displayHeader(const std::string &title, const std::string &subtitle = "") const;

    // 显示部分标题
    void displaySectionHeader(const std::string &sectionName) const;

    // 显示菜单
    void displayMenu(const std::string &title, const std::vector<std::string> &options, bool showExit) const;

    // 显示消息
    void displayMessage(const std::string &message, MessageType type = MessageType::INFO) const;

    // 获取整数输入
    int getIntInput(const std::string& prompt, int max, int min) const;

    // 获取字符串输入
    std::string getStringInput(const std::string& prompt, bool allowEmpty = false) const;

    // 获取密码输入
    std::string getPasswordInput(const std::string& prompt) const;

    // 获取 yes/no 确认
    bool getConfirmation(const std::string &prompt) const;

    // 画一条水平线
    void drawLine(int width, char lineChar = '-') const;

    // 画一个带内容的框
    void drawBox(const std::string& content, int width = 0) const;

    // 画一个带多个内容的框
    void drawBox(const std::vector<std::string>& lines, int width = 0) const;

    // 画一个进度条
    void drawProgressBar(int current, int total, const std::string& label = "", int width = 50) const;

    // 显示一个带动画的进度信息
    void displayLoading(const std::string& message, int durationMs = 1000) const;

    // 显示当前日期和时间
    void displayDateTime() const;

    // 显示一条水平分割线
    void displaySeparator() const;

    // 指定宽度的居中文本
    std::string centerText(const std::string& content, int width) const;

    // 截断文本以适应宽度
    std::string padText(const std::string& content, int width, Alignment align = Alignment::LEFT) const;

    // 截断文本以适应宽度
    std::string truncatedText(const std::string& content, int width, bool addEllipsis = true) const;

    // 常数
    static constexpr int DEFAULT_WIDTH = 70;
    static constexpr int DEFAULT_PADDING = 2;

private:
    DisplayMode currentMode;

    // 高级模式的方框绘制字符
    struct BoxChars {
        std::string topLeft;
        std::string topRight;
        std::string bottomLeft;
        std::string bottomRight;
        std::string horizontal;
        std::string vertical;
        std::string teeDown;
        std::string teeLeft;
        std::string teeRight;
        std::string teeUp;
        std::string cross;
    };

    // 根据当前模式获取框字符
    BoxChars getBoxChars() const;

    // 助手: 根据类型获取消息前缀
    std::string getMessagePrefix(MessageType type) const;

    // 助手: 将字符串转换为大写
    std::string toUpper(const std::string& str) const;

    // 助手: 将字符串转换为小写
    std::string toLower(const std::string& str) const;

    // 助手: 去除空格
    std::string trim(const std::string& str) const;
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_UI_H
