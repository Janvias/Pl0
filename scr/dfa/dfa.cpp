#include "dfa.h"
#include <queue>
#include <algorithm>

DFAState::DFAState(int id, const std::set<NFAState*>& nfaStates, bool isAccept) 
    : id(id), nfaStates(nfaStates), isAccept(isAccept) {}

DFA::DFA() : startState(nullptr) {}

DFA::~DFA() {
    for (DFAState* state : states) {
        delete state;
    }
}

DFAState* DFA::addState(const std::set<NFAState*>& nfaStates, bool isAccept) {
    DFAState* state = new DFAState(states.size(), nfaStates, isAccept);
    states.push_back(state);
    if (isAccept) {
        acceptStates.insert(state);
    }
    return state;
}

DFAState* DFA::findState(const std::set<NFAState*>& nfaStates) {
    for (DFAState* state : states) {
        if (state->nfaStates == nfaStates) {
            return state;
        }
    }
    return nullptr;
}

DFA* DFA::minimize() {
    if (states.empty()) return new DFA();
    
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
    
    std::map<DFAState*, int> stateToPartition;
    for (int i = 0; i < partitions.size(); i++) {
        for (DFAState* state : partitions[i]) {
            stateToPartition[state] = i;
        }
    }
    
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::set<DFAState*>> newPartitions;
        
        for (const std::set<DFAState*>& partition : partitions) {
            if (partition.size() <= 1) {
                newPartitions.push_back(partition);
                continue;
            }
            
            std::map<std::vector<int>, std::set<DFAState*>> groups;
            
            for (DFAState* state : partition) {
                std::vector<int> signature;
                for (char c : alphabet) {
                    DFAState* next = state->transitions.count(c) ? state->transitions[c] : nullptr;
                    signature.push_back(next ? stateToPartition[next] : -1);
                }
                groups[signature].insert(state);
            }
            
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
        
        stateToPartition.clear();
        for (int i = 0; i < partitions.size(); i++) {
            for (DFAState* state : partitions[i]) {
                stateToPartition[state] = i;
            }
        }
    }
    
    DFA* minDFA = new DFA();
    minDFA->alphabet = alphabet;
    
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
        
        if (partitions[i].count(startState)) {
            minDFA->startState = newState;
        }
    }
    
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

DFA* DFA::nfaToDFA(NFA* nfa) {
    DFA* dfa = new DFA();
    dfa->alphabet = nfa->alphabet;
    
    std::set<NFAState*> initialClosure = nfa->epsilonClosure({nfa->start});
    bool isInitialAccept = (initialClosure.count(nfa->accept) > 0);
    DFAState* initialState = dfa->addState(initialClosure, isInitialAccept);
    dfa->startState = initialState;
    
    std::queue<DFAState*> q;
    q.push(initialState);
    
    while (!q.empty()) {
        DFAState* current = q.front();
        q.pop();
        
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
