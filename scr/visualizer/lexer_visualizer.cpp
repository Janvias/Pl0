/**
 * @file lexer_visualizer.cpp
 * @brief PL/0 词法分析可视化程序（ASCII艺术版）
 * @description 使用纯 C++ 实现词法分析过程的可视化，无需图形库
 * 
 * 功能：
 * 1. 词法分析（识别关键字、标识符、常数、运算符、界限符）
 * 2. 单词分类表可视化（表格显示）
 * 3. 状态转换图可视化（ASCII自动机图）
 * 4. 识别流程图可视化（ASCII流程图）
 * 5. 实时输出分析结果
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace std;

//============================================================================
// 常量定义
//============================================================================

// Token 类型
enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    DELIMITER,
    ERROR,
    END_OF_FILE,
    UNKNOWN
};

// Token 结构
struct Token {
    TokenType type;
    string value;
    int line;
    int startPos;
    int endPos;
};

// 关键字表
const map<string, bool> KEYWORDS = {
    {"const", true}, {"var", true}, {"procedure", true},
    {"begin", true}, {"end", true}, {"if", true},
    {"then", true}, {"while", true}, {"do", true},
    {"call", true}, {"read", true}, {"write", true},
    {"odd", true}
};

// DFA 状态
enum class DFAState {
    S0_START,
    S1_IDENT,
    S2_NUMBER,
    S3_OPERATOR,
    S4_DELIMITER,
    S5_TWOBYTE_OP,
    SF_FINAL,
    S_ERROR
};

// 动画延迟（毫秒）
const int ANIMATION_DELAY = 200;

//============================================================================
// 工具函数
//============================================================================

// 获取 Token 类型名称
string getTokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::DELIMITER: return "DELIMITER";
        case TokenType::ERROR: return "ERROR";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

// 获取 Token 颜色代码（ANSI）
string getTokenColor(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD: return "\033[33m";      // 黄色
        case TokenType::IDENTIFIER: return "\033[36m";   // 青色
        case TokenType::NUMBER: return "\033[93m";       // 亮黄色
        case TokenType::OPERATOR: return "\033[35m";     // 紫色
        case TokenType::DELIMITER: return "\033[94m";    // 蓝色
        case TokenType::ERROR: return "\033[91m";       // 红色
        default: return "\033[0m";
    }
}

// 重置颜色
const string RESET = "\033[0m";

// 高亮文本
string highlight(const string& text, TokenType type) {
    return getTokenColor(type) + text + RESET;
}

// 判断是否为运算符
bool isOperator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
           ch == '=' || ch == '#' || ch == '<' || ch == '>';
}

// 判断是否为界限符
bool isDelimiter(char ch) {
    return ch == ';' || ch == ',' || ch == '.' || ch == '(' || 
           ch == ')' || ch == ':';
}

//============================================================================
// 词法分析器类
//============================================================================

class LexerVisualizer {
private:
    // 输入缓冲区
    string inputBuffer;
    vector<Token> tokens;
    
    // 当前分析位置
    int currentPos;
    int currentLine;
    int currentCol;
    
    // 统计信息
    int keywordCount;
    int identifierCount;
    int numberCount;
    int operatorCount;
    int delimiterCount;
    
    // 当前分析步骤描述
    string currentStep;
    
    // 输出文件流
    ofstream outFile;

public:
    LexerVisualizer() {
        currentPos = 0;
        currentLine = 1;
        currentCol = 1;
        keywordCount = identifierCount = numberCount = 
                      operatorCount = delimiterCount = 0;
        currentStep = "准备开始词法分析...";
    }
    
    // 加载文件
    bool loadFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        inputBuffer.clear();
        string line;
        while (getline(file, line)) {
            inputBuffer += line + "\n";
        }
        file.close();
        
        return true;
    }
    
    // 从控制台输入代码
    void inputFromConsole() {
        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════════╗\n";
        cout << "║           PL/0 词法分析可视化程序 (ASCII 艺术版)             ║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n";
        cout << "\n请输入 PL/0 代码（输入完成后按 Ctrl+Z 并回车结束）:\n";
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        inputBuffer.clear();
        string line;
        while (getline(cin, line)) {
            inputBuffer += line + "\n";
        }
        
        // 移除最后的换行
        if (!inputBuffer.empty() && inputBuffer.back() == '\n') {
            inputBuffer.pop_back();
        }
    }
    
    // 清除屏幕
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    // 等待按键
    void waitForEnter() {
        cout << "\n按 Enter 键继续...";
        cin.ignore();
        cin.get();
    }
    
    // 动画延迟
    void animate() {
        #ifdef _WIN32
            _sleep(ANIMATION_DELAY);
        #else
            usleep(ANIMATION_DELAY * 1000);
        #endif
    }
    
    //============================================================================
    // 可视化函数
    //============================================================================
    
    // 绘制主界面
    void drawMainUI() {
        clearScreen();
        
        // 标题栏
        cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║           PL/0 词法分析可视化系统                          ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        
        // 代码输入区域
        cout << "║ 源代码输入                                              ║\n";
        cout << "║ ─────────────────────────────────────────────────────── ║\n";
        
        if (inputBuffer.empty()) {
            cout << "║ (空)                                                   ║\n";
        } else {
            stringstream ss(inputBuffer);
            string line;
            int lineNum = 1;
            
            while (getline(ss, line) && lineNum <= 10) {
                if (line.length() > 60) {
                    line = line.substr(0, 60);
                }
                printf("║ %3d │ %-60s ║\n", lineNum, line.c_str());
                lineNum++;
            }
            if (lineNum > 10) {
                cout << "║     │ ... (更多行省略)                                    ║\n";
            }
        }
        
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        
        // Token 序列
        cout << "║ Token 序列                                    实时分析 ║\n";
        cout << "║ ─────────────────────────────────────────────────────────────── ║\n";
        
        for (size_t i = 0; i < tokens.size() && i < 15; i++) {
            const Token& t = tokens[i];
            string typeStr = getTokenTypeName(t.type);
            string valueStr = t.value;
            if (valueStr.length() > 12) {
                valueStr = valueStr.substr(0, 12) + "...";
            }
            printf("║ %2zu. %-12s %-15s (行%-3d)                        ║\n",
                   i + 1, typeStr.c_str(), valueStr.c_str(), t.line);
        }
        
        if (tokens.size() > 15) {
            cout << "║ ... (更多 Token 省略)                                            ║\n";
        }
        
        if (tokens.empty() && !inputBuffer.empty()) {
            cout << "║ 等待分析...                                                      ║\n";
        }
        
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        
        // 单词分类统计表
        drawClassificationTable();
        
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        
        // 状态栏
        cout << "║ ";
        cout << "\033[1;36m当前步骤:\033[0m " << currentStep;
        int spaces = 90 - 14 - currentStep.length();
        for (int i = 0; i < spaces && i < 200; i++) cout << " ";
        cout << "║\n";
        
        printf("║ 位置: 第 %d 行, 第 %d 列  |  Token 总数: %-3zu                            ║\n",
               currentLine, currentCol, tokens.size());
        
        cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
    }
    
    // 绘制分类表
    void drawClassificationTable() {
        cout << "║ 单词分类统计表                                                             ║\n";
        cout << "║ ───────────────────────────────────────────────────────────────────────────── ║\n";
        
        int total = max(1, keywordCount + identifierCount + numberCount + 
                        operatorCount + delimiterCount);
        
        // 表头
        printf("║ %-20s │ %-25s │ %6s │ %8s │ %10s ║\n",
               "分类", "示例", "数量", "百分比", "分布图");
        cout << "║──────────────────────┼───────────────────────────┼────────┼──────────┼────────────║\n";
        
        // 关键字
        double kp = (keywordCount * 100.0) / total;
        printf("║ %-20s │ %-25s │ %6d │ %7.1f%% │ ", 
               "关键字(KEYWORD)", "const, var, begin...", keywordCount, kp);
        drawBar(keywordCount, total, 10);
        cout << " ║\n";
        
        // 标识符
        double ip = (identifierCount * 100.0) / total;
        printf("║ %-20s │ %-25s │ %6d │ %7.1f%% │ ", 
               "标识符(IDENTIFIER)", "x, y, sum, i...", identifierCount, ip);
        drawBar(identifierCount, total, 10);
        cout << " ║\n";
        
        // 常数
        double np = (numberCount * 100.0) / total;
        printf("║ %-20s │ %-25s │ %6d │ %7.1f%% │ ", 
               "常数(NUMBER)", "10, 100, 12345...", numberCount, np);
        drawBar(numberCount, total, 10);
        cout << " ║\n";
        
        // 运算符
        double op = (operatorCount * 100.0) / total;
        printf("║ %-20s │ %-25s │ %6d │ %7.1f%% │ ", 
               "运算符(OPERATOR)", "+, -, *, /, :=...", operatorCount, op);
        drawBar(operatorCount, total, 10);
        cout << " ║\n";
        
        // 界限符
        double dp = (delimiterCount * 100.0) / total;
        printf("║ %-20s │ %-25s │ %6d │ %7.1f%% │ ", 
               "界限符(DELIMITER)", ";, ,, (, ), .", delimiterCount, dp);
        drawBar(delimiterCount, total, 10);
        cout << " ║\n";
    }
    
    // 绘制条形图
    void drawBar(int value, int total, int width) {
        if (total == 0) {
            for (int i = 0; i < width; i++) cout << " ";
            return;
        }
        
        int barLen = (value * width) / total;
        for (int i = 0; i < barLen; i++) cout << "█";
        for (int i = barLen; i < width; i++) cout << "░";
    }
    
    // 绘制状态转换图
    void drawStateTransitionDiagram() {
        clearScreen();
        
        cout << "\n";
        cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║                    状态转换图 (DFA)                         ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║                                                                  ║\n";
        cout << "║                        ┌─────────┐                               ║\n";
        cout << "║                        │  S0     │                               ║\n";
        cout << "║                        │ (开始)  │                               ║\n";
        cout << "║                        └────┬────┘                               ║\n";
        cout << "║                             │                                     ║\n";
        cout << "║         ┌───────────────────┼───────────────────┐                 ║\n";
        cout << "║         │                   │                   │                 ║\n";
        cout << "║         ▼                   ▼                   ▼                 ║\n";
        cout << "║    ┌─────────┐         ┌─────────┐         ┌─────────┐               ║\n";
        cout << "║    │   S1    │◄─循环──│   S2    │         │   S3    │               ║\n";
        cout << "║    │ 标识符  │ 字母/   │  数字   │         │ 运算符  │               ║\n";
        cout << "║    └────┬────┘ 数字   └────┬────┘         └────┬────┘               ║\n";
        cout << "║         │                  │                   │                   ║\n";
        cout << "║         │                  │                   │                   ║\n";
        cout << "║         └──────────────────┴───────────────────┘                   ║\n";
        cout << "║                             │                                     ║\n";
        cout << "║                             ▼                                     ║\n";
        cout << "║                      ┌─────────┐                                   ║\n";
        cout << "║                      │   SF    │                                   ║\n";
        cout << "║                      │ (完成)  │                                   ║\n";
        cout << "║                      └─────────┘                                   ║\n";
        cout << "║                                                                  ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║                           状态说明                                    ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║  S0: 开始状态 - 初始状态，等待输入字符                                  ║\n";
        cout << "║  S1: 标识符状态 - 读取字母，后续可接字母或数字                           ║\n";
        cout << "║  S2: 数字状态 - 读取数字，后续只能接数字                                 ║\n";
        cout << "║  S3: 运算符状态 - 识别运算符                                             ║\n";
        cout << "║  SF: 完成状态 - Token 识别完成                                           ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║                           转换规则                                      ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║  字母    ──► S0 → S1 → ... → SF (标识符/关键字)                           ║\n";
        cout << "║  数字    ──► S0 → S2 → ... → SF (数字)                                   ║\n";
        cout << "║  运算符  ──► S0 → S3 → SF (运算符)                                      ║\n";
        cout << "║  界限符  ──► S0 → SF (界限符，单字符)                                     ║\n";
        cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
        
        waitForEnter();
    }
    
    // 绘制识别流程图
    void drawRecognitionFlowchart() {
        clearScreen();
        
        cout << "\n";
        cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║                    词法识别流程图                              ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║                                                                  ║\n";
        cout << "║                            ┌─────────┐                           ║\n";
        cout << "║                            │  开始   │                           ║\n";
        cout << "║                            └────┬────┘                           ║\n";
        cout << "║                                 │                                ║\n";
        cout << "║                                 ▼                                ║\n";
        cout << "║                        ┌─────────────────┐                        ║\n";
        cout << "║                        │   读取下一个字符  │                        ║\n";
        cout << "║                        └────────┬────────┘                        ║\n";
        cout << "║                                 │                                ║\n";
        cout << "║                                 ▼                                ║\n";
        cout << "║                       ┌───────────────────┐                      ║\n";
        cout << "║                       │    是空白字符?     │                      ║\n";
        cout << "║                       └─────────┬─────────┘                      ║\n";
        cout << "║                    ┌────────────┴────────────┐                    ║\n";
        cout << "║                    │ 是                      │ 否                  ║\n";
        cout << "║                    ▼                         ▼                    ║\n";
        cout << "║             ┌──────────────┐    ┌──────────────────────┐          ║\n";
        cout << "║             │    跳过       │    │    判断字符类型      │          ║\n";
        cout << "║             └──────┬───────┘    └──────────┬───────────┘          ║\n";
        cout << "║                    │                       │                       ║\n";
        cout << "║                    └───────────┬───────────┘                       ║\n";
        cout << "║                                │                                 ║\n";
        cout << "║              ┌─────────────────┼─────────────────┐                ║\n";
        cout << "║              │                 │                 │                ║\n";
        cout << "║              ▼                 ▼                 ▼                ║\n";
        cout << "║      ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          ║\n";
        cout << "║      │   标识符/     │  │    数字       │  │   运算符/    │          ║\n";
        cout << "║      │   关键字     │  │   常量       │  │   界限符     │          ║\n";
        cout << "║      └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          ║\n";
        cout << "║             │                 │                 │                  ║\n";
        cout << "║             └─────────────────┼─────────────────┘                  ║\n";
        cout << "║                               │                                    ║\n";
        cout << "║                               ▼                                    ║\n";
        cout << "║                      ┌─────────────────┐                           ║\n";
        cout << "║                      │   输出 Token    │                           ║\n";
        cout << "║                      └────────┬────────┘                           ║\n";
        cout << "║                               │                                    ║\n";
        cout << "║                               ▼                                    ║\n";
        cout << "║                    ┌───────────────────┐                           ║\n";
        cout << "║                    │   已到文件末尾?   │                           ║\n";
        cout << "║                    └─────────┬─────────┘                           ║\n";
        cout << "║                      ┌───────┴───────┐                             ║\n";
        cout << "║                      │ 是             │ 否                          ║\n";
        cout << "║                      ▼                ▼                              ║\n";
        cout << "║               ┌────────────┐  ┌───────────────┐                    ║\n";
        cout << "║               │    结束     │  │ 返回读取字符   │                    ║\n";
        cout << "║               └────────────┘  └───────────────┘                    ║\n";
        cout << "║                                                                  ║\n";
        cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
        
        waitForEnter();
    }
    
    // 绘制详细分类表（带颜色）
    void drawDetailedClassificationTable() {
        clearScreen();
        
        cout << "\n";
        cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║                    单词分类详细表                                ║\n";
        cout << "╠══════════════════════════════════════════════════════════════════════════╣\n";
        cout << "║                                                                  ║\n";
        cout << "║  ┌─────────────────────────────────────────────────────────────┐        ║\n";
        cout << "║  │ 1. 关键字 (KEYWORD)                                          │        ║\n";
        cout << "║  │    用途: PL/0 语言保留字，具有固定含义                        │        ║\n";
        cout << "║  │    列表: const, var, procedure, begin, end, if, then,       │        ║\n";
        cout << "║  │          while, do, call, odd, read, write                   │        ║\n";
        cout << "║  │    示例: const n = 10;  →  const 是关键字                   │        ║\n";
        cout << "║  └─────────────────────────────────────────────────────────────┘        ║\n";
        cout << "║                                                                  ║\n";
        cout << "║  ┌─────────────────────────────────────────────────────────────┐        ║\n";
        cout << "║  │ 2. 标识符 (IDENTIFIER)                                       │        ║\n";
        cout << "║  │    用途: 用户定义的名称，标识变量、过程等                    │        ║\n";
        cout << "║  │    规则: 字母开头，后接字母或数字，长度≤8                    │        ║\n";
        cout << "║  │    示例: x, y, sum, count, maxValue                         │        ║\n";
        cout << "║  └─────────────────────────────────────────────────────────────┘        ║\n";
        cout << "║                                                                  ║\n";
        cout << "║  ┌─────────────────────────────────────────────────────────────┐        ║\n";
        cout << "║  │ 3. 常数 (NUMBER)                                             │        ║\n";
        cout << "║  │    用途: 表示数值常量                                        │        ║\n";
        cout << "║  │    规则: 纯数字序列，范围 0-65535，长度≤8                    │        ║\n";
        cout << "║  │    示例: 10, 100, 32767, 65535                               │        ║\n";
        cout << "║  └─────────────────────────────────────────────────────────────┘        ║\n";
        cout << "║                                                                  ║\n";
        cout << "║  ┌─────────────────────────────────────────────────────────────┐        ║\n";
        cout << "║  │ 4. 运算符 (OPERATOR)                                         │        ║\n";
        cout << "║  │    分类:                                                     │        ║\n";
        cout << "║  │    - 算术运算符: +, -, *, /                                   │        ║\n";
        cout << "║  │    - 关系运算符: =, <>, <, >, <=, >=                         │        ║\n";
        cout << "║  │    - 赋值运算符: :=                                           │        ║\n";
        cout << "║  └─────────────────────────────────────────────────────────────┘        ║\n";
        cout << "║                                                                  ║\n";
        cout << "║  ┌─────────────────────────────────────────────────────────────┐        ║\n";
        cout << "║  │ 5. 界限符 (DELIMITER)                                        │        ║\n";
        cout << "║  │    用途: 分隔语句、参数列表等                                │        ║\n";
        cout << "║  │    列表: ; (分号), , (逗号), ( (左括号), ) (右括号), . (句号) │        ║\n";
        cout << "║  └─────────────────────────────────────────────────────────────┘        ║\n";
        cout << "║                                                                  ║\n";
        cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
        
        waitForEnter();
    }
    
    //============================================================================
    // 词法分析函数
    //============================================================================
    
    // 词法分析
    void analyze() {
        tokens.clear();
        keywordCount = identifierCount = numberCount = 
                      operatorCount = delimiterCount = 0;
        
        currentPos = 0;
        currentLine = 1;
        currentCol = 1;
        
        // 打开输出文件
        outFile.open("lexer_analysis_result.txt");
        outFile << "========================================\n";
        outFile << "       PL/0 词法分析报告\n";
        outFile << "========================================\n\n";
        outFile << "源代码:\n";
        outFile << "----------------------------------------\n";
        outFile << inputBuffer << "\n\n";
        outFile << "Token 序列:\n";
        outFile << "----------------------------------------\n";
        
        while (currentPos < inputBuffer.length()) {
            char ch = inputBuffer[currentPos];
            
            // 跳过空白
            if (isspace(ch)) {
                if (ch == '\n') {
                    currentLine++;
                    currentCol = 1;
                } else {
                    currentCol++;
                }
                currentPos++;
                currentStep = "跳过空白字符...";
                drawMainUI();
                animate();
                continue;
            }
            
            // 跳过注释
            if (ch == '/' && currentPos + 1 < inputBuffer.length() && 
                inputBuffer[currentPos + 1] == '/') {
                currentStep = "跳过单行注释...";
                while (currentPos < inputBuffer.length() && 
                       inputBuffer[currentPos] != '\n') {
                    currentPos++;
                }
                drawMainUI();
                animate();
                continue;
            }
            
            // 字母开头 -> 标识符或关键字
            if (isalpha(ch)) {
                analyzeIdentifier();
                continue;
            }
            
            // 数字开头 -> 常数
            if (isdigit(ch)) {
                analyzeNumber();
                continue;
            }
            
            // 运算符
            if (isOperator(ch)) {
                analyzeOperator();
                continue;
            }
            
            // 界限符
            if (isDelimiter(ch)) {
                analyzeDelimiter();
                continue;
            }
            
            // 未知字符
            Token errorToken;
            errorToken.type = TokenType::ERROR;
            errorToken.value = string(1, ch);
            errorToken.line = currentLine;
            errorToken.startPos = currentPos;
            errorToken.endPos = currentPos + 1;
            tokens.push_back(errorToken);
            
            currentStep = "发现非法字符: " + string(1, ch);
            currentPos++;
            currentCol++;
            drawMainUI();
            animate();
        }
        
        // 添加 EOF Token
        Token eofToken;
        eofToken.type = TokenType::END_OF_FILE;
        eofToken.value = "EOF";
        eofToken.line = currentLine;
        tokens.push_back(eofToken);
        
        currentStep = "词法分析完成！";
        drawMainUI();
        
        // 输出到文件
        for (size_t i = 0; i < tokens.size(); i++) {
            const Token& t = tokens[i];
            outFile << setw(3) << i + 1 << ". "
                    << setw(12) << getTokenTypeName(t.type)
                    << " \"" << t.value << "\""
                    << " (行 " << t.line << ")\n";
        }
        
        outFile << "\n统计信息:\n";
        outFile << "----------------------------------------\n";
        outFile << "关键字: " << keywordCount << "\n";
        outFile << "标识符: " << identifierCount << "\n";
        outFile << "常数: " << numberCount << "\n";
        outFile << "运算符: " << operatorCount << "\n";
        outFile << "界限符: " << delimiterCount << "\n";
        outFile << "总计: " << tokens.size() << "\n";
        
        outFile.close();
        
        cout << "\n分析结果已保存到 lexer_analysis_result.txt\n";
    }
    
    // 分析标识符
    void analyzeIdentifier() {
        int startPos = currentPos;
        int startCol = currentCol;
        string value;
        
        while (currentPos < inputBuffer.length() && 
               isalnum(inputBuffer[currentPos])) {
            value += inputBuffer[currentPos];
            currentPos++;
            currentCol++;
        }
        
        Token token;
        token.startPos = startPos;
        token.endPos = currentPos;
        token.line = currentLine;
        token.value = value;
        
        // 检查是否为关键字
        if (KEYWORDS.find(value) != KEYWORDS.end()) {
            token.type = TokenType::KEYWORD;
            keywordCount++;
            currentStep = "识别关键字: " + value;
        } else {
            token.type = TokenType::IDENTIFIER;
            identifierCount++;
            currentStep = "识别标识符: " + value;
        }
        
        tokens.push_back(token);
        drawMainUI();
        animate();
    }
    
    // 分析数字
    void analyzeNumber() {
        int startPos = currentPos;
        int startCol = currentCol;
        string value;
        
        while (currentPos < inputBuffer.length() && 
               isdigit(inputBuffer[currentPos])) {
            value += inputBuffer[currentPos];
            currentPos++;
            currentCol++;
        }
        
        Token token;
        token.type = TokenType::NUMBER;
        token.value = value;
        token.line = currentLine;
        token.startPos = startPos;
        token.endPos = currentPos;
        tokens.push_back(token);
        
        numberCount++;
        currentStep = "识别常数: " + value;
        drawMainUI();
        animate();
    }
    
    // 分析运算符
    void analyzeOperator() {
        int startPos = currentPos;
        string value;
        value += inputBuffer[currentPos];
        currentPos++;
        currentCol++;
        
        // 检查双字符运算符
        if (currentPos < inputBuffer.length()) {
            char nextCh = inputBuffer[currentPos];
            string twoChar = value + nextCh;
            
            if (twoChar == "<=" || twoChar == ">=" || twoChar == ":=" || 
                twoChar == "<>") {
                value = twoChar;
                currentPos++;
                currentCol++;
            }
        }
        
        Token token;
        token.type = TokenType::OPERATOR;
        token.value = value;
        token.line = currentLine;
        token.startPos = startPos;
        token.endPos = currentPos;
        tokens.push_back(token);
        
        operatorCount++;
        currentStep = "识别运算符: " + value;
        drawMainUI();
        animate();
    }
    
    // 分析界限符
    void analyzeDelimiter() {
        int startPos = currentPos;
        string value;
        value += inputBuffer[currentPos];
        currentPos++;
        currentCol++;
        
        Token token;
        token.type = TokenType::DELIMITER;
        token.value = value;
        token.line = currentLine;
        token.startPos = startPos;
        token.endPos = currentPos;
        tokens.push_back(token);
        
        delimiterCount++;
        currentStep = "识别界限符: " + value;
        drawMainUI();
        animate();
    }
    
    //============================================================================
    // 主菜单
    //============================================================================
    
    void showMenu() {
        clearScreen();
        
        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════════╗\n";
        cout << "║           PL/0 词法分析可视化系统 - 主菜单                  ║\n";
        cout << "╠════════════════════════════════════════════════════════════╣\n";
        cout << "║                                                            ║\n";
        cout << "║   1. 开始词法分析                                           ║\n";
        cout << "║   2. 查看状态转换图 (DFA)                                   ║\n";
        cout << "║   3. 查看识别流程图                                         ║\n";
        cout << "║   4. 查看单词分类详细表                                     ║\n";
        cout << "║   5. 保存分析结果                                           ║\n";
        cout << "║   0. 退出程序                                               ║\n";
        cout << "║                                                            ║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n";
        cout << "\n请选择操作: ";
    }
    
    void run() {
        int choice;
        
        do {
            showMenu();
            cin >> choice;
            cin.ignore(); // 清除换行符
            
            switch (choice) {
                case 1:
                    analyze();
                    break;
                case 2:
                    drawStateTransitionDiagram();
                    break;
                case 3:
                    drawRecognitionFlowchart();
                    break;
                case 4:
                    drawDetailedClassificationTable();
                    break;
                case 5:
                    saveResults();
                    break;
                case 0:
                    cout << "\n感谢使用，再见！\n";
                    break;
                default:
                    cout << "\n无效选择，请重试！\n";
                    waitForEnter();
            }
        } while (choice != 0);
    }
    
    void saveResults() {
        if (tokens.empty()) {
            cout << "\n请先进行词法分析！\n";
            waitForEnter();
            return;
        }
        
        ofstream file("lexer_analysis_result.txt");
        if (!file.is_open()) {
            cout << "\n无法创建文件！\n";
            waitForEnter();
            return;
        }
        
        file << "========================================\n";
        file << "       PL/0 词法分析报告\n";
        file << "========================================\n\n";
        
        file << "源代码:\n";
        file << "----------------------------------------\n";
        file << inputBuffer << "\n\n";
        
        file << "Token 序列:\n";
        file << "----------------------------------------\n";
        for (size_t i = 0; i < tokens.size(); i++) {
            const Token& t = tokens[i];
            file << setw(3) << i + 1 << ". "
                 << setw(12) << getTokenTypeName(t.type)
                 << " \"" << t.value << "\""
                 << " (行 " << t.line << ")\n";
        }
        
        file << "\n统计信息:\n";
        file << "----------------------------------------\n";
        file << "关键字: " << keywordCount << "\n";
        file << "标识符: " << identifierCount << "\n";
        file << "常数: " << numberCount << "\n";
        file << "运算符: " << operatorCount << "\n";
        file << "界限符: " << delimiterCount << "\n";
        file << "总计: " << tokens.size() << "\n";
        
        file.close();
        
        cout << "\n结果已保存到 lexer_analysis_result.txt\n";
        waitForEnter();
    }
};

//============================================================================
// 主函数
//============================================================================

int main() {
    LexerVisualizer visualizer;
    
    // 从控制台输入代码
    visualizer.inputFromConsole();
    
    // 运行主循环
    visualizer.run();
    
    return 0;
}
