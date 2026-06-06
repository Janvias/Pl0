#include <iostream>
#include <stack>
#include "pl0_regex/pl0_regex.h"
#include "dfa/regex_parser.h"
#include "dfa/nfa.h"
#include "dfa/dfa.h"

int main() {
    try {
        std::cout << "Testing PL/0 Identifier regex: " << PL0Regex::REG_IDENT << std::endl;
        
        std::string withConcat = RegexParser::insertConcat(PL0Regex::REG_IDENT);
        std::string postfix = RegexParser::shuntingYard(withConcat);
        
        NFA* nfa = NFA::thompsonConstruction(postfix);
        DFA* dfa = DFA::nfaToDFA(nfa);
        DFA* minDFA = dfa->minimize();
        
        std::cout << "DFA states: " << minDFA->states.size() << std::endl;
        
        std::string testStrings[] = {"x", "abc", "a123", "_abc", "123", "1a", ""};
        for (const std::string& s : testStrings) {
            bool accepted = minDFA->accepts(s);
            std::cout << "String \"" << s << "\" is " << (accepted ? "accepted" : "rejected") << std::endl;
        }
        
        delete nfa;
        delete dfa;
        delete minDFA;
        
        std::cout << "\nTesting PL/0 Number regex: " << PL0Regex::REG_NUMBER << std::endl;
        
        withConcat = RegexParser::insertConcat(PL0Regex::REG_NUMBER);
        postfix = RegexParser::shuntingYard(withConcat);
        
        nfa = NFA::thompsonConstruction(postfix);
        dfa = DFA::nfaToDFA(nfa);
        minDFA = dfa->minimize();
        
        std::cout << "DFA states: " << minDFA->states.size() << std::endl;
        
        std::string numStrings[] = {"0", "123", "999", "012", ""};
        for (const std::string& s : numStrings) {
            bool accepted = minDFA->accepts(s);
            std::cout << "String \"" << s << "\" is " << (accepted ? "accepted" : "rejected") << std::endl;
        }
        
        delete nfa;
        delete dfa;
        delete minDFA;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
