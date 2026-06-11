/**
 * @file pl0_regex.h
 * @brief PL/0 Regular Expression Definitions
 * @details Defines regular expression patterns for PL/0 lexical elements
 *          used by the DFA-based lexer validation.
 */

#ifndef PL0_REGEX_H
#define PL0_REGEX_H

#include <string>

namespace PL0Regex {

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
