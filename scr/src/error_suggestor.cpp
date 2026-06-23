/**
 * @file error_suggestor.cpp
 * @brief 错误修复建议器实现 — 基于模式的错误诊断
 * @details 将编译器错误消息与常见PL/0错误知识库进行匹配，
 *          生成结构化的修复建议。支持缺失Token、标识符错误、
 *          表达式错误、语法错误和结构错误等多种错误类型。
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "../include/pl0_error_suggestor.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace PL0 {

//============================================================================
// 构造函数
//============================================================================

ErrorSuggestor::ErrorSuggestor() {
    initKnowledgeBase();
}

//============================================================================
// 知识库初始化
//============================================================================
// 定义常见PL/0编译错误的模式匹配规则和修复建议：
// - 缺失Token类：缺少分号、关键字、操作符等
// - 标识符类：未定义、重复声明等
// - 表达式类：括号不匹配、非法Token等
// - 语法类：期望标识符、数字等
// - 结构类：意外Token等

void ErrorSuggestor::initKnowledgeBase() {
    knowledgeBase_ = {
        // ---- Missing tokens ----
        {
            "Expected '.' at end",
            "Missing period '.' at end of program",
            {"Add '.' at the end of the last line"}
        },
        {
            "Expected ';'",
            "Missing semicolon ';' in statement or declaration",
            {"Add semicolon ';' at current position",
             "Check if previous line ends with semicolon"}
        },
        {
            "Expected 'then'",
            "Missing 'then' keyword in if statement",
            {"Add 'then' after the condition expression",
             "Check if statement format: if <condition> then <statement>"}
        },
        {
            "Expected 'do'",
            "Missing 'do' keyword in while statement",
            {"Add 'do' after the condition expression",
             "Check if statement format: while <condition> do <statement>"}
        },
        {
            "Expected 'end'",
            "Missing matching 'end' keyword",
            {"Add 'end' at the end of statement block",
             "Check if begin-end pairs are properly matched"}
        },
        {
            "Expected ':='",
            "Missing ':=' operator in assignment",
            {"Use ':=' instead of '=' for assignment",
             "In PL/0, x = 10 should be written as x := 10"}
        },

        // ---- Identifiers ----
        {
            "Undefined identifier",
            "Using undeclared or undefined identifier",
            {"Check if variable name is spelled correctly",
             "Declare the variable before use (var declaration)",
             "Verify identifier is in scope"}
        },
        {
            "Redeclared",
            "Identifier redeclared",
            {"Remove duplicate declaration or use different variable name",
             "Check if declared multiple times in the same scope"}
        },

        // ---- Expression errors ----
        {
            "Expected ')'",
            "Unbalanced parentheses, missing closing ')'",
            {"Add ')' at the end of expression",
             "Check if parentheses are properly matched"}
        },
        {
            "Unexpected token in expression",
            "Invalid token in expression",
            {"Check if expression syntax is correct",
             "Expression consists of identifiers, numbers, operators and parentheses",
             "Example: x + 5 * (y - 2)"}
        },

        // ---- Syntax errors ----
        {
            "Expected identifier",
            "Missing identifier (variable or procedure name)",
            {"Add valid identifier at current position",
             "Identifier must start with letter, max 8 characters"}
        },
        {
            "Expected number",
            "Missing numeric constant",
            {"Add number at current position",
             "Constant declaration format: const identifier = number"}
        },

        // ---- Structure errors ----
        {
            "Unexpected token",
            "Unexpected token, program structure may be incorrect",
            {"Check if syntax at current position follows PL/0 rules",
             "Verify keyword spelling (const, var, begin, end, etc.)",
             "Review syntax structure of nearby code"}
        },
    };
}

//============================================================================
// 模式匹配
//============================================================================
// 使用大小写不敏感的子串匹配，判断错误消息是否符合某一错误模式

bool ErrorSuggestor::matchPattern(const std::string& msg,
                                   const std::string& pattern) const {
    // Case-insensitive substring match
    auto it = std::search(
        msg.begin(), msg.end(),
        pattern.begin(), pattern.end(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); }
    );
    return it != msg.end();
}

//============================================================================
// 获取建议
//============================================================================
// 根据错误消息查找匹配的错误模式，返回对应的分析和修复建议列表

std::vector<std::string> ErrorSuggestor::getSuggestions(
    const std::string& errorMsg) const {

    std::vector<std::string> suggestions;

    for (const auto& fix : knowledgeBase_) {
        if (matchPattern(errorMsg, fix.pattern)) {
            suggestions.push_back("[Analysis] " + fix.description);
            for (size_t i = 0; i < fix.fixes.size(); i++) {
                suggestions.push_back("  " + std::to_string(i + 1) + ". " +
                                      fix.fixes[i]);
            }
        }
    }

    if (suggestions.empty()) {
        suggestions.push_back("[Analysis] Cannot automatically identify this error type");
        suggestions.push_back("  1. Check code against PL/0 syntax rules");
        suggestions.push_back("  2. Verify keyword spelling and symbols");
        suggestions.push_back("  3. Confirm statement structure and nesting levels match");
    }

    return suggestions;
}

//============================================================================
// 打印格式化的建议
//============================================================================
// 将错误建议以友好的格式输出到指定流

void ErrorSuggestor::printSuggestions(std::ostream& os,
                                       const std::string& errorMsg) const {
    auto suggestions = getSuggestions(errorMsg);

    os << "\n========================================\n";
    os << "           Error Fix Suggestions\n";
    os << "========================================\n";

    for (const auto& s : suggestions) {
        os << s << "\n";
    }

    os << "========================================\n";
}

} // namespace PL0
