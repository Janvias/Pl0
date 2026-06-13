/**
 * @file pl0_regex.h
 * @brief PL/0正则表达式定义
 * @details 定义PL/0词法元素的正则表达式模式，
 *          用于DFA词法分析器验证
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_REGEX_H
#define PL0_REGEX_H

#include <string>

namespace PL0Regex {

//============================================================================
// 正则表达式模式定义
//============================================================================

// 标识符: 字母开头，后跟字母或数字（最多8个字符）
static const std::string REG_IDENT =
    "([a-zA-Z]|[a-zA-Z][a-zA-Z0-9]|[a-zA-Z][a-zA-Z0-9]{2}|"
    "[a-zA-Z][a-zA-Z0-9]{3}|[a-zA-Z][a-zA-Z0-9]{4}|"
    "[a-zA-Z][a-zA-Z0-9]{5}|[a-zA-Z][a-zA-Z0-9]{6}|"
    "[a-zA-Z][a-zA-Z0-9]{7})";

// 数字: 一位或多位数字（最多8位）
static const std::string REG_NUMBER =
    "([0-9]|[0-9]{2}|[0-9]{3}|[0-9]{4}|[0-9]{5}|[0-9]{6}|[0-9]{7}|[0-9]{8})";

// 关键字
static const std::string REG_KEYWORD =
    "(const|var|procedure|begin|end|if|then|while|do|call|read|write|odd)";

// 双字符运算符
static const std::string REG_DOUBLE_OP = "(<=|>=|:=)";

// 单字符运算符和分隔符
static const std::string REG_SINGLE_OP = "(\\+|-|\\*|/|=|#|<|>)";

} // namespace PL0Regex

#endif // PL0_REGEX_H
