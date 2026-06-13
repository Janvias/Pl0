/**
 * @file nfa.cpp
 * @brief NFA（非确定有限自动机）实现
 * @details 实现NFA的构建、ε闭包计算、move操作和Thompson构造法
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "nfa.h"
#include <stack>
#include <cctype>

//============================================================================
// NFAState 构造函数
//============================================================================

NFAState::NFAState(int id) : id(id) {}

//============================================================================
// NFA 构造函数 / 析构函数
//============================================================================

NFA::NFA() : start(nullptr), accept(nullptr) {}

NFA::~NFA() {
    for (NFAState* state : states) {
        delete state;
    }
}

//============================================================================
// 添加NFA状态
//============================================================================

NFAState* NFA::addState() {
    NFAState* state = new NFAState(states.size());
    states.insert(state);
    return state;
}

//============================================================================
// 添加状态转换
//============================================================================

void NFA::addTransition(NFAState* from, char symbol, NFAState* to) {
    from->transitions[symbol].insert(to);
    if (symbol != '\0') {
        alphabet.insert(symbol);
    }
}

//============================================================================
// 计算ε闭包
//============================================================================
// 从给定状态集合出发，通过ε转换能到达的所有状态
// 使用BFS算法遍历所有可达状态
//============================================================================

std::set<NFAState*> NFA::epsilonClosure(const std::set<NFAState*>& states) {
    std::set<NFAState*> closure = states;
    std::queue<NFAState*> q;
    
    for (NFAState* state : states) {
        q.push(state);
    }
    
    while (!q.empty())NFAState* current = q.front();
        q.pop();
        
        // 检查是否有ε转换（'\0'表示ε）
        if (current->transitions.count('\0')) {
            for (NFAState* next : current->transitions['\0']) {
                if (closure.find(next) == closure.end()) {
                    closure.insert(next);
                    q.push(next);
                }
            }
        }
    }
    
    return closure;
}

//============================================================================
// move操作
//============================================================================
// 从给定状态集合出发，通过指定字符能到达的所有状态（不含ε转换）
//============================================================================

std::set<NFAState*> NFA::move(const std::set<NFAState*>& states, char symbol) {
    std::set<NFAState*> result;
    
    for (NFAState* state : states) {
        if (state->transitions.count(symbol)) {
            for (NFAState* next : state->transitions[symbol]) {
                result.insert(next);
            }
        }
    }
    
    return result;
}

//============================================================================
// 扩展字符类
//============================================================================
// 将[a-z]这样的字符类展开为具体字符集合
// 支持范围表示（如a-z）和单个字符
//============================================================================

std::set<char> expandCharClass(const std::string& charClass) {
    std::set<char> chars;
    
    if (charClass.size() < 3) {
        return chars;
    }
    
    for (size_t i = 1; i < charClass.size() - 1; i++) {
        if (charClass[i] == '-' && i > 0 && i < charClass.size() - 2) {
            // 处理范围表示，如a-z
            char start = charClass[i - 1];
            char end = charClass[i + 1];
            if (start <= end) {
                for (char c = start; c <= end; c++) {
                    chars.insert(c);
                }
            }
            i += 2;
        } else {
            // 单个字符
            chars.insert(charClass[i]);
        }
    }
    
    return chars;
}

//============================================================================
// Thompson构造法：后缀正则表达式转NFA
//============================================================================
// 支持以下操作：
// - 字符和字符类：创建基本的NFA（起始->接受）
// - 连接(.)：将两个NFA串联
// - 选择(|)：将两个NFA并联（新起始状态通过ε连接到两个NFA的起始）
// - Kleene闭包(*)：创建循环结构（可重复0次或多次）
// - 正闭包(+)：创建循环结构（至少重复1次）
//============================================================================

NFA* NFA::thompsonConstruction(const std::string& postfix) {
    std::stack<NFA*> nfaStack;
    
    size_t i = 0;
    while (i < postfix.size()) {
        // 处理字符类 [a-z]
        if (postfix[i] == '[') {
            size_t j = i;
            while (j < postfix.size() && postfix[j] != ']') {
                j++;
            }
            
            if (j >= postfix.size()) {
                i++;
                continue;
            }
            
            std::string charClass = postfix.substr(i, j - i + 1);
            std::set<char> chars = expandCharClass(charClass);
            
            // 创建字符类NFA：起始状态通过所有字符连接到接受状态
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            for (char c : chars) {
                nfa->addTransition(start, c, accept);
            }
            
            nfa->start = start;
            nfa->accept = accept;
            nfaStack.push(nfa);
            i = j + 1;
        } 
        // 处理普通字符
        else if (isalnum(postfix[i])) {
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            nfa->addTransition(start, postfix[i], accept);
            nfa->start = start;
            nfa->accept = accept;
            nfaStack.push(nfa);
            i++;
        } 
        // 处理连接操作符(.)
        else if (postfix[i] == '.') {
            if (nfaStack.size() < 2) {
                i++;
                continue;
            }
            
            NFA* b = nfaStack.top(); nfaStack.pop();
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            // 串联：a的接受状态通过ε连接到b的起始状态
            a->addTransition(a->accept, '\0', b->start);
            a->states.insert(b->states.begin(), b->states.end());
            a->alphabet.insert(b->alphabet.begin(), b->alphabet.end());
            a->accept = b->accept;
            
            // 关键修复：在删除 b 之前清空它的 states 集合，
            // 这样析构函数就不会删除已经被 a 接管的状态
            b->states.clear();
            delete b;
            nfaStack.push(a);
            i++;
        } 
        // 处理选择操作符(|)
        else if (postfix[i] == '|') {
            if (nfaStack.size() < 2) {
                i++;
                continue;
            }
            
            NFA* b = nfaStack.top(); nfaStack.pop();
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            // 并联：创建新的起始和接受状态
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->states.insert(b->states.begin(), b->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            nfa->alphabet.insert(b->alphabet.begin(), b->alphabet.end());
            
            // 新起始状态通过ε连接到a和b的起始状态
            nfa->addTransition(start, '\0', a->start);
            nfa->addTransition(start, '\0', b->start);
            // a和b的接受状态通过ε连接到新接受状态
            nfa->addTransition(a->accept, '\0', accept);
            nfa->addTransition(b->accept, '\0', accept);
            
            nfa->start = start;
            nfa->accept = accept;
            
            // 关键修复：清空状态集合以避免重复删除
            a->states.clear();
            b->states.clear();
            delete a;
            delete b;
            nfaStack.push(nfa);
            i++;
        } 
        // 处理Kleene闭包(*)
        else if (postfix[i] == '*') {
            if (nfaStack.empty()) {
                i++;
                continue;
            }
            
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            // 创建循环结构：可重复0次或多次
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            
            // 新起始状态通过ε连接到a的起始和新接受状态（允许0次匹配）
            nfa->addTransition(start, '\0', a->start);
            nfa->addTransition(start, '\0', accept);
            // a的接受状态通过ε连接回a的起始状态（允许重复）
            nfa->addTransition(a->accept, '\0', a->start);
            nfa->addTransition(a->accept, '\0', accept);
            
            nfa->start = start;
            nfa->accept = accept;
            
            // 关键修复：清空状态集合以避免重复删除
            a->states.clear();
            delete a;
            nfaStack.push(nfa);
            i++;
        } 
        // 处理正闭包(+)
        else if (postfix[i] == '+') {
            if (nfaStack.empty()) {
                i++;
                continue;
            }
            
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            // 创建循环结构：至少重复1次
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            
            // 新起始状态通过ε连接到a的起始状态（必须至少匹配1次）
            nfa->addTransition(start, '\0', a->start);
            // a的接受状态通过ε连接回a的起始状态（允许重复）
            nfa->addTransition(a->accept, '\0', a->start);
            nfa->addTransition(a->accept, '\0', accept);
            
            nfa->start = start;
            nfa->accept = accept;
            
            // 关键修复：清空状态集合以避免重复删除
            a->states.clear();
            delete a;
            nfaStack.push(nfa);
            i++;
        } 
        // 跳过其他字符
        else {
            i++;
        }
    }
    
    if (nfaStack.empty()) {
        return nullptr;
    }
    
    return nfaStack.top();
}
