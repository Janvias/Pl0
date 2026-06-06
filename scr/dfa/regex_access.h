#ifndef REGEX_ACCESS_H
#define REGEX_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// 获取标识符的正则表达式
const char* get_regex_ident();

// 获取数字的正则表达式
const char* get_regex_number();

// 获取关键字的正则表达式
const char* get_regex_keyword();

// 获取运算符的正则表达式
const char* get_regex_operator();

// 获取分隔符的正则表达式
const char* get_regex_delimiter();

#ifdef __cplusplus
}
#endif

#endif // REGEX_ACCESS_H
