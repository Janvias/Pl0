/**
 * @file syntax_analyzer.hpp
 * @brief PL/0 Syntax Analyzer (LL(1) + SLR(1))
 */

#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <sstream>
#include <algorithm>

using namespace std;

// ==================== Data Structures ====================

struct Production {
    string left;
    vector<string> right;
    int index;
    
    Production() : index(-1) {}
    Production(const string& l, const vector<string>& r, int i) : left(l), right(r), index(i) {}
    
    string toString() const {
        string s = left + " -> ";
        for (size_t i = 0; i < right.size(); ++i) s += (i ? " " : "") + right[i];
        return s;
    }
    
    bool operator<(const Production& o) const { return index < o.index; }
    bool operator==(const Production& o) const { return index == o.index; }
};

struct LRItem {
    Production prod;
    size_t dotPos;
    
    LRItem(const Production& p, size_t d) : prod(p), dotPos(d) {}
    
    string toString() const {
        stringstream ss;
        ss << prod.left << " -> ";
        for (size_t i = 0; i < prod.right.size(); ++i) {
            if (i == dotPos) ss << "·";
            ss << prod.right[i] << " ";
        }
        if (dotPos == prod.right.size()) ss << "·";
        return ss.str();
    }
    
    bool operator==(const LRItem& o) const { return prod.index == o.prod.index && dotPos == o.dotPos; }
    bool operator<(const LRItem& o) const { return prod.index < o.prod.index || (prod.index == o.prod.index && dotPos < o.dotPos); }
};

// ==================== Grammar Class ====================

class Grammar {
private:
    vector<Production> prods;
    set<string> terms, nonTerms;
    string startSym;
    map<string, set<string>> first, follow;
    map<Production, set<string>> select;
    map<string, map<string, Production>> llTable;
    
    void addProd(const string& l, const vector<string>& r) {
        prods.emplace_back(l, r, prods.size() + 1);
        nonTerms.insert(l);
        for (const auto& s : r) if (s != "ε" && !nonTerms.count(s)) terms.insert(s);
    }
    
    // FIRST(X1X2...Xn)
    set<string> firstOfString(const vector<string>& syms) {
        set<string> result;
        if (syms.empty() || syms[0] == "ε") return result.insert("ε"), result;
        
        for (const auto& s : syms) {
            for (const auto& x : first[s]) if (x != "ε") result.insert(x);
            if (!first[s].count("ε")) break;
        }
        
        bool allEps = all_of(syms.begin(), syms.end(), [this](const string& s) { return s == "ε" || first[s].count("ε"); });
        if (allEps) result.insert("ε");
        return result;
    }
    
public:
    Grammar() { initPL0(); computeFirst(); computeFollow(); computeSelect(); buildLL1Table(); }
    
