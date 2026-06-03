#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <cctype>
#include <iomanip>
#include <algorithm>

using namespace std;

// ==================== 词法分析部分 ====================
enum TokenType {
    TOK_EOF, TOK_IDENT, TOK_NUMBER,
    // 关键字
    TOK_CONST, TOK_VAR, TOK_PROCEDURE, TOK_BEGIN, TOK_END,
    TOK_IF, TOK_THEN, TOK_WHILE, TOK_DO, TOK_CALL, TOK_ODD,
    // 运算符和分隔符
    TOK_PLUS, TOK_MINUS, TOK_MULT, TOK_DIV,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_LPAREN, TOK_RPAREN, TOK_COMMA, TOK_SEMICOLON,
    TOK_ASSIGN, TOK_PERIOD, TOK_DOLLAR  // $作为输入结束符
};

struct Token {
    TokenType type;
    string value;
    int line;

    Token(TokenType t=TOK_EOF, const string& v="", int l=0) : type(t), value(v), line(l) {}
};

class Lexer {
private:
    ifstream file;
    char currentChar;
    int currentLine;
    map<string, TokenType> keywords;

    void nextChar() {
        file.get(currentChar);
        if (currentChar == '\n') currentLine++;
    }

    void skipWhitespace() {
        while (isspace(currentChar) && !file.eof()) nextChar();
    }

    Token identifierOrKeyword() {
        string id;
        while (isalnum(currentChar) && !file.eof()) {
            id += currentChar;
            nextChar();
        }
        return keywords.count(id) ? Token(keywords[id], id, currentLine) : Token(TOK_IDENT, id, currentLine);
    }

    Token number() {
        string num;
        while (isdigit(currentChar) && !file.eof()) {
            num += currentChar;
            nextChar();
        }
        return Token(TOK_NUMBER, num, currentLine);
    }

public:
    Lexer(const string& filename) : currentLine(1) {
        file.open(filename);
        if (!file.is_open()) { cerr << "无法打开文件: " << filename << endl; exit(1); }
        
        keywords["const"]=TOK_CONST; keywords["var"]=TOK_VAR; keywords["procedure"]=TOK_PROCEDURE;
        keywords["begin"]=TOK_BEGIN; keywords["end"]=TOK_END; keywords["if"]=TOK_IF;
        keywords["then"]=TOK_THEN; keywords["while"]=TOK_WHILE; keywords["do"]=TOK_DO;
        keywords["call"]=TOK_CALL; keywords["odd"]=TOK_ODD;
        
        nextChar();
    }

    ~Lexer() { if (file.is_open()) file.close(); }

    Token nextToken() {
        skipWhitespace();
        if (file.eof()) return Token(TOK_EOF, "", currentLine);
        if (isalpha(currentChar)) return identifierOrKeyword();
        if (isdigit(currentChar)) return number();

        char c = currentChar;
        nextChar();
        switch(c) {
            case '+': return Token(TOK_PLUS, "+", currentLine);
            case '-': return Token(TOK_MINUS, "-", currentLine);
            case '*': return Token(TOK_MULT, "*", currentLine);
            case '/': return Token(TOK_DIV, "/", currentLine);
            case '=': return Token(TOK_EQ, "=", currentLine);
            case '#': return Token(TOK_NEQ, "#", currentLine);
            case '<':
                if (currentChar == '=') { nextChar(); return Token(TOK_LE, "<=", currentLine); }
                return Token(TOK_LT, "<", currentLine);
            case '>':
                if (currentChar == '=') { nextChar(); return Token(TOK_GE, ">=", currentLine); }
                return Token(TOK_GT, ">", currentLine);
            case '(': return Token(TOK_LPAREN, "(", currentLine);
            case ')': return Token(TOK_RPAREN, ")", currentLine);
            case ',': return Token(TOK_COMMA, ",", currentLine);
            case ';': return Token(TOK_SEMICOLON, ";", currentLine);
            case '.': return Token(TOK_PERIOD, ".", currentLine);
            case ':':
                if (currentChar == '=') { nextChar(); return Token(TOK_ASSIGN, ":=", currentLine); }
                cerr << "第" << currentLine << "行: 未知字符: :" << endl; exit(1);
            default:
                cerr << "第" << currentLine << "行: 未知字符: " << c << endl; exit(1);
        }
    }
};

