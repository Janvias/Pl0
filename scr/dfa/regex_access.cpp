/**
 * @file regex_access.cpp
 * @brief PL/0正则表达式访问接口实现
 * @details 实现获取PL/0语言各类token正则表达式的函数
 *          从pl0_regex.h中获取正则定义并进行预处理
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "regex_access.h"
#include "../pl0_regex/pl0_regex.h"

extern "C" {

/**
 * @brief 获取标识符的正则表达式
 * @details 从PL0Regex::REG_IDENT获取，并移除外围括号
 */
const char* get_regex_ident() {
    static std::string ident = PL0Regex::REG_IDENT;
    // 移除外围括号（如果有）
    if (ident.size() >= 2 && ident.front() == '(' && ident.back() == ')') {
        ident = ident.substr(1, ident.size() - 2);
    }
    return ident.c_str();
}

/**
 * @brief 获取数字的正则表达式
 * @details 从PL0Regex::REG_NUMBER获取，并移除外围括号
 */
const char* get_regex_number() {
    static std::string number = PL0Regex::REG_NUMBER;
    // 移除外围括号（如果有）
    if (number.size() >= 2 && number.front() == '(' && number.back() == ')') {
        number = number.substr(1, number.size() - 2);
    }
    return number.c_str();
}

/**
 * @brief 获取关键字的正则表达式
 * @details 从PL0Regex::REG_KEYWORD获取，并移除外围括号
 */
const char* get_regex_keyword() {
    static std::string keyword = PL0Regex::REG_KEYWORD;
    // 移除外围括号（如果有）
    if (keyword.size() >= 2 && keyword.front() == '(' && keyword.back() == ')') {
        keyword = keyword.substr(1, keyword.size() - 2);
    }
    return keyword.c_str();
}

/**
 * @brief 获取运算符的正则表达式
 * @details 合并双字符运算符和单字符运算符
 *          从PL0Regex::REG_DOUBLE_OP和PL0Regex::REG_SINGLE_OP获取
 */
const char* get_regex_operator() {
    // 合并双字符运算符和单字符运算符（移除外围括号）
    static std::string op;
    std::string double_op = PL0Regex::REG_DOUBLE_OP;
    std::string single_op = PL0Regex::REG_SINGLE_OP;
    
    // 移除外围括号（如果有）
    if (double_op.size() >= 2 && double_op.front() == '(' && double_op.back() == ')') {
        double_op = double_op.substr(1, double_op.size() - 2);
    }
    if (single_op.size() >= 2 && single_op.front() == '(' && single_op.back() == ')') {
        single_op = single_op.substr(1, single_op.size() - 2);
    }
    
    // 合并运算符，使用选择运算符'|'连接
    op = double_op + "|" + single_op;
    return op.c_str();
}

/**
 * @brief 获取分隔符的正则表达式
 * @details 定义PL/0语言的分隔符：逗号、分号、点、左括号、右括号
 */
const char* get_regex_delimiter() {
    // 从单字符运算符中提取分隔符
    static std::string delimiter = ",|;|\\.|\\(|\\)";
    return delimiter.c_str();
}

}