    void initPL0() {
        prods.clear(); terms.clear(); nonTerms.clear();
        startSym = "Program";
        
        // Program Structure
        addProd("Program", {"SubProgram", "."});
        addProd("SubProgram", {"ConstDecl", "VarDecl", "ProcDecl", "Stmt"});
        
        // Constant Declaration
        addProd("ConstDecl", {"const", "ConstDef", "ConstDefPrime", ";", "ConstDeclPrime"});
        addProd("ConstDecl", {"ε"});
        addProd("ConstDeclPrime", {"const", "ConstDef", "ConstDefPrime", ";", "ConstDeclPrime"});
        addProd("ConstDeclPrime", {"ε"});
        addProd("ConstDef", {"Ident", "=", "Number"});
        addProd("ConstDefPrime", {",", "ConstDef", "ConstDefPrime"});
        addProd("ConstDefPrime", {"ε"});
         
        // Variable Declaration
        addProd("VarDecl", {"var", "Ident", "VarListPrime", ";", "VarDeclPrime"});
        addProd("VarDecl", {"ε"});
        addProd("VarDeclPrime", {"var", "Ident", "VarListPrime", ";", "VarDeclPrime"});
        addProd("VarDeclPrime", {"ε"});
        addProd("VarListPrime", {",", "Ident", "VarListPrime"});
        addProd("VarListPrime", {"ε"});
        
        // Procedure Declaration
        addProd("ProcDecl", {"procedure", "Ident", ";", "SubProgram", ";", "ProcDeclPrime"});
        addProd("ProcDecl", {"ε"});
        addProd("ProcDeclPrime", {"procedure", "Ident", ";", "SubProgram", ";", "ProcDeclPrime"});
        addProd("ProcDeclPrime", {"ε"});
        
        // Statements
        addProd("Stmt", {"Ident", ":=", "Expr"});
        addProd("Stmt", {"if", "Cond", "then", "Stmt"});
        addProd("Stmt", {"while", "Cond", "do", "Stmt"});
        addProd("Stmt", {"call", "Ident"});
        addProd("Stmt", {"read", "Ident"});
        addProd("Stmt", {"write", "Expr"});
        addProd("Stmt", {"begin", "Stmt", "StmtPrime", "end"});
        addProd("Stmt", {"ε"});
        addProd("StmtPrime", {";", "Stmt", "StmtPrime"});
        addProd("StmtPrime", {"ε"});
        
        // Conditions
        addProd("Cond", {"odd", "Expr"});
        addProd("Cond", {"Expr", "RelOp", "Expr"});
        
        // Expressions
        addProd("Expr", {"+", "Term", "ExprPrime"});
        addProd("Expr", {"-", "Term", "ExprPrime"});
        addProd("Expr", {"Term", "ExprPrime"});
        addProd("ExprPrime", {"+", "Term", "ExprPrime"});
        addProd("ExprPrime", {"-", "Term", "ExprPrime"});
        addProd("ExprPrime", {"ε"});
        
        // Terms
        addProd("Term", {"Factor", "TermPrime"});
        addProd("TermPrime", {"*", "Factor", "TermPrime"});
        addProd("TermPrime", {"/", "Factor", "TermPrime"});
        addProd("TermPrime", {"ε"});
        
        // Factors
        addProd("Factor", {"Ident"});
        addProd("Factor", {"Number"});
        addProd("Factor", {"(", "Expr", ")"});
        
        // Relational Operators
        for (const auto& op : {"=", "#", "<", "<=", ">", ">="}) addProd("RelOp", {op});
        
        // Terminals
        terms = {"const", "var", "procedure", "begin", "end", "if", "then", 
                 "while", "do", "call", "read", "write", "odd", 
                 "+", "-", "*", "/", "=", "#", "<", "<=", ">", ">=", 
                 ":=", ";", ",", ".", "(", ")", "Ident", "Number", "ε", "#"};
    }
    
    // Compute FIRST sets
    void computeFirst() {
        first.clear();
        for (const auto& t : terms) if (t != "ε") first[t].insert(t);
        
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& p : prods) {
                for (const auto& s : firstOfString(p.right))
                    if (first[p.left].insert(s).second) changed = true;
            }
        }
    }
    
    // Compute FOLLOW sets
    void computeFollow() {
        follow.clear();
        follow[startSym].insert("#");
        
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& p : prods) {
                for (size_t i = 0; i < p.right.size(); ++i) {
                    const string& B = p.right[i];
                    if (!nonTerms.count(B)) continue;
                    
                    vector<string> beta(p.right.begin() + i + 1, p.right.end());
                    auto f = firstOfString(beta);
                    
                    for (const auto& s : f) if (s != "ε")
                        if (follow[B].insert(s).second) changed = true;
                    
                    if (f.count("ε") || beta.empty())
                        for (const auto& s : follow[p.left])
                            if (follow[B].insert(s).second) changed = true;
                }
            }
        }
    }
    
    // Compute SELECT sets
    void computeSelect() {
        select.clear();
        for (const auto& p : prods) {
            auto f = firstOfString(p.right);
            for (const auto& s : f) if (s != "ε") select[p].insert(s);
            if (f.count("ε")) for (const auto& s : follow[p.left]) select[p].insert(s);
        }
    }
    
    // Build LL(1) parsing table
    bool buildLL1Table() {
        llTable.clear();
        bool ok = true;
        for (const auto& p : prods) {
            for (const auto& t : select[p]) {
                string term = (t == "#") ? "END" : t;
                if (llTable[p.left].count(term)) { cout << "Conflict: " << p.left << "," << term << endl; ok = false; }
                llTable[p.left][term] = p;
            }
        }
        return ok;
    }
    
    // Accessors
    bool isTerm(const string& s) const { return terms.count(s) || s == "#"; }
    bool isNonTerm(const string& s) const { return nonTerms.count(s); }
    const vector<Production>& getProds() const { return prods; }
    const string& getStart() const { return startSym; }
    const map<string, set<string>>& getFirst() const { return first; }
    const map<string, set<string>>& getFollow() const { return follow; }
    const map<string, map<string, Production>>& getLLTable() const { return llTable; }
};

