/**
 * @file dfa.cpp
 * @brief DFA（确定有限自动机）实现
 * @details 实现DFA的构建、子集构造法（NFA转DFA）和Hopcroft最小化算法
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "dfa.h"
#include <queue>
#include <algorithm>

//============================================================================
// DFAState 构造函数
//============================================================================

DFAState::DFAState(int id, const std::set<NFAState*>& nfaStates, bool isAccept) 
    : id(id), nfaStates(nfaStates), isAccept(isAccept) {}

//============================================================================
// DFA 构造函数 / 析构函数
//============================================================================

DFA::DFA() : startState(nullptr) {}

DFA::~DFA() {
    for (DFAState* state : states) {
        delete state;
    }
}

//============================================================================
// 添加DFA状态
//============================================================================

DFAState* DFA::addState(const std::set<NFAState*>& nfaStates, bool isAccept) {
    DFAState* state = new DFAState(states.size(), nfaStates, isAccept);
    states.push_back(state);
    if (isAccept) {
        acceptStates.insert(state);
    }
    return state;
}

//============================================================================
// 查找已存在的DFA状态
//============================================================================

DFAState* DFA::findState(const std::set<NFAState*>& nfaStates) {
    for (DFAState* state : states) {
        if (state->nfaStates == nfaStates) {
            return state;
        }
    }
    return nullptr;
}

//============================================================================
// Hopcroft算法最小化DFA
//============================================================================
// 算法步骤：
// 1. 初始划分：接受状态和非接受状态两个等价类
// 2. 对每个等价类，检查所有字符的转换是否指向同一等价类
// 3. 若转换不一致，则分裂等价类
// 4. 重复步骤2-3直到不再变化
//============================================================================

DFA* DFA::minimize() {
    if (states.empty()) return new DFA();
    
    // 初始划分：接受状态和非接受状态
    std::vector<std::set<DFAState*>> partitions;
    std::set<DFAState*> accept, nonAccept;
    
    for (DFAState* state : states) {
        if (state->isAccept) {
            accept.insert(state);
        } else {
            nonAccept.insert(state);
        }
    }
    
    if (!nonAccept.empty()) partitions.push_back(nonAccept);
    if (!accept.empty()) partitions.push_back(accept);
    
    // 建立状态到等价类的映射
    std::map<DFAState*, int> stateToPartition;
    for (int i = 0; i < partitions.size(); i++) {
        for (DFAState* state : partitions[i]) {
            stateToPartition[state] = i;
        }
    }
    
    // 迭代分裂等价类
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::set<DFAState*>> newPartitions;
        
        for (const std::set<DFAState*>& partition : partitions) {
            if (partition.size() <= 1) {
                newPartitions.push_back(partition);
                continue;
            }
            
            // 根据转换目标等价类分组
            std::map<std::vector<int>, std::set<DFAState*>> groups;
            
            for (DFAState* state : partition) {
                std::vector<int> signature;
                for (char c : alphabet) {
                    DFAState* next = state->transitions.count(c) ? state->transitions[c] : nullptr;
                    signature.push_back(next ? stateToPartition[next] : -1);
                }
                groups[signature].insert(state);
            }
            
            // 如果分组数大于1，说明需要分裂
            if (groups.size() > 1) {
                changed = true;
                for (const auto& pair : groups) {
                    newPartitions.push_back(pair.second);
                }
            } else {
                newPartitions.push_back(partition);
            }
        }
        
        partitions = newPartitions;
        
        // 更新状态到等价类的映射
        stateToPartition.clear();
        for (int i = 0; i < partitions.size(); i++) {
            for (DFAState* state : partitions[i]) {
                stateToPartition[state] = i;
            }
        }
    }
    
    // 构建最小化DFA
    DFA* minDFA = new DFA();
    minDFA->alphabet = alphabet;
    
    // 为每个等价类创建一个新状态
    std::map<int, DFAState*> partitionToState;
    for (int i = 0; i < partitions.size(); i++) {
        bool isAccept = false;
        std::set<NFAState*> nfaStates;
        for (DFAState* state : partitions[i]) {
            if (state->isAccept) isAccept = true;
            nfaStates.insert(state->nfaStates.begin(), state->nfaStates.end());
        }
        DFAState* newState = minDFA->addState(nfaStates, isAccept);
        partitionToState[i] = newState;
        
        // 设置起始状态
        if (partitions[i].count(startState)) {
            minDFA->startState = newState;
        }
    }
    
    // 添加转换
    for (int i = 0; i < partitions.size(); i++) {
        DFAState* oldState = *partitions[i].begin();
        DFAState* newState = partitionToState[i];
        
        for (char c : alphabet) {
            if (oldState->transitions.count(c)) {
                DFAState* oldNext = oldState->transitions[c];
                int nextPartition = stateToPartition[oldNext];
                newState->transitions[c] = partitionToState[nextPartition];
            }
        }
    }
    
    return minDFA;
}

//============================================================================
// 测试字符串是否被DFA接受
//============================================================================

bool DFA::accepts(const std::string& s) {
    DFAState* current = startState;
    for (char c : s) {
        if (alphabet.count(c) == 0) {
            return false;
        }
        if (current->transitions.count(c) == 0) {
            return false;
        }
        current = current->transitions[c];
    }
    return current->isAccept;
}

//============================================================================
// 子集构造法：NFA转DFA
//============================================================================
// 算法步骤：
// 1. 计算NFA起始状态的ε闭包作为DFA起始状态
// 2. 对每个DFA状态和每个字母表字符，计算move后的ε闭包
// 3. 若产生新状态集合则创建新DFA状态
// 4. 重复直到没有新状态产生
//============================================================================

DFA* DFA::nfaToDFA(NFA* nfa) {
    DFA* dfa = new DFA();
    dfa->alphabet = nfa->alphabet;
    
    // 计算初始ε闭包
    std::set<NFAState*> initialClosure = nfa->epsilonClosure({nfa->start});
    bool isInitialAccept = (initialClosure.count(nfa->accept) > 0);
    DFAState* initialState = dfa->addState(initialClosure, isInitialAccept);
    dfa->startState = initialState;
    
    // BFS处理所有状态
    std::queue<DFAState*> q;
    q.push(initialState);
    
    while (!q.empty()) {
        DFAState* current = q.front();
        q.pop();
        
        // 对每个字母表字符计算转换
        for (char c : nfa->alphabet) {
            std::set<NFAState*> moveResult = nfa->move(current->nfaStates, c);
            std::set<NFAState*> closure = nfa->epsilonClosure(moveResult);
            
            if (closure.empty()) continue;
            
            DFAState* nextState = dfa->findState(closure);
            if (!nextState) {
                bool isAccept = (closure.count(nfa->accept) > 0);
                nextState = dfa->addState(closure, isAccept);
                q.push(nextState);
            }
            
            current->transitions[c] = nextState;
        }
    }
    
    return dfa;
}
