#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

typedef int StateID;
const char EPSILON = 0x01;
const char LETTER = 'L';
const char DIGIT = 'D';
StateID stateCount = 0;

enum TokenType {
    KEYWORD,
    IDENT,
    NUMBER,
    DOUBLE_OP,
    SINGLE_OP,
    TOKEN_ERROR
};

struct Token {
    TokenType type;
    string value;
    int line;
    int col;
    Token(TokenType t, string v, int l, int c) : type(t), value(v), line(l), col(c) {}
};

StateID newState() { return stateCount++; }

struct NFAEdge {
    StateID from, to;
    char ch;
    NFAEdge(StateID f, StateID t, char c) : from(f), to(t), ch(c) {}
};

struct NFASegment {
    StateID start;
    StateID end;
    TokenType tokenType;
    NFASegment(StateID s, StateID e, TokenType t = TOKEN_ERROR) : start(s), end(e), tokenType(t) {}
};

class NFA_Builder {
public:
    vector<NFAEdge> edges;
    map<StateID, TokenType> finalStates;

    NFASegment buildSegment(string postfix, TokenType type) {
        stack<NFASegment> st;
        
        for (char c : postfix) {
            if (c == LETTER || c == DIGIT || (isalnum(c) && c != '.' && c != '|' && c != '*' && c != '+')) {
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, e, c);
                st.push(NFASegment(s, e));
            } else if (c == '.') {
                if (st.size() < 2) continue;
                NFASegment b = st.top(); st.pop();
                NFASegment a = st.top(); st.pop();
                edges.emplace_back(a.end, b.start, EPSILON);
                st.push(NFASegment(a.start, b.end));
            } else if (c == '|') {
                if (st.size() < 2) continue;
                NFASegment b = st.top(); st.pop();
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(s, b.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(b.end, e, EPSILON);
                st.push(NFASegment(s, e));
            } else if (c == '*') {
                if (st.empty()) continue;
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(a.end, a.start, EPSILON);
                edges.emplace_back(s, e, EPSILON);
                st.push(NFASegment(s, e));
            } else if (c == '+') {
                if (st.empty()) continue;
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(a.end, a.start, EPSILON);
                st.push(NFASegment(s, e));
            }
        }
        
        if (st.empty()) return NFASegment(0, 0);
        NFASegment res = st.top();
        finalStates[res.end] = type;
        return res;
    }

    NFASegment mergeAllSegments(const vector<NFASegment>& segments) {
        StateID start = newState();
        StateID end = newState();
        for (auto& seg : segments) {
            edges.emplace_back(start, seg.start, EPSILON);
            edges.emplace_back(seg.end, end, EPSILON);
        }
        return NFASegment(start, end);
    }

    set<StateID> epsilonClosure(set<StateID> states) {
        if (states.empty()) return states;
        queue<StateID> q;
        for (StateID s : states) q.push(s);
        set<StateID> closure = states;
        
        while (!q.empty()) {
            StateID cur = q.front();
            q.pop();
            for (auto& e : edges) {
                if (e.from == cur && e.ch == EPSILON && !closure.count(e.to)) {
                    closure.insert(e.to);
                    q.push(e.to);
                }
            }
        }
        return closure;
    }
};

struct DFAState {
    set<StateID> nfaStates;
    bool isFinal = false;
    TokenType tokenType = TOKEN_ERROR;
    map<char, int> trans;
};

class DFA_Subset {
public:
    vector<DFAState> dfaStates;
    map<set<StateID>, int> idMap;

    void buildDFA(NFA_Builder& nfa) {
        dfaStates.clear();
        idMap.clear();
        
        NFASegment merged = nfa.mergeAllSegments(vector<NFASegment>());
        set<StateID> startSet;
        startSet.insert(merged.start);
        set<StateID> startClosure = nfa.epsilonClosure(startSet);
        
        idMap[startClosure] = 0;
        dfaStates.emplace_back();
        dfaStates[0].nfaStates = startClosure;
        
        for (StateID s : startClosure) {
            if (nfa.finalStates.count(s)) {
                dfaStates[0].isFinal = true;
                dfaStates[0].tokenType = nfa.finalStates[s];
                break;
            }
        }
        
        queue<int> q;
        q.push(0);
        
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            
            set<char> chars;
            for (StateID s : dfaStates[u].nfaStates) {
                for (auto& e : nfa.edges) {
                    if (e.from == s && e.ch != EPSILON) {
                        chars.insert(e.ch);
                    }
                }
            }
            
            for (char c : chars) {
                set<StateID> to;
                for (StateID s : dfaStates[u].nfaStates) {
                    for (auto& e : nfa.edges) {
                        if (e.from == s && e.ch == c) {
                            to.insert(e.to);
                        }
                    }
                }
                set<StateID> closure = nfa.epsilonClosure(to);
                
                if (!idMap.count(closure)) {
                    idMap[closure] = dfaStates.size();
                    dfaStates.emplace_back();
                    dfaStates.back().nfaStates = closure;
                    
                    for (StateID s : closure) {
                        if (nfa.finalStates.count(s)) {
                            dfaStates.back().isFinal = true;
                            dfaStates.back().tokenType = nfa.finalStates[s];
                            break;
                        }
                    }
                    q.push(idMap[closure]);
                }
                dfaStates[u].trans[c] = idMap[closure];
            }
        }
    }
};

