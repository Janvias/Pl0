/**
 * @file dfa_wrapper.h
 * @brief DFA封装接口（C语言兼容）
 * @details 提供C语言接口，用于从正则表达式创建DFA并测试字符串匹配
 *          使用extern "C"确保C和C++代码可以互操作
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef DFA_WRAPPER_H
#define DFA_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 从正则表达式创建DFA
 * @details 完整流程：正则表达式 -> 后缀 -> NFA -> DFA -> 最小化DFA
 * @param regex 正则表达式字符串
 * @return DFA指针（void*形式，供C语言使用）
 */
void* create_dfa_from_regex(const char* regex);

/**
 * @brief 测试字符串是否被DFA接受
 * @param dfa DFA指针
 * @param str 待测试字符串
 * @return 1表示接受，0表示不接受
 */
int dfa_accepts(void* dfa, const char* str);

/**
 * @brief 销毁DFA
 * @param dfa DFA指针
 */
void destroy_dfa(void* dfa);

#ifdef __cplusplus
}
#endif

#endif // DFA_WRAPPER_H
