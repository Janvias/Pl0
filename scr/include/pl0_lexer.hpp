/**
 * @file pl0_lexer.hpp
 * @brief PL/0编译器词法分析器类
 * @details 本类实现PL/0编译器的词法分析功能，
 *          逐字符读取源代码，识别Token，
 *          并使用DFA验证标识符和数字
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_LEXER_HPP
#define PL0_LEXER_HPP

#include "pl0_core.hpp"

// DFA类型的前向声明（全局命名空间）
class DFA;
class NFA;

namespace PL0 {

// DFA封装函数的前向声明
extern "C" {
    void* create_dfa_from_regex(const char* regex);
    int dfa_accepts(void* dfa, const char* str);
    void destroy_dfa(void* dfa);
    const char* get_regex_ident();
    const char* get_regex_number();
}

/**
 * @class Lexer
 * @brief PL/0源代码词法分析器
 * @details Lexer类读取PL/0源文件，对输入进行词法分析，
 *          并提供顺序获取Token的方法
 */
class Lexer {
public:
    Lexer(const std::string& filename);
    ~Lexer();

    // Token获取
    Token getNextToken();       // 获取下一个Token
    Token peekToken();          // 预览下一个Token
    void ungetToken(const Token& t);  // 回退Token

    // 词法分析
    std::vector<Token> analyze();  // 执行完整词法分析

    // 错误处理
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }
    const std::vector<Token>& getLexErrors() const { return lexErrors_; }
    int getErrorCount() const { return static_cast<int>(lexErrors_.size()); }

    // 错误恢复
    void recoverFromError(char illegalChar);  // 从非法字符恢复

    // DFA访问器（用于可视化输出）
    const DFA* getIdentDFA() const { return static_cast<const DFA*>(dfaIdent_); }
    const DFA* getNumberDFA() const { return static_cast<const DFA*>(dfaNumber_); }

    // Token缓冲区访问
    const std::vector<Token>& getTokenBuffer() const { return tokenBuffer_; }

    // 可视化输出
    void printTokens(std::ostream& os) const;         // 输出Token列表
    void printStatistics(std::ostream& os) const;     // 输出统计信息
    void printClassificationTable(std::ostream& os) const;  // 输出分类表

private:
    std::ifstream file_;       // 输入文件流
    char currentChar_;         // 当前字符
    int currentLine_;          // 当前行号
    bool hasError_;            // 是否有错误
    std::string errorMessage_; // 错误消息

    // Token缓冲区
    std::vector<Token> tokenBuffer_;  // Token缓冲区
    size_t bufferPosition_;           // 缓冲区位置

    // 词法错误记录
    std::vector<Token> lexErrors_;    // 词法错误列表

    // 关键字表
    static const std::unordered_map<std::string, bool> keywords_;

    // DFA指针
    void* dfaIdent_;   // 标识符DFA
    void* dfaNumber_;  // 数字DFA

    // 私有方法
    void readChar();            // 读取下一个字符
    void skipWhitespace();      // 跳过空白字符
    Token analyzeIdentifier();  // 分析标识符
    Token analyzeNumber();      // 分析数字
    Token analyzeOperator();    // 分析运算符
    Token analyzeDelimiter();   // 分析分隔符
    bool isKeyword(const std::string& str) const;  // 判断是否为关键字

    // DFA相关
    void initDFA();             // 初始化DFA
    void destroyDFA();          // 销毁DFA
    bool validateWithDFA(void* dfa, const std::string& str);  // DFA验证
};

} // namespace PL0

#endif // PL0_LEXER_HPP
