/**
 * @file dfa.h
 * @brief DFA（确定有限自动机）定义
 * @details 实现DFA的数据结构、NFA到DFA的转换（子集构造法）和DFA最小化（Hopcroft算法）
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef DFA_H
#define DFA_H

#include "nfa.h"
#include <vector>
#include <set>
#include <map>
#include <string>

/**
 * @struct DFAState
 * @brief DFA状态结构
 * @details 每个DFA状态对应一组NFA状态的集合（子集构造法的结果）
 */
struct DFAState {
    int id;                                 // 状态编号
    std::set<NFAState*> nfaStates;          // 对应的NFA状态集合
    bool isAccept;                          // 是否为接受状态
    std::map<char, DFAState*> transitions;  // 状态转换表：字符 -> 目标状态
    
    DFAState(int id, const std::set<NFAState*>& nfaStates, bool isAccept);
};

/**
 * @class DFA
 * @brief 确定有限自动机类
 * @details 提供DFA的构建、最小化和字符串匹配功能
 */
class DFA {
public:
    std::vector<DFAState*> states;          // 所有状态列表
    DFAState* startState;                   // 起始状态
    std::set<DFAState*> acceptStates;       // 接受状态集合
    std::set<char> alphabet;                // 字母表（所有可能的输入字符）

    DFA();
    ~DFA();

    /**
     * @brief 添加新状态
     * @param nfaStates 该DFA状态对应的NFA状态集合
     * @param isAccept 是否为接受状态
     * @return 新创建的DFA状态指针
     */
    DFAState* addState(const std::set<NFAState*>& nfaStates, bool isAccept);
    
    /**
     * @brief 查找已存在的状态
     * @param nfaStates NFA状态集合
     * @return 找到的DFA状态指针，若不存在则返回nullptr
     */
    DFAState* findState(const std::set<NFAState*>& nfaStates);
    
    /**
     * @brief Hopcroft算法最小化DFA
     * @details 将DFA状态划分为等价类，合并等价状态
     * @return 最小化后的新DFA指针
     */
    DFA* minimize();
    
    /**
     * @brief 测试字符串是否被接受
     * @param s 待测试的字符串
     * @return 若字符串被DFA接受则返回true
     */
    bool accepts(const std::string& s);
    
    /**
     * @brief 子集构造法：NFA转DFA
     * @details 使用ε闭包和move操作将NFA转换为等价的DFA
     * @param nfa 源NFA指针
     * @return 构建出的DFA指针
     */
    static DFA* nfaToDFA(NFA* nfa);
};

#endif // DFA_H