class DFA_Minimize {
public:
    vector<DFAState> minDfa;
    map<int, int> groupId;

    void minimize(DFA_Subset& dfa) {
        int n = dfa.dfaStates.size();
        if (n == 0) return;

        map<TokenType, set<int>> initialGroups;
        for (int i = 0; i < n; i++) {
            initialGroups[dfa.dfaStates[i].tokenType].insert(i);
        }

        vector<set<int>> part;
        map<set<int>, int> pid;
        queue<set<int>> q;

        for (auto& p : initialGroups) {
            part.push_back(p.second);
            pid[p.second] = part.size() - 1;
            q.push(p.second);
        }

        while (!q.empty()) {
            auto cur = q.front();
            q.pop();

            set<char> chs;
            for (int s : cur)
                for (auto& t : dfa.dfaStates[s].trans)
                    chs.insert(t.first);

            for (char c : chs) {
                map<int, set<int>> sp;
                for (int s : cur) {
                    int nx = -1;
                    if (dfa.dfaStates[s].trans.count(c))
                        nx = dfa.dfaStates[s].trans[c];

                    int g = -1;
                    for (auto& p : pid)
                        if (p.first.count(nx)) {
                            g = p.second;
                            break;
                        }
                    sp[g].insert(s);
                }

                if (sp.size() > 1) {
                    pid.erase(cur);
                    auto it = find(part.begin(), part.end(), cur);
                    if (it != part.end()) part.erase(it);

                    for (auto& p : sp) {
                        part.push_back(p.second);
                        pid[p.second] = part.size() - 1;
                        q.push(p.second);
                    }
                    break;
                }
            }
        }

        minDfa.clear();
        groupId.clear();
        int idx = 0;
        for (auto& p : part) {
            DFAState st;
            int rep = *p.begin();
            st.isFinal = dfa.dfaStates[rep].isFinal;
            st.tokenType = dfa.dfaStates[rep].tokenType;
            
            for (int x : p) groupId[x] = idx;
            minDfa.push_back(st);
            idx++;
        }

        for (int i = 0; i < (int)part.size(); i++) {
            int rep = *part[i].begin();
            for (auto& t : dfa.dfaStates[rep].trans) {
                minDfa[i].trans[t.first] = groupId[t.second];
            }
        }
    }
};

class Lexer {
private:
    DFA_Minimize& dfa;
    string src;
    vector<Token> tokens;
    vector<string> errors;
    int pos = 0;
    int line = 1;
    int col = 1;

    void skipWhitespace() {
        while (pos < (int)src.size() && isspace(src[pos])) {
            if (src[pos] == '\n') { line++; col = 1; }
            else { col++; }
            pos++;
        }
    }

    void skipComment() {
        if (pos + 1 < (int)src.size() && src[pos] == '/' && src[pos+1] == '/') {
            while (pos < (int)src.size() && src[pos] != '\n') { pos++; col++; }
        } else if (pos + 1 < (int)src.size() && src[pos] == '/' && src[pos+1] == '*') {
            pos += 2; col += 2;
            while (pos + 1 < (int)src.size()) {
                if (src[pos] == '*' && src[pos+1] == '/') { pos += 2; col += 2; return; }
                if (src[pos] == '\n') { line++; col = 1; }
                else { col++; }
                pos++;
            }
            errors.push_back("Error: Unclosed comment");
        }
    }

public:
    Lexer(DFA_Minimize& d, string s) : dfa(d), src(s) {}

