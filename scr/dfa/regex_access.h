/**
 * @file regex_access.h
 * @brief PL/0正则表达式访问接口（C语言兼容）
 * @details 提供获取PL/0语言各类token正则表达式的C语言接口
 *          包括标识符、数字、关键字、运算符和分隔符的正则定义
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef REGEX_ACCESS_H
#define REGEX_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取标识符的正则表达式
 * @return 标识符正则表达式字符串
 */
const char* get_regex_ident();

/**
 * @brief 获取数字的正则表达式
 * @return 数字正则表达式字符串
 */
const char* get_regex_number();

/**
 * @brief 获取关键字的正则表达式
 * @return 关键字正则表达式字符串
 */
const char* get_regex_keyword();

/**
 * @brief 获取运算符的正则表达式
 * @return 运算符正则表达式字符串
 */
const char* get_regex_operator();

/**
 * @brief 获取分隔符的正则表达式
 * @return 分隔符正则表达式字符串
 */
const char* get_regex_delimiter();

#ifdef __cplusplus
}
#endif

#endif // REGEX_ACCESS_H
