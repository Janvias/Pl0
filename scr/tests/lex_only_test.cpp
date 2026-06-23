/**
 * @file lex_only_test.cpp
 * @brief 独立词法分析测试 — 仅运行词法分析，输出所有Token含错误
 */
#include "../include/pl0_lexer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: lex_only_test <file.pl0>\n"; return 1; }

    PL0::Lexer lexer(argv[1]);
    if (lexer.hasError()) { std::cerr << lexer.getErrorMessage() << "\n"; return 1; }

    auto tokens = lexer.analyze();

    std::cout << "=== Token Stream ===\n";
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& t = tokens[i];
        if (t.type == PL0::TokenType::END_OF_FILE) break;
        std::string typeStr;
        switch (t.type) {
            case PL0::TokenType::KEYWORD:    typeStr = "KEYWORD";    break;
            case PL0::TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
            case PL0::TokenType::NUMBER:     typeStr = "NUMBER";     break;
            case PL0::TokenType::OPERATOR:   typeStr = "OPERATOR";   break;
            case PL0::TokenType::DELIMITER:  typeStr = "DELIMITER";  break;
            case PL0::TokenType::ERROR:      typeStr = "ERROR";      break;
            default: typeStr = "UNKNOWN"; break;
        }
        std::cout << "[" << i << "] (" << typeStr << ", " << t.value << ") Line:" << t.line << "\n";
    }

    // 错误列表
    const auto& errors = lexer.getLexErrors();
    if (!errors.empty()) {
        std::cout << "\n=== Lexical Errors Detected (" << errors.size() << ") ===\n";
        for (const auto& e : errors)
            std::cout << "Line " << e.line << ": " << e.value << "\n";
    } else {
        std::cout << "\n(No lexical errors)\n";
    }

    lexer.printStatistics(std::cout);
    lexer.printClassificationTable(std::cout);
    return 0;
}
