#include "dfa_wrapper.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

extern "C" {

void* create_dfa_from_regex(const char* regex) {
    std::string regexStr(regex);
    std::string regexWithConcat = RegexParser::insertConcat(regexStr);
    std::string postfix = RegexParser::shuntingYard(regexWithConcat);
    
    NFA* nfa = NFA::thompsonConstruction(postfix);
    DFA* dfa = DFA::nfaToDFA(nfa);
    DFA* minDFA = dfa->minimize();
    
    delete nfa;
    delete dfa;
    
    return minDFA;
}

int dfa_accepts(void* dfa, const char* str) {
    DFA* d = static_cast<DFA*>(dfa);
    return d->accepts(std::string(str)) ? 1 : 0;
}

void destroy_dfa(void* dfa) {
    DFA* d = static_cast<DFA*>(dfa);
    delete d;
}

}