// ==================== SLR(1)语法分析部分 ====================
// 产生式结构体
struct Production {
    int left;           // 左部非终结符编号
    vector<int> right;  // 右部符号编号
    string desc;        // 产生式描述（用于输出）

    Production(int l=0, const vector<int>& r={}, const string& d="") 
        : left(l), right(r), desc(d) {}
};

// LR项目结构体
struct LRItem {
    int prodId;         // 产生式编号
    int dotPos;         // 点的位置

    LRItem(int p=0, int d=0) : prodId(p), dotPos(d) {}

    bool operator==(const LRItem& other) const {
        return prodId == other.prodId && dotPos == other.dotPos;
    }

    bool operator<(const LRItem& other) const {
        return prodId < other.prodId || (prodId == other.prodId && dotPos < other.dotPos);
    }
};

// SLR(1)分析器类
class SLRParser {
private:
    Lexer& lexer;
    vector<Token> tokens;  // 词法分析得到的记号流
    int tokenPtr;          // 当前记号指针
    bool hasError;

    // 文法定义
    vector<Production> productions;  // 产生式列表
    map<string, int> symbolId;       // 符号名->编号
    map<int, string> symbolName;     // 编号->符号名
    int terminalCount;               // 终结符数量
    int nonTerminalCount;            // 非终结符数量

    // SLR(1)核心数据结构
    vector<set<LRItem>> itemSets;    // 项目集规范族
    map<pair<int, int>, int> goTable; // 转移函数: (状态, 符号) -> 下一状态
    map<pair<int, int>, pair<char, int>> actionTable; // ACTION表: (状态, 终结符) -> (动作, 参数)
    map<pair<int, int>, int> gotoTable; // GOTO表: (状态, 非终结符) -> 下一状态
    map<int, set<int>> followSet;    // FOLLOW集

