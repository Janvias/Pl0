/**
 * @file nfa.h
 * @brief NFA（非确定有限自动机）定义
 * @details 实现NFA的数据结构和Thompson构造法（正则表达式转NFA）
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef NFA_H
#define NFA_H

#include <set>
#include <map>
#include <queue>
#include <string>

/**
 * @struct NFAState
 * @brief NFA状态结构
 * @details 每个状态可以有多个相同字符的转换（非确定性）
 */
struct NFAState {
    int id;                                          // 状态编号
    std::map<char, std::set<NFAState*>> transitions; // 状态转换表：字符 -> 目标状态集合
    NFAState(int id);
};

/**
 * @class NFA
 * @brief 非确定有限自动机类
 * @details 提供NFA的构建、ε闭包计算和move操作
 */
class NFA {
public:
    NFAState* start;                   // 起始状态
    NFAState* accept;                  // 接受状态
    std::set<NFAState*> states;        // 所有状态集合
    std::set<char> alphabet;           // 字母表（所有可能的输入字符）

    NFA();
    ~NFA();

    /**
     * @brief 添加新状态
     * @return 新创建的NFA状态指针
     */
    NFAState* addState();
    
    /**
     * @brief 添加状态转换
     * @param from 源状态
     * @param symbol 转换字符（'\0'表示ε转换）
     * @param to 目标状态
     */
    void addTransition(NFAState* from, char symbol, NFAState* to);
    
    /**
     * @brief 计算ε闭包
     * @details 从给定状态集合出发，通过ε转换能到达的所有状态
     * @param states 初始状态集合
     * @return ε闭包结果（包含所有可达状态）
     */
    std::set<NFAState*> epsilonClosure(const std::set<NFAState*>& states);
    
    /**
     * @brief move操作
     * @details 从给定状态集合出发，通过指定字符能到达的所有状态
     * @param states 初始状态集合
     * @param symbol 转换字符
     * @return move操作结果
     */
    std::set<NFAState*> move(const std::set<NFAState*>& states, char symbol);
    
    /**
     * @brief Thompson构造法：后缀正则表达式转NFA
     * @details 支持字符、连接(.)、选择(|)、闭包(*)、正闭包(+)
     * @param postfix 后缀形式的正则表达式
     * @return 构建出的NFA指针
     */
    static NFA* thompsonConstruction(const std::string& postfix);
};

/**
 * @brief 扩展字符类
 * @details 将[a-z]这样的字符类展开为具体字符集合
 * @param charClass 字符类字符串（如"[a-z]"）
 * @return 展开后的字符集合
 */
std::set<char> expandCharClass(const std::string& charClass);

#endif // NFA_H
