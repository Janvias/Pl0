#ifndef NFA_H
#define NFA_H

#include <set>
#include <map>
#include <queue>
#include <string>

struct NFAState {
    int id;
    std::map<char, std::set<NFAState*>> transitions;
    NFAState(int id);
};

class NFA {
public:
    NFAState* start;
    NFAState* accept;
    std::set<NFAState*> states;
    std::set<char> alphabet;

    NFA();
    ~NFA();

    NFAState* addState();
    void addTransition(NFAState* from, char symbol, NFAState* to);
    std::set<NFAState*> epsilonClosure(const std::set<NFAState*>& states);
    std::set<NFAState*> move(const std::set<NFAState*>& states, char symbol);
    static NFA* thompsonConstruction(const std::string& postfix);
};

std::set<char> expandCharClass(const std::string& charClass);

#endif // NFA_H