    // 初始化PL/0文法
    void initGrammar() {
        // 定义终结符（编号0~terminalCount-1）
        vector<string> terminals = {
            "ident", "number", "const", "var", "procedure", "begin", "end",
            "if", "then", "while", "do", "call", "odd", "+", "-", "*", "/",
            "=", "#", "<", ">", "<=", ">=", "(", ")", ",", ";", ":=", ".", "$"
        };
        terminalCount = terminals.size();
        for (int i=0; i<terminals.size(); i++) {
            symbolId[terminals[i]] = i;
            symbolName[i] = terminals[i];
        }

        // 定义非终结符（编号terminalCount~...）
        vector<string> nonTerminals = {
            "program", "block", "const_decl", "const_def_list", "const_def",
            "var_decl", "var_def_list", "proc_decl", "statement", "statement_list",
            "condition", "expression", "term", "factor", "relop"
        };
        nonTerminalCount = nonTerminals.size();
        for (int i=0; i<nonTerminals.size(); i++) {
            int id = terminalCount + i;
            symbolId[nonTerminals[i]] = id;
            symbolName[id] = nonTerminals[i];
        }

        // 定义产生式
        productions.emplace_back(symbolId["program"], 
            vector<int>{symbolId["block"], symbolId["."]}, 
            "program -> block .");
        productions.emplace_back(symbolId["block"], 
            vector<int>{symbolId["const_decl"], symbolId["var_decl"], symbolId["proc_decl"], symbolId["statement"]},
            "block -> const_decl var_decl proc_decl statement");
        productions.emplace_back(symbolId["const_decl"], 
            vector<int>{symbolId["const"], symbolId["const_def_list"], symbolId[";"]},
            "const_decl -> const const_def_list ;");
        productions.emplace_back(symbolId["const_def_list"], 
            vector<int>{symbolId["const_def"]},
            "const_def_list -> const_def");
        productions.emplace_back(symbolId["const_def_list"], 
            vector<int>{symbolId["const_def_list"], symbolId[","], symbolId["const_def"]},
            "const_def_list -> const_def_list , const_def");
        productions.emplace_back(symbolId["const_def"], 
            vector<int>{symbolId["ident"], symbolId["="], symbolId["number"]},
            "const_def -> ident = number");
        productions.emplace_back(symbolId["var_decl"], 
            vector<int>{symbolId["var"], symbolId["var_def_list"], symbolId[";"]},
            "var_decl -> var var_def_list ;");
        productions.emplace_back(symbolId["var_def_list"], 
            vector<int>{symbolId["ident"]},
            "var_def_list -> ident");
        productions.emplace_back(symbolId["var_def_list"], 
            vector<int>{symbolId["var_def_list"], symbolId[","], symbolId["ident"]},
            "var_def_list -> var_def_list , ident");
        productions.emplace_back(symbolId["proc_decl"], 
            vector<int>{symbolId["proc_decl"], symbolId["procedure"], symbolId["ident"], symbolId[";"], symbolId["block"], symbolId[";"]},
            "proc_decl -> proc_decl procedure ident ; block ;");
        productions.emplace_back(symbolId["proc_decl"], 
            vector<int>{},
            "proc_decl -> ε");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{symbolId["ident"], symbolId[":="], symbolId["expression"]},
            "statement -> ident := expression");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{symbolId["call"], symbolId["ident"]},
            "statement -> call ident");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{symbolId["begin"], symbolId["statement_list"], symbolId["end"]},
            "statement -> begin statement_list end");
        productions.emplace_back(symbolId["statement_list"], 
            vector<int>{symbolId["statement"]},
            "statement_list -> statement");
        productions.emplace_back(symbolId["statement_list"], 
            vector<int>{symbolId["statement_list"], symbolId[";"], symbolId["statement"]},
            "statement_list -> statement_list ; statement");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{symbolId["if"], symbolId["condition"], symbolId["then"], symbolId["statement"]},
            "statement -> if condition then statement");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{symbolId["while"], symbolId["condition"], symbolId["do"], symbolId["statement"]},
            "statement -> while condition do statement");
        productions.emplace_back(symbolId["statement"], 
            vector<int>{},
            "statement -> ε");
        productions.emplace_back(symbolId["condition"], 
            vector<int>{symbolId["odd"], symbolId["expression"]},
            "condition -> odd expression");
        productions.emplace_back(symbolId["condition"], 
            vector<int>{symbolId["expression"], symbolId["relop"], symbolId["expression"]},
            "condition -> expression relop expression");
        productions.emplace_back(symbolId["expression"], 
            vector<int>{symbolId["term"]},
            "expression -> term");
        productions.emplace_back(symbolId["expression"], 
            vector<int>{symbolId["expression"], symbolId["+"], symbolId["term"]},
            "expression -> expression + term");
        productions.emplace_back(symbolId["expression"], 
            vector<int>{symbolId["expression"], symbolId["-"], symbolId["term"]},
            "expression -> expression - term");
        productions.emplace_back(symbolId["term"], 
            vector<int>{symbolId["factor"]},
            "term -> factor");
        productions.emplace_back(symbolId["term"], 
            vector<int>{symbolId["term"], symbolId["*"], symbolId["factor"]},
            "term -> term * factor");
        productions.emplace_back(symbolId["term"], 
            vector<int>{symbolId["term"], symbolId["/"], symbolId["factor"]},
            "term -> term / factor");
        productions.emplace_back(symbolId["factor"], 
            vector<int>{symbolId["ident"]},
            "factor -> ident");
        productions.emplace_back(symbolId["factor"], 
            vector<int>{symbolId["number"]},
            "factor -> number");
        productions.emplace_back(symbolId["factor"], 
            vector<int>{symbolId["("], symbolId["expression"], symbolId[")"]},
            "factor -> ( expression )");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId["="]},
            "relop -> =");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId["#"]},
            "relop -> #");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId["<"]},
            "relop -> <");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId[">"]},
            "relop -> >");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId["<="]},
            "relop -> <=");
        productions.emplace_back(symbolId["relop"], 
            vector<int>{symbolId[">="]},
            "relop -> >=");
    }

    // 计算项目集的闭包
    set<LRItem> closure(const set<LRItem>& items) {
        set<LRItem> result = items;
        bool changed;
        do {
            changed = false;
            set<LRItem> newItems;
            for (const auto& item : result) {
                const Production& prod = productions[item.prodId];
                if (item.dotPos < prod.right.size()) {
                    int nextSym = prod.right[item.dotPos];
                    // 如果下一个符号是非终结符
                    if (nextSym >= terminalCount) {
                        // 添加该非终结符的所有产生式的初始项目
                        for (int i=0; i<productions.size(); i++) {
                            if (productions[i].left == nextSym) {
                                LRItem newItem(i, 0);
                                if (!result.count(newItem) && !newItems.count(newItem)) {
                                    newItems.insert(newItem);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
            result.insert(newItems.begin(), newItems.end());
        } while (changed);
        return result;
    }

    // 转移函数go
    set<LRItem> go(const set<LRItem>& items, int symbol) {
        set<LRItem> result;
        for (const auto& item : items) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos < prod.right.size() && prod.right[item.dotPos] == symbol) {
                result.insert(LRItem(item.prodId, item.dotPos + 1));
            }
        }
        return closure(result);
    }

    // 生成项目集规范族
    void generateItemSets() {
        // 初始项目集：program -> · block .
        LRItem startItem(0, 0);
        set<LRItem> startSet = closure({startItem});
        itemSets.push_back(startSet);

        bool changed;
        do {
            changed = false;
            int currentSize = itemSets.size();
            for (int i=0; i<currentSize; i++) {
                // 收集所有可能的转移符号
                set<int> symbols;
                for (const auto& item : itemSets[i]) {
                    const Production& prod = productions[item.prodId];
                    if (item.dotPos < prod.right.size()) {
                        symbols.insert(prod.right[item.dotPos]);
                    }
                }

                // 对每个符号计算转移
                for (int sym : symbols) {
                    set<LRItem> nextSet = go(itemSets[i], sym);
                    if (nextSet.empty()) continue;

                    // 检查是否已存在
                    int found = -1;
                    for (int j=0; j<itemSets.size(); j++) {
                        if (itemSets[j] == nextSet) {
                            found = j;
                            break;
                        }
                    }

                    if (found == -1) {
                        itemSets.push_back(nextSet);
                        found = itemSets.size() - 1;
                        changed = true;
                    }

                    goTable[{i, sym}] = found;
                }
            }
        } while (changed);
    }

    // 计算FIRST集（简化版，仅用于FOLLOW集计算）
    set<int> first(int symbol) {
        set<int> result;
        if (symbol < terminalCount) {
            result.insert(symbol);
            return result;
        }

        for (int i=0; i<productions.size(); i++) {
            if (productions[i].left == symbol) {
                if (productions[i].right.empty()) {
                    // 产生式右部为空，不添加任何符号
                } else {
                    set<int> firstSym = first(productions[i].right[0]);
                    result.insert(firstSym.begin(), firstSym.end());
                }
            }
        }
        return result;
    }

    // 计算FOLLOW集
    void computeFollowSet() {
        // 开始符号的FOLLOW集包含$
        followSet[symbolId["program"]].insert(symbolId["$"]);

        bool changed;
        do {
            changed = false;
            for (int i=0; i<productions.size(); i++) {
                const Production& prod = productions[i];
                for (int j=0; j<prod.right.size(); j++) {
                    int B = prod.right[j];
                    if (B >= terminalCount) { // B是非终结符
                        // 情况1: A -> αBβ，添加FIRST(β)\{ε}到FOLLOW(B)
                        if (j+1 < prod.right.size()) {
                            set<int> firstBeta = first(prod.right[j+1]);
                            for (int sym : firstBeta) {
                                if (!followSet[B].count(sym)) {
                                    followSet[B].insert(sym);
                                    changed = true;
                                }
                            }
                        }

                        // 情况2: A -> αB 或 A -> αBβ且β能推出ε，添加FOLLOW(A)到FOLLOW(B)
                        bool canEpsilon = true;
                        for (int k=j+1; k<prod.right.size(); k++) {
                            set<int> firstK = first(prod.right[k]);
                            if (!firstK.empty()) {
                                canEpsilon = false;
                                break;
                            }
                        }

                        if (canEpsilon) {
                            int A = prod.left;
                            for (int sym : followSet[A]) {
                                if (!followSet[B].count(sym)) {
                                    followSet[B].insert(sym);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        } while (changed);
    }

    // 构造SLR(1)分析表
    void buildSLRTable() {
        for (int i=0; i<itemSets.size(); i++) {
            const auto& itemSet = itemSets[i];
            for (const auto& item : itemSet) {
                const Production& prod = productions[item.prodId];
                
                if (item.dotPos < prod.right.size()) {
                    // 移进项目
                    int a = prod.right[item.dotPos];
                    if (a < terminalCount) { // 终结符
                        int j = goTable[{i, a}];
                        actionTable[{i, a}] = {'s', j};
                    } else { // 非终结符
                        int j = goTable[{i, a}];
                        gotoTable[{i, a}] = j;
                    }
                } else {
                    // 归约项目或接受项目
                    if (item.prodId == 0) {
                        // 接受项目: program -> block . ·
                        actionTable[{i, symbolId["$"]}] = {'a', 0};
                    } else {
                        // 归约项目: A -> α ·
                        int A = prod.left;
                        for (int a : followSet[A]) {
                            actionTable[{i, a}] = {'r', item.prodId};
                        }
                    }
                }
            }
        }
    }

    // 将词法记号转换为符号编号
    int tokenToSymbol(const Token& tok) {
        switch(tok.type) {
            case TOK_IDENT: return symbolId["ident"];
            case TOK_NUMBER: return symbolId["number"];
            case TOK_CONST: return symbolId["const"];
            case TOK_VAR: return symbolId["var"];
            case TOK_PROCEDURE: return symbolId["procedure"];
            case TOK_BEGIN: return symbolId["begin"];
            case TOK_END: return symbolId["end"];
            case TOK_IF: return symbolId["if"];
            case TOK_THEN: return symbolId["then"];
            case TOK_WHILE: return symbolId["while"];
            case TOK_DO: return symbolId["do"];
            case TOK_CALL: return symbolId["call"];
            case TOK_ODD: return symbolId["odd"];
            case TOK_PLUS: return symbolId["+"];
            case TOK_MINUS: return symbolId["-"];
            case TOK_MULT: return symbolId["*"];
            case TOK_DIV: return symbolId["/"];
            case TOK_EQ: return symbolId["="];
            case TOK_NEQ: return symbolId["#"];
            case TOK_LT: return symbolId["<"];
            case TOK_GT: return symbolId[">"];
            case TOK_LE: return symbolId["<="];
            case TOK_GE: return symbolId[">="];
            case TOK_LPAREN: return symbolId["("];
            case TOK_RPAREN: return symbolId[")"];
            case TOK_COMMA: return symbolId[","];
            case TOK_SEMICOLON: return symbolId[";"];
            case TOK_ASSIGN: return symbolId[":="];
            case TOK_PERIOD: return symbolId["."];
            case TOK_EOF: return symbolId["$"];
            default: return -1;
        }
    }

    // 错误处理
    void error(int state, const Token& tok) {
        cerr << "第" << tok.line << "行错误: 语法错误，状态" << state 
             << "遇到意外符号 '" << tok.value << "'" << endl;
        hasError = true;
    }

public:
    SLRParser(Lexer& l) : lexer(l), tokenPtr(0), hasError(false) {
        initGrammar();
        generateItemSets();
        computeFollowSet();
        buildSLRTable();

        // 预读所有记号
        Token tok;
        do {
            tok = lexer.nextToken();
            tokens.push_back(tok);
        } while (tok.type != TOK_EOF);
        // 添加结束符
        tokens.emplace_back(TOK_EOF, "$", tokens.back().line);
    }

    // 执行语法分析
    bool parse() {
        cout << "==================== SLR(1)语法分析过程 ====================" << endl;
        cout << left << setw(6) << "步骤" << setw(20) << "状态栈" << setw(20) << "符号栈" 
             << setw(30) << "剩余输入" << "动作" << endl;
        cout << "------------------------------------------------------------------------" << endl;

        stack<int> stateStack;
        stack<string> symbolStack;
        stateStack.push(0);
        symbolStack.push("#");
        int step = 1;

        while (true) {
            int currentState = stateStack.top();
            Token currentTok = tokens[tokenPtr];
            int currentSym = tokenToSymbol(currentTok);

            // 输出当前状态
            string stateStr;
            stack<int> tempState = stateStack;
            while (!tempState.empty()) {
                stateStr = to_string(tempState.top()) + " " + stateStr;
                tempState.pop();
            }

            string symbolStr;
            stack<string> tempSymbol = symbolStack;
            while (!tempSymbol.empty()) {
                symbolStr = tempSymbol.top() + " " + symbolStr;
                tempSymbol.pop();
            }

            string inputStr;
            for (int i=tokenPtr; i<min(tokenPtr+5, (int)tokens.size()); i++) {
                inputStr += tokens[i].value + " ";
            }
            if (tokenPtr+5 < tokens.size()) inputStr += "...";

            // 查ACTION表
            if (!actionTable.count({currentState, currentSym})) {
                error(currentState, currentTok);
                return false;
            }

            auto action = actionTable[{currentState, currentSym}];
            char actionType = action.first;
            int actionParam = action.second;

            cout << left << setw(6) << step << setw(20) << stateStr 
                 << setw(20) << symbolStr << setw(30) << inputStr;

            if (actionType == 's') {
                // 移进
                cout << "移进 " << actionParam << endl;
                stateStack.push(actionParam);
                symbolStack.push(currentTok.value);
                tokenPtr++;
            } else if (actionType == 'r') {
                // 归约
                const Production& prod = productions[actionParam];
                cout << "归约: " << prod.desc << endl;

                // 弹出右部符号
                for (int i=0; i<prod.right.size(); i++) {
                    stateStack.pop();
                    symbolStack.pop();
                }

                // 压入左部符号
                int newState = gotoTable[{stateStack.top(), prod.left}];
                stateStack.push(newState);
                symbolStack.push(symbolName[prod.left]);
            } else if (actionType == 'a') {
                // 接受
                cout << "接受" << endl;
                cout << "------------------------------------------------------------------------" << endl;
                cout << "语法分析完成，程序语法正确！" << endl;
                return true;
            }

            step++;
        }
    }

    // 输出项目集规范族
    void printItemSets() {
        cout << endl << "==================== 项目集规范族 ====================" << endl;
        for (int i=0; i<itemSets.size(); i++) {
            cout << "I" << i << ":" << endl;
            for (const auto& item : itemSets[i]) {
                const Production& prod = productions[item.prodId];
                cout << "  " << symbolName[prod.left] << " -> ";
                for (int j=0; j<prod.right.size(); j++) {
                    if (j == item.dotPos) cout << "· ";
                    cout << symbolName[prod.right[j]] << " ";
                }
                if (item.dotPos == prod.right.size()) cout << "·";
                cout << endl;
            }
            cout << endl;
        }
    }

    // 输出SLR(1)分析表
    void printSLRTable() {
        cout << endl << "==================== SLR(1)分析表 ====================" << endl;
        cout << "状态 | ACTION表" << endl;
        cout << "-----|------------------------------------------------------------------------" << endl;
        for (int i=0; i<itemSets.size(); i++) {
            cout << setw(4) << i << " | ";
            for (int j=0; j<terminalCount; j++) {
                if (actionTable.count({i, j})) {
                    auto action = actionTable[{i, j}];
                    if (action.first == 's') cout << "s" << action.second << " ";
                    else if (action.first == 'r') cout << "r" << action.second << " ";
                    else if (action.first == 'a') cout << "acc ";
                }
            }
            cout << endl;
        }

        cout << endl << "状态 | GOTO表" << endl;
        cout << "-----|------------------------------------------------------------------------" << endl;
        for (int i=0; i<itemSets.size(); i++) {
            cout << setw(4) << i << " | ";
            for (int j=terminalCount; j<terminalCount+nonTerminalCount; j++) {
                if (gotoTable.count({i, j})) {
                    cout << symbolName[j] << ":" << gotoTable[{i, j}] << " ";
                }
            }
            cout << endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "用法: " << argv[0] << " <PL/0源文件>" << endl;
        return 1;
    }

    Lexer lexer(argv[1]);
    SLRParser parser(lexer);

    // 可选：输出项目集和分析表
    // parser.printItemSets();
    // parser.printSLRTable();

    return parser.parse() ? 0 : 1;
}