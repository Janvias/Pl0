/**
 * @file dfa_wrapper.cpp
 * @brief DFA封装接口实现
 * @details 实现C语言兼容的DFA创建、测试和销毁接口
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "dfa_wrapper.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

extern "C" {

/**
 * @brief 从正则表达式创建最小化DFA
 * @details 完整的正则表达式处理流程：
 *          1. 扩展{n}量词
 *          2. 插入显式连接符
 *          3. 调度场算法转换为后缀
 *          4. Thompson构造法创建NFA
 *          5. 子集构造法转换为DFA
 *          6. Hopcroft算法最小化DFA
 */
void* create_dfa_from_regex(const char* regex) {
    std::string regexStr(regex);
    // 预处理：在正则表达式流程之前扩展{n}量词
    std::string expanded = RegexParser::expandQuantifiers(regexStr);
    // 插入显式连接符
    std::string regexWithConcat = RegexParser::insertConcat(expanded);
    // 调度场算法转换为后缀
    std::string postfix = RegexParser::shuntingYard(regexWithConcat);

    // Thompson构造法：后缀 -> NFA
    NFA* nfa = NFA::thompsonConstruction(postfix);
    // 子集构造法：NFA -> DFA
    DFA* dfa = DFA::nfaToDFA(nfa);
    // Hopcroft算法：DFA最小化
    DFA* minDFA = dfa->minimize();

    // 清理中间产物
    delete nfa;
    delete dfa;

    return minDFA;
}

/**
 * @brief 测试字符串是否被DFA接受
 */
int dfa_accepts(void* dfa, const char* str) {
    DFA* d = static_cast<DFA*>(dfa);
    return d->accepts(std::string(str)) ? 1 : 0;
}

/**
 * @brief 销毁DFA
 */
void destroy_dfa(void* dfa) {
    DFA* d = static_cast<DFA*>(dfa);
    delete d;
}

}
