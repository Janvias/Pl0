#include "regex_access.h"
#include "../pl0_regex/pl0_regex.h"

extern "C" {

const char* get_regex_ident() {
    static std::string ident = PL0Regex::REG_IDENT;
    // 移除括号
    if (ident.size() >= 2 && ident.front() == '(' && ident.back() == ')') {
        ident = ident.substr(1, ident.size() - 2);
    }
    return ident.c_str();
}

const char* get_regex_number() {
    static std::string number = PL0Regex::REG_NUMBER;
    // 移除括号
    if (number.size() >= 2 && number.front() == '(' && number.back() == ')') {
        number = number.substr(1, number.size() - 2);
    }
    return number.c_str();
}

const char* get_regex_keyword() {
    static std::string keyword = PL0Regex::REG_KEYWORD;
    // 移除括号
    if (keyword.size() >= 2 && keyword.front() == '(' && keyword.back() == ')') {
        keyword = keyword.substr(1, keyword.size() - 2);
    }
    return keyword.c_str();
}

const char* get_regex_operator() {
    // 合并双字符运算符和单字符运算符（去除括号）
    static std::string op;
    std::string double_op = PL0Regex::REG_DOUBLE_OP;
    std::string single_op = PL0Regex::REG_SINGLE_OP;
    
    // 移除括号
    if (double_op.size() >= 2 && double_op.front() == '(' && double_op.back() == ')') {
        double_op = double_op.substr(1, double_op.size() - 2);
    }
    if (single_op.size() >= 2 && single_op.front() == '(' && single_op.back() == ')') {
        single_op = single_op.substr(1, single_op.size() - 2);
    }
    
    // 合并运算符，去除分隔符部分，保留运算符
    op = double_op + "|" + single_op;
    return op.c_str();
}

const char* get_regex_delimiter() {
    // 从单字符运算符中提取分隔符
    static std::string delimiter = ",|;|\\.|\\(|\\)";
    return delimiter.c_str();
}

}
