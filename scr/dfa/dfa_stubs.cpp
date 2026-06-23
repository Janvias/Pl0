/**
 * @file dfa_stubs.cpp
 * @brief DFA函数桩实现 — 用于绕过慢速DFA构造
 * @details 提供DFA接口的简单实现，用于调试和性能测试。
 *          这些桩函数跳过实际的正则表达式编译和DFA验证，
 *          始终返回"接受"状态，适用于快速测试词法分析器的其他部分。
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

extern "C" {

/**
 * @brief 创建DFA（桩函数）
 * @details 不执行实际的正则表达式编译，直接返回非空指针表示DFA已创建
 * @param regex 正则表达式字符串（未使用）
 * @return 非空指针（始终返回1）
 */
void* create_dfa_from_regex(const char* regex) {
    // 返回非空指针表示"DFA存在"（验证总是通过）
    (void)regex; // 未使用参数
    return reinterpret_cast<void*>(1);
}

/**
 * @brief 测试字符串是否被DFA接受（桩函数）
 * @details 跳过DFA验证，始终返回接受状态
 * @param dfa DFA指针（未使用）
 * @param str 待测试字符串（未使用）
 * @return 始终返回1（表示接受）
 */
int dfa_accepts(void* dfa, const char* str) {
    // 始终接受（跳过DFA验证以提高速度）
    (void)dfa;  // 未使用参数
    (void)str;  // 未使用参数
    return 1;
}

/**
 * @brief 销毁DFA（桩函数）
 * @details 不执行实际的销毁操作，因为没有分配任何资源
 * @param dfa DFA指针（未使用）
 */
void destroy_dfa(void* dfa) {
    // 无需销毁（没有分配资源）
    (void)dfa;  // 未使用参数
}

}