    vector<Token> analyse() {
        tokens.clear();
        errors.clear();
        pos = 0;
        line = 1;
        col = 1;

        if (dfa.minDfa.empty()) {
            errors.push_back("Error: Empty DFA");
            return tokens;
        }

        while (pos < (int)src.size()) {
            skipWhitespace();
            if (pos >= (int)src.size()) break;
            skipComment();
            if (pos >= (int)src.size()) break;

            int startPos = pos;
            int startLine = line;
            int startCol = col;
            int currentState = 0;
            int lastFinalState = -1;
            int lastFinalPos = pos;

            while (pos < (int)src.size()) {
                if (currentState < 0 || currentState >= (int)dfa.minDfa.size()) break;

                char c = src[pos];
                char input = c;
                if (isalpha(c)) input = LETTER;
                else if (isdigit(c)) input = DIGIT;

                if (dfa.minDfa[currentState].trans.count(input)) {
                    int nextState = dfa.minDfa[currentState].trans[input];
                    if (nextState < 0 || nextState >= (int)dfa.minDfa.size()) break;
                    currentState = nextState;
                    pos++;
                    if (c == '\n') { line++; col = 1; }
                    else { col++; }

                    if (dfa.minDfa[currentState].isFinal) {
                        lastFinalState = currentState;
                        lastFinalPos = pos;
                    }
                } else {
                    break;
                }
            }

            if (lastFinalState != -1 && lastFinalState < (int)dfa.minDfa.size()) {
                string value = src.substr(startPos, lastFinalPos - startPos);
                TokenType type = dfa.minDfa[lastFinalState].tokenType;
                tokens.emplace_back(type, value, startLine, startCol);
                pos = lastFinalPos;
                line = startLine;
                col = startCol;
                for (int i = startPos; i < lastFinalPos; i++) {
                    if (src[i] == '\n') { line++; col = 1; }
                    else { col++; }
                }
            } else {
                errors.push_back("Error: Invalid character at line " + to_string(line) + ", col " + to_string(col));
                pos++; col++;
            }
        }

        return tokens;
    }

    vector<string> getErrors() { return errors; }
};

int main() {
    cout << "=== Simple DFA Lexer Test ===" << endl;

    vector<pair<string, TokenType>> rules = {
        {"const", KEYWORD},
        {"var", KEYWORD},
        {"begin", KEYWORD},
        {"end", KEYWORD},
        {"if", KEYWORD},
        {"then", KEYWORD},
        {"while", KEYWORD},
        {"do", KEYWORD},
        {":=", DOUBLE_OP},
        {"+", SINGLE_OP},
        {"-", SINGLE_OP},
        {"*", SINGLE_OP},
        {"/", SINGLE_OP},
        {"=", SINGLE_OP},
        {"<", SINGLE_OP},
        {">", SINGLE_OP},
        {";", SINGLE_OP},
        {"(", SINGLE_OP},
        {")", SINGLE_OP},
        {"L(L|D)*", IDENT},
        {"D+", NUMBER}
    };

    NFA_Builder nfaBuilder;
    vector<NFASegment> segments;

    for (auto& rule : rules) {
        segments.push_back(nfaBuilder.buildSegment(rule.first, rule.second));
    }
    nfaBuilder.mergeAllSegments(segments);
    cout << "NFA states: " << stateCount << endl;

    DFA_Subset dfaBuilder;
    dfaBuilder.buildDFA(nfaBuilder);
    cout << "DFA states: " << dfaBuilder.dfaStates.size() << endl;

    DFA_Minimize dfaMin;
    dfaMin.minimize(dfaBuilder);
    cout << "Minimized DFA states: " << dfaMin.minDfa.size() << endl;

    string testCode = "const a=10; var b; begin a := b + 5; end.";
    Lexer lexer(dfaMin, testCode);
    vector<Token> tokens = lexer.analyse();

    cout << "\nTokens found: " << tokens.size() << endl;
    for (auto& t : tokens) {
        string typeStr;
        switch(t.type) {
            case KEYWORD: typeStr = "KEYWORD"; break;
            case IDENT: typeStr = "IDENT"; break;
            case NUMBER: typeStr = "NUMBER"; break;
            case DOUBLE_OP: typeStr = "DOUBLE_OP"; break;
            case SINGLE_OP: typeStr = "SINGLE_OP"; break;
            default: typeStr = "ERROR";
        }
        cout << "(" << typeStr << ", " << t.value << ") ";
    }
    cout << endl;

    return 0;
}