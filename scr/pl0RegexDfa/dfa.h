#ifndef DFA_H
#define DFA_H

#include "nfa.h"
#include <vector>
#include <set>
#include <map>
#include <string>

struct DFAState {
    int id;
    std::set<NFAState*> nfaStates;
    bool isAccept;
    std::map<char, DFAState*> transitions;
    
    DFAState(int id, const std::set<NFAState*>& nfaStates, bool isAccept);
};

class DFA {
public:
    std::vector<DFAState*> states;
    DFAState* startState;
    std::set<DFAState*> acceptStates;
    std::set<char> alphabet;

    DFA();
    ~DFA();

    DFAState* addState(const std::set<NFAState*>& nfaStates, bool isAccept);
    DFAState* findState(const std::set<NFAState*>& nfaStates);
    // Hopcroft算法最小化DFA
    DFA* minimize();
    // 测试字符串是否被接受
    bool accepts(const std::string& s);
    // 子集构造法：NFA转DFA
    static DFA* nfaToDFA(NFA* nfa);
};

#endif // DFA_H