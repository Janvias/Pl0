#include "nfa.h"
#include <stack>
#include <cctype>

NFAState::NFAState(int id) : id(id) {}

NFA::NFA() : start(nullptr), accept(nullptr) {}

NFA::~NFA() {
    for (NFAState* state : states) {
        delete state;
    }
}

NFAState* NFA::addState() {
    NFAState* state = new NFAState(states.size());
    states.insert(state);
    return state;
}

void NFA::addTransition(NFAState* from, char symbol, NFAState* to) {
    from->transitions[symbol].insert(to);
    if (symbol != '\0') {
        alphabet.insert(symbol);
    }
}

std::set<NFAState*> NFA::epsilonClosure(const std::set<NFAState*>& states) {
    std::set<NFAState*> closure = states;
    std::queue<NFAState*> q;
    
    for (NFAState* state : states) {
        q.push(state);
    }
    
    while (!q.empty()) {
        NFAState* current = q.front();
        q.pop();
        
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

std::set<char> expandCharClass(const std::string& charClass) {
    std::set<char> chars;
    
    if (charClass.size() < 3) {
        return chars;
    }
    
    for (size_t i = 1; i < charClass.size() - 1; i++) {
        if (charClass[i] == '-' && i > 0 && i < charClass.size() - 2) {
            char start = charClass[i - 1];
            char end = charClass[i + 1];
            if (start <= end) {
                for (char c = start; c <= end; c++) {
                    chars.insert(c);
                }
            }
            i += 2;
        } else {
            chars.insert(charClass[i]);
        }
    }
    
    return chars;
}

NFA* NFA::thompsonConstruction(const std::string& postfix) {
    std::stack<NFA*> nfaStack;
    
    size_t i = 0;
    while (i < postfix.size()) {
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
        } else if (isalnum(postfix[i])) {
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            nfa->addTransition(start, postfix[i], accept);
            nfa->start = start;
            nfa->accept = accept;
            nfaStack.push(nfa);
            i++;
        } else if (postfix[i] == '.') {
            if (nfaStack.size() < 2) {
                i++;
                continue;
            }
            
            NFA* b = nfaStack.top(); nfaStack.pop();
            NFA* a = nfaStack.top(); nfaStack.pop();
            
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
        } else if (postfix[i] == '|') {
            if (nfaStack.size() < 2) {
                i++;
                continue;
            }
            
            NFA* b = nfaStack.top(); nfaStack.pop();
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->states.insert(b->states.begin(), b->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            nfa->alphabet.insert(b->alphabet.begin(), b->alphabet.end());
            
            nfa->addTransition(start, '\0', a->start);
            nfa->addTransition(start, '\0', b->start);
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
        } else if (postfix[i] == '*') {
            if (nfaStack.empty()) {
                i++;
                continue;
            }
            
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            
            nfa->addTransition(start, '\0', a->start);
            nfa->addTransition(start, '\0', accept);
            nfa->addTransition(a->accept, '\0', a->start);
            nfa->addTransition(a->accept, '\0', accept);
            
            nfa->start = start;
            nfa->accept = accept;
            
            // 关键修复：清空状态集合以避免重复删除
            a->states.clear();
            delete a;
            nfaStack.push(nfa);
            i++;
        } else if (postfix[i] == '+') {
            if (nfaStack.empty()) {
                i++;
                continue;
            }
            
            NFA* a = nfaStack.top(); nfaStack.pop();
            
            NFA* nfa = new NFA();
            NFAState* start = nfa->addState();
            NFAState* accept = nfa->addState();
            
            nfa->states.insert(a->states.begin(), a->states.end());
            nfa->alphabet.insert(a->alphabet.begin(), a->alphabet.end());
            
            nfa->addTransition(start, '\0', a->start);
            nfa->addTransition(a->accept, '\0', a->start);
            nfa->addTransition(a->accept, '\0', accept);
            
            nfa->start = start;
            nfa->accept = accept;
            
            // 关键修复：清空状态集合以避免重复删除
            a->states.clear();
            delete a;
            nfaStack.push(nfa);
            i++;
        } else {
            i++;
        }
    }
    
    if (nfaStack.empty()) {
        return nullptr;
    }
    
    return nfaStack.top();
}
