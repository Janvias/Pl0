/**
 * @file pl0_lexer.hpp
 * @brief PL/0 Compiler Lexer Class
 * @details This class implements lexical analysis for the PL/0 compiler.
 *          It reads source code character by character, identifies tokens,
 *          and performs DFA-based validation for identifiers and numbers.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_LEXER_HPP
#define PL0_LEXER_HPP

#include "pl0_core.hpp"

namespace PL0 {

// Forward declaration for DFA wrappers
extern "C" {
    void* create_dfa_from_regex(const char* regex);
    int dfa_accepts(void* dfa, const char* str);
    void destroy_dfa(void* dfa);
    const char* get_regex_ident();
    const char* get_regex_number();
}

/**
 * @class Lexer
 * @brief Lexical analyzer for PL/0 source code
 * @details The Lexer class reads PL/0 source files, tokenizes the input,
 *          and provides methods for retrieving tokens sequentially.
 */
class Lexer {
public:
    Lexer(const std::string& filename);
    ~Lexer();

    // Token获取
    Token getNextToken();
    Token peekToken();
    void ungetToken(const Token& t);

    // 词法分析
    std::vector<Token> analyze();

    // 错误处理
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

    // 可视化输出
    void printTokens(std::ostream& os) const;
    void printStatistics(std::ostream& os) const;
    void printClassificationTable(std::ostream& os) const;

private:
    std::ifstream file_;
    char currentChar_;
    int currentLine_;
    bool hasError_;
    std::string errorMessage_;

    // Token缓冲区
    std::vector<Token> tokenBuffer_;
    size_t bufferPosition_;

    // 关键字表
    static const std::unordered_map<std::string, bool> keywords_;

    // DFA指针
    void* dfaIdent_;
    void* dfaNumber_;

    // 私有方法
    void readChar();
    void skipWhitespace();
    Token analyzeIdentifier();
    Token analyzeNumber();
    Token analyzeOperator();
    Token analyzeDelimiter();
    bool isKeyword(const std::string& str) const;

    // DFA相关
    void initDFA();
    void destroyDFA();
    bool validateWithDFA(void* dfa, const std::string& str);
};

} // namespace PL0

#endif // PL0_LEXER_HPP
