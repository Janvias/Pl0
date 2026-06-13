/**
 * @file lexer.cpp
 * @brief 词法分析器实现
 * @details 实现PL/0源代码的词法分析，包括Token识别、DFA验证和错误处理
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "../include/pl0_lexer.hpp"
#include "../dfa/dfa_wrapper.h"
#include "../dfa/regex_access.h"

namespace PL0 {

//============================================================================
// 关键字表初始化
//============================================================================

const std::unordered_map<std::string, bool> Lexer::keywords_ = {
    {"const", true}, {"var", true}, {"procedure", true},
    {"begin", true}, {"end", true}, {"if", true},
    {"then", true}, {"while", true}, {"do", true},
    {"call", true}, {"read", true}, {"write", true},
    {"odd", true}
};

//============================================================================
// Lexer 实现
//============================================================================

Lexer::Lexer(const std::string& filename)
    : currentChar_(0), currentLine_(1), hasError_(false),
      bufferPosition_(0), dfaIdent_(nullptr), dfaNumber_(nullptr) {
    file_.open(filename, std::ios::in);
    if (!file_.is_open()) {
        hasError_ = true;
        errorMessage_ = "Cannot open file: " + filename;
        return;
    }
    readChar();
}

Lexer::~Lexer() {
    destroyDFA();
    if (file_.is_open()) {
        file_.close();
    }
}

//============================================================================
// 读取下一个字符
//============================================================================

void Lexer::readChar() {
    currentChar_ = static_cast<char>(file_.get());
    if (currentChar_ == '\n') {
        currentLine_++;
    }
}

//============================================================================
// 跳过空白字符
//============================================================================

void Lexer::skipWhitespace() {
    // MinGW workaround: file_.eof()可能不可靠触发；
    // 使用显式EOF字符检测作为辅助保护
    while (!file_.eof() &&
           currentChar_ != static_cast<char>(std::char_traits<char>::eof()) &&
           std::isspace(static_cast<unsigned char>(currentChar_))) {
        readChar();
    }
}

//============================================================================
// 分析标识符
//============================================================================

Token Lexer::analyzeIdentifier() {
    std::string value;
    while (!file_.eof() && std::isalnum(currentChar_)) {
        value += currentChar_;
        readChar();
    }

    if (isKeyword(value)) {
        return Token(TokenType::KEYWORD, value, currentLine_);
    }

    // DFA验证
    if (dfaIdent_ && !validateWithDFA(dfaIdent_, value)) {
        return Token(TokenType::ERROR, "Invalid identifier: " + value, currentLine_);
    }

    if (value.length() > MAX_IDENTIFIER_LENGTH) {
        return Token(TokenType::ERROR, "Identifier too long: " + value, currentLine_);
    }

    return Token(TokenType::IDENTIFIER, value, currentLine_);
}

//============================================================================
// 分析数字
//============================================================================

Token Lexer::analyzeNumber() {
    std::string value;
    while (!file_.eof() && std::isdigit(currentChar_)) {
        value += currentChar_;
        readChar();
    }

    if (!file_.eof() && std::isalpha(currentChar_)) {
        while (!file_.eof() && std::isalnum(currentChar_)) {
            value += currentChar_;
            readChar();
        }
        return Token(TokenType::ERROR, "Invalid word: " + value, currentLine_);
    }

    // DFA验证
    if (dfaNumber_ && !validateWithDFA(dfaNumber_, value)) {
        return Token(TokenType::ERROR, "Invalid number: " + value, currentLine_);
    }

    if (value.length() > MAX_NUMBER_LENGTH) {
        return Token(TokenType::ERROR, "Number too long: " + value, currentLine_);
    }

    return Token(TokenType::NUMBER, value, currentLine_);
}

Token Lexer::analyzeOperator() {
    std::string value;
    value += currentChar_;
    char c1 = currentChar_;
    readChar();

    // 双字符运算符
    if ((c1 == '<' && currentChar_ == '=') ||
        (c1 == '>' && currentChar_ == '=') ||
        (c1 == '<' && currentChar_ == '>')) {
        value += currentChar_;
        readChar();
    }

    // 处理注释
    if (c1 == '/') {
        if (currentChar_ == '/') {
            // 单行注释
            while (!file_.eof() && currentChar_ != '\n') {
                readChar();
            }
            return getNextToken();
        }
        if (currentChar_ == '*') {
            // 多行注释
            readChar();
            while (!file_.eof()) {
                if (currentChar_ == '*' && file_.peek() == '/') {
                    readChar();
                    readChar();
                    break;
                }
                readChar();
            }
            return getNextToken();
        }
    }

    return Token(TokenType::OPERATOR, value, currentLine_);
}

Token Lexer::analyzeDelimiter() {
    std::string value;
    value += currentChar_;
    char c1 = currentChar_;
    readChar();

    if (c1 == ':' && currentChar_ == '=') {
        value += currentChar_;
        readChar();
        return Token(TokenType::OPERATOR, value, currentLine_);
    }

    return Token(TokenType::DELIMITER, value, currentLine_);
}

Token Lexer::getNextToken() {
    if (bufferPosition_ < tokenBuffer_.size()) {
        return tokenBuffer_[bufferPosition_++];
    }

    skipWhitespace();

    // MinGW workaround: also check for real EOF character
    if (file_.eof() ||
        currentChar_ == static_cast<char>(std::char_traits<char>::eof())) {
        return Token(TokenType::END_OF_FILE, "EOF", currentLine_);
    }

    Token token;
    if (std::isalpha(currentChar_)) {
        token = analyzeIdentifier();
    } else if (std::isdigit(currentChar_)) {
        token = analyzeNumber();
    } else if (currentChar_ == '+' || currentChar_ == '-' ||
               currentChar_ == '*' || currentChar_ == '/' ||
               currentChar_ == '=' || currentChar_ == '#' ||
               currentChar_ == '<' || currentChar_ == '>') {
        token = analyzeOperator();
    } else if (currentChar_ == ';' || currentChar_ == ',' ||
               currentChar_ == '.' || currentChar_ == '(' ||
               currentChar_ == ')' || currentChar_ == ':') {
        token = analyzeDelimiter();
    } else {
        // 错误恢复：记录非法字符，跳过并继续词法分析
        char illegalChar = currentChar_;
        recoverFromError(illegalChar);
        readChar();
        token = Token(TokenType::ERROR,
                      "Illegal character: '" + std::string(1, illegalChar) + "'",
                      currentLine_);
    }

    tokenBuffer_.push_back(token);
    bufferPosition_++;
    return token;
}

Token Lexer::peekToken() {
    Token t = getNextToken();
    bufferPosition_--;
    return t;
}

void Lexer::ungetToken(const Token& t) {
    if (bufferPosition_ > 0) {
        bufferPosition_--;
        tokenBuffer_[bufferPosition_] = t;
    }
}

std::vector<Token> Lexer::analyze() {
    initDFA();
    std::vector<Token> tokens;
    Token token;

    while (true) {
        token = getNextToken();
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
        tokens.push_back(token);
    }

    return tokens;
}

bool Lexer::isKeyword(const std::string& str) const {
    return keywords_.find(str) != keywords_.end();
}

void Lexer::recoverFromError(char illegalChar) {
    // 记录错误token到错误列表
    std::string msg = "Illegal character: '" + std::string(1, illegalChar) + "'";
    lexErrors_.push_back(Token(TokenType::ERROR, msg, currentLine_));

    // 跳过当前非法字符已在getNextToken()中处理（readChar调用后）
    // 错误恢复策略：仅跳过单个非法字符，继续正常词法分析
    // 这足以处理绝大多数词法错误（如输入了@、$等符号）
}

void Lexer::initDFA() {
    dfaIdent_ = create_dfa_from_regex(get_regex_ident());
    dfaNumber_ = create_dfa_from_regex(get_regex_number());
}

void Lexer::destroyDFA() {
    if (dfaIdent_) {
        destroy_dfa(dfaIdent_);
        dfaIdent_ = nullptr;
    }
    if (dfaNumber_) {
        destroy_dfa(dfaNumber_);
        dfaNumber_ = nullptr;
    }
}

bool Lexer::validateWithDFA(void* dfa, const std::string& str) {
    if (!dfa) return true;
    return dfa_accepts(dfa, str.c_str()) != 0;
}

void Lexer::printTokens(std::ostream& os) const {
    os << "\n===== LEXICAL ANALYSIS RESULT =====\n";
    for (const auto& t : tokenBuffer_) {
        if (t.type == TokenType::END_OF_FILE) break;
        switch (t.type) {
            case TokenType::KEYWORD:    os << "(keyword," << t.value << ")\n"; break;
            case TokenType::IDENTIFIER: os << "(identifier," << t.value << ")\n"; break;
            case TokenType::NUMBER:     os << "(number," << t.value << ")\n"; break;
            case TokenType::OPERATOR:   os << "(operator," << t.value << ")\n"; break;
            case TokenType::DELIMITER: os << "(delimiter," << t.value << ")\n"; break;
            default: break;
        }
    }
}

void Lexer::printStatistics(std::ostream& os) const {
    int count[5] = {0};
    for (const auto& t : tokenBuffer_) {
        switch (t.type) {
            case TokenType::KEYWORD:    count[0]++; break;
            case TokenType::IDENTIFIER: count[1]++; break;
            case TokenType::NUMBER:    count[2]++; break;
            case TokenType::OPERATOR:   count[3]++; break;
            case TokenType::DELIMITER: count[4]++; break;
            default: break;
        }
    }

    os << "\n===== TOKEN STATISTICS =====\n";
    os << "keyword: " << count[0] << "\n";
    os << "identifier: " << count[1] << "\n";
    os << "number: " << count[2] << "\n";
    os << "operator: " << count[3] << "\n";
    os << "delimiter: " << count[4] << "\n";
    os << "total: " << (count[0] + count[1] + count[2] + count[3] + count[4]) << "\n";
}

void Lexer::printClassificationTable(std::ostream& os) const {
    os << "\n===== WORD CLASSIFICATION TABLE =====\n";
    os << std::left << std::setw(6) << "Index"
       << std::setw(12) << "Type"
       << std::setw(15) << "Value"
       << std::setw(6) << "Line" << "\n";
    os << "------ ------------ --------------- ------\n";

    int index = 0;
    for (const auto& t : tokenBuffer_) {
        if (t.type == TokenType::END_OF_FILE) break;
        std::string typeStr;
        switch (t.type) {
            case TokenType::KEYWORD:    typeStr = "KEYWORD"; break;
            case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
            case TokenType::NUMBER:     typeStr = "NUMBER"; break;
            case TokenType::OPERATOR:   typeStr = "OPERATOR"; break;
            case TokenType::DELIMITER:  typeStr = "DELIMITER"; break;
            default:                    typeStr = "ERROR"; break;
        }
        os << std::left << std::setw(6) << index++
           << std::setw(12) << typeStr
           << std::setw(15) << t.value
           << std::setw(6) << t.line << "\n";
    }
}

} // namespace PL0