// ==================== LL(1) Parser ====================

class LL1Parser {
private:
    Grammar g;
    vector<string> log;
    
    string getStackStr(stack<string> stk) const {
        string s = "[";
        vector<string> v;
        while (!stk.empty()) { v.push_back(stk.top()); stk.pop(); }
        for (auto it = v.rbegin(); it != v.rend(); ++it) s += (s.length() > 1 ? "," : "") + *it;
        return s + "]";
    }
    
public:
    LL1Parser() { g.computeFirst(); g.computeFollow(); g.computeSelect(); g.buildLL1Table(); }
    
    bool parse(const vector<pair<string, string>>& tokens) {
        log.clear();
        stack<string> stk;
        stk.push("#"); stk.push(g.getStart());
        log.push_back("===== LL(1) Analysis Start =====");
        
        int idx = 0;
        while (!stk.empty()) {
            string top = stk.top();
            string cur = (idx >= tokens.size()) ? "#" : tokens[idx].first;
            
            log.push_back("Stack: " + getStackStr(stk) + " | Input: " + cur);
            
            if (top == "#" && cur == "#") return log.push_back("Accept! Analysis Success"), true;
            
            if (g.isTerm(top)) {
                if (top == cur) { log.push_back("Match: " + top); stk.pop(); ++idx; }
                else return log.push_back("Error: Expect " + top + ", got " + cur), false;
            } else {
                string lookup = (cur == "#") ? "END" : cur;
                auto row = g.getLLTable().find(top);
                if (row == g.getLLTable().end() || !row->second.count(lookup))
                    return log.push_back("Error: LLTable[" + top + "," + lookup + "] is empty"), false;
                
                Production p = row->second.at(lookup);
                log.push_back("Expand: " + p.toString());
                stk.pop();
                for (auto it = p.right.rbegin(); it != p.right.rend(); ++it)
                    if (*it != "ε") stk.push(*it);
            }
        }
        return false;
    }
    
    const vector<string>& getLog() const { return log; }
    const Grammar& getGrammar() const { return g; }
};

// ==================== SLR(1) Parser ====================

class SLRParser {
private:
    Grammar g;
    vector<set<LRItem>> states;
    map<pair<int, string>, int> actionShift, gotoTable, actionReduce;
    set<pair<int, string>> accept;
    vector<string> log;
    
    // Find state index in states list
    int findState(const set<LRItem>& items) const {
        for (size_t i = 0; i < states.size(); ++i)
            if (states[i] == items) return (int)i;
        return -1;
    }
    
    // Closure of LR items
    set<LRItem> closure(set<LRItem> items) {
        for (size_t i = 0; i < items.size(); ++i) {
            auto it = next(items.begin(), i);
            if (it->dotPos < it->prod.right.size()) {
                const string& B = it->prod.right[it->dotPos];
                if (g.isNonTerm(B))
                    for (const auto& p : g.getProds())
                        if (p.left == B) items.insert(LRItem(p, 0));
            }
        }
        return items;
    }
    
    // GO(I, X)
    set<LRItem> go(const set<LRItem>& items, const string& X) {
        set<LRItem> result;
        for (const auto& item : items)
            if (item.dotPos < item.prod.right.size() && item.prod.right[item.dotPos] == X)
                result.insert(LRItem(item.prod, item.dotPos + 1));
        return closure(result);
    }
    
    // Build LR items (canonical collection)
    void buildItems() {
        states.clear();
        states.push_back(closure({LRItem(Production("S'", {g.getStart()}, 0), 0)}));
        
        for (size_t i = 0; i < states.size(); ++i) {
            set<string> symbols;
            for (const auto& item : states[i])
                if (item.dotPos < item.prod.right.size())
                    symbols.insert(item.prod.right[item.dotPos]);
            
            for (const auto& X : symbols) {
                set<LRItem> newItems = go(states[i], X);
                if (findState(newItems) == -1) states.push_back(newItems);
            }
        }
    }
    
    // Build SLR parsing table
    void buildTable() {
        actionShift.clear(); actionReduce.clear(); gotoTable.clear(); accept.clear();
        
        for (size_t i = 0; i < states.size(); ++i) {
            for (const auto& item : states[i]) {
                bool isReduce = (item.dotPos == item.prod.right.size()) ||
                               (item.prod.right.size() == 1 && item.prod.right[0] == "ε");
                
                if (isReduce) {
                    if (item.prod.left == "S'") accept.insert({(int)i, "#"});
                    else {
                        auto followIt = g.getFollow().find(item.prod.left);
                        if (followIt != g.getFollow().end())
                            for (const auto& a : followIt->second)
                                actionReduce[{(int)i, a == "#" ? "END" : a}] = item.prod.index;
                    }
                } else {
                    const string& X = item.prod.right[item.dotPos];
                    if (g.isTerm(X) && X != "ε") {
                        int j = findState(go(states[i], X));
                        if (j >= 0) actionShift[{(int)i, X}] = j;
                    } else if (g.isNonTerm(X)) {
                        int j = findState(go(states[i], X));
                        if (j >= 0) gotoTable[{(int)i, X}] = j;
                    }
                }
            }
        }
    }
    
public:
    SLRParser() { g.computeFirst(); g.computeFollow(); buildItems(); buildTable(); }
    
    bool parse(const vector<pair<string, string>>& tokens) {
        log.clear();
        vector<int> stateStk{0};
        vector<string> symStk{"#"};
        log.push_back("===== SLR(1) Analysis Start =====");
        
        int idx = 0;
        while (true) {
            int s = stateStk.back();
            string cur = (idx >= tokens.size()) ? "#" : tokens[idx].first;
            string lookup = (cur == "#") ? "END" : cur;
            
            log.push_back("StateStack: [" + to_string(stateStk[0]));
            for (size_t i = 1; i < stateStk.size(); ++i) log.back() += "," + to_string(stateStk[i]);
            log.back() += "] | SymStack: [" + symStk[0];
            for (size_t i = 1; i < symStk.size(); ++i) log.back() += "," + symStk[i];
            log.back() += "] | Input: " + cur;
            
            if (accept.count({s, "#"}) && cur == "#") return log.push_back("Accept! Analysis Success"), true;
            
            if (actionShift.count({s, lookup})) {
                int nextState = actionShift[{s, lookup}];
                log.push_back("Shift: " + cur + ", State" + to_string(nextState));
                stateStk.push_back(nextState);
                symStk.push_back(cur);
                ++idx;
                continue;
            }
            
            if (actionReduce.count({s, lookup})) {
                int prodIdx = actionReduce[{s, lookup}];
                const Production& p = g.getProds()[prodIdx - 1];
                log.push_back("Reduce: " + p.toString());
                
                bool isEps = (p.right.size() == 1 && p.right[0] == "ε");
                if (!isEps) for (size_t i = 0; i < p.right.size(); ++i) { stateStk.pop_back(); symStk.pop_back(); }
                
                symStk.push_back(p.left);
                if (!gotoTable.count({stateStk.back(), p.left}))
                    return log.push_back("Error: GOTO[" + to_string(stateStk.back()) + "," + p.left + "] not found"), false;
                
                stateStk.push_back(gotoTable[{stateStk.back(), p.left}]);
                log.push_back("GOTO(" + to_string(stateStk[stateStk.size()-2]) + "," + p.left + ")=" + to_string(stateStk.back()));
                continue;
            }
            
            return log.push_back("Error: ACTION[" + to_string(s) + "," + lookup + "] is empty"), false;
        }
    }
    
    const vector<string>& getLog() const { return log; }
    const vector<set<LRItem>>& getStates() const { return states; }
};

// ==================== Visualizer ====================

class SyntaxVisualizer {
public:
    static void printGrammar(const Grammar& g) {
        cout << "\n===== PL/0 Grammar =====\n";
        for (const auto& p : g.getProds()) cout << "(" << p.index << ") " << p.toString() << "\n";
        
        cout << "\n===== FIRST Sets =====\n";
        for (const auto& kv : g.getFirst())
            if (g.isNonTerm(kv.first)) {
                cout << "FIRST(" << kv.first << ") = {";
                for (const auto& s : kv.second) cout << s << ",";
                cout << "}\n";
            }
        
        cout << "\n===== FOLLOW Sets =====\n";
        for (const auto& kv : g.getFollow()) {
            cout << "FOLLOW(" << kv.first << ") = {";
            for (const auto& s : kv.second) cout << s << ",";
            cout << "}\n";
        }
    }
    
    static void printStates(const SLRParser& slr) {
        cout << "\n===== LR(0) Item Sets =====\n";
        for (size_t i = 0; i < slr.getStates().size(); ++i) {
            cout << "\nState " << i << ":\n";
            for (const auto& item : slr.getStates()[i]) cout << "  " << item.toString() << "\n";
        }
    }
};

#endif
