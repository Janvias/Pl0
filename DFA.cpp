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

// ===================== 全局定义 =====================
typedef int StateID;
const char EPSILON = 0x01;
const char ESCAPE = 0x02;  // 转义标记
const char LETTER = 'L';  // 代表任意字母
const char DIGIT = 'D';   // 代表任意数字
StateID stateCount = 0;

// Token类型枚举
enum TokenType {
    KEYWORD,
    IDENT,
    NUMBER,
    DOUBLE_OP,
    SINGLE_OP,
    TOKEN_ERROR
};

// Token结构体
struct Token {
    TokenType type;
    string value;
    int line;
    int col;

    Token(TokenType t, string v, int l, int c) : type(t), value(v), line(l), col(c) {}
};

// 获取新状态ID
StateID newState() {
    return stateCount++;
}

// ===================== 1. 正则解析器 =====================
class RegexParser {
private:
    int precedence(char op) {
        if (op == '*') return 3;
        if (op == '+') return 3;
        if (op == '.') return 2;
        if (op == '|') return 1;
        return 0;
    }

    bool needConcat(char a, char b) {
        return (isalnum(a) || a == '*' || a == '+' || a == ')' || a == EPSILON || a == LETTER || a == DIGIT)
            && (isalnum(b) || b == '(' || b == EPSILON || b == LETTER || b == DIGIT);
    }

public:
    string infixToPostfix(string regex) {
        string processed;
        int n = regex.size();

        for (int i = 0; i < n; i++) {
            // 处理转义字符
            if (regex[i] == '\\' && i + 1 < n) {
                i++;  // 跳过转义符
                if (!processed.empty() && needConcat(processed.back(), ESCAPE)) {
                    processed += '.';
                }
                processed += ESCAPE;  // 添加转义标记
                processed += regex[i];
            } else {
                if (!processed.empty() && needConcat(processed.back(), regex[i])) {
                    processed += '.';
                }
                processed += regex[i];
            }
        }

        stack<char> opStack;
        string postfix;

        for (int i = 0; i < (int)processed.size(); i++) {
            char c = processed[i];
            // 处理转义字符
            if (c == ESCAPE && i + 1 < (int)processed.size()) {
                postfix += ESCAPE;
                postfix += processed[i + 1];
                i++;
            } else if (isalnum(c) || c == EPSILON || c == LETTER || c == DIGIT) {
                postfix += c;
            } else if (c == '(') {
                opStack.push(c);
            } else if (c == ')') {
                while (!opStack.empty() && opStack.top() != '(') {
                    postfix += opStack.top();
                    opStack.pop();
                }
                if (!opStack.empty()) opStack.pop();
            } else {
                while (!opStack.empty() && opStack.top() != '(' && precedence(opStack.top()) >= precedence(c)) {
                    postfix += opStack.top();
                    opStack.pop();
                }
                opStack.push(c);
            }
        }

        while (!opStack.empty()) {
            if (opStack.top() == '(') {
                opStack.pop();
                continue;
            }
            postfix += opStack.top();
            opStack.pop();
        }

        return postfix;
    }
};

// ===================== 2. NFA构造器（带Token类型） =====================
struct NFAEdge {
    StateID from, to;
    char ch;
    NFAEdge(StateID f, StateID t, char c) : from(f), to(t), ch(c) {}
};

struct NFASegment {
    StateID start;
    StateID end;
    TokenType tokenType;  // 该片段对应的Token类型
    NFASegment(StateID s, StateID e, TokenType t = TOKEN_ERROR) : start(s), end(e), tokenType(t) {}
};

class NFA_Builder {
public:
    vector<NFAEdge> edges;
    map<StateID, TokenType> finalStates;  // 终态 → Token类型

    // 构建单个正则的NFA片段
    NFASegment buildSegment(string postfix, TokenType type) {
        stack<NFASegment> st;
        for (int i = 0; i < (int)postfix.size(); i++) {
            char c = postfix[i];
            // 处理转义字符
            if (c == ESCAPE && i + 1 < (int)postfix.size()) {
                i++;
                char escapedChar = postfix[i];
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, e, escapedChar);
                st.push(NFASegment(s, e));
            }
            // 普通字符：不是操作符的字符都是普通字符
            else if (c != '.' && c != '|' && c != '*' && c != '+' && c != '(' && c != ')') {
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, e, c);
                st.push(NFASegment(s, e));
            } else if (c == '.' && st.size() >= 2) {
                NFASegment b = st.top(); st.pop();
                NFASegment a = st.top(); st.pop();
                edges.emplace_back(a.end, b.start, EPSILON);
                st.push(NFASegment(a.start, b.end));
            } else if (c == '|' && st.size() >= 2) {
                NFASegment b = st.top(); st.pop();
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(s, b.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(b.end, e, EPSILON);
                st.push(NFASegment(s, e));
            } else if (c == '*' && !st.empty()) {
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(a.end, a.start, EPSILON);
                edges.emplace_back(s, e, EPSILON);
                st.push(NFASegment(s, e));
            } else if (c == '+' && !st.empty()) {
                NFASegment a = st.top(); st.pop();
                StateID s = newState();
                StateID e = newState();
                edges.emplace_back(s, a.start, EPSILON);
                edges.emplace_back(a.end, e, EPSILON);
                edges.emplace_back(a.end, a.start, EPSILON);
                st.push(NFASegment(s, e));
            }
        }

        if (st.empty()) {
            StateID s = newState();
            StateID e = newState();
            return NFASegment(s, e);
        }
        
        NFASegment res = st.top();
        finalStates[res.end] = type;  // 标记终态对应的Token类型
        return res;
    }

    // 合并所有NFA片段为一个总NFA
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

    set<StateID> move(set<StateID> S, char c) {
        set<StateID> res;
        for (StateID s : S) {
            for (auto& e : edges) {
                if (e.from == s) {
                    if (e.ch == c) {
                        res.insert(e.to);
                    } else if (e.ch == LETTER && isalpha(c)) {
                        res.insert(e.to);
                    } else if (e.ch == DIGIT && isdigit(c)) {
                        res.insert(e.to);
                    }
                }
            }
        }
        return res;
    }

    set<char> getAllChars() {
        set<char> chs;
        chs.insert(LETTER);  // 添加字符类
        chs.insert(DIGIT);   // 添加数字类
        for (auto& e : edges) {
            if (e.ch != EPSILON) {
                chs.insert(e.ch);
            }
        }
        return chs;
    }
};

// ===================== 3. DFA构造器（带Token类型） =====================
struct DFAState {
    set<StateID> nfaStates;
    bool isFinal = false;
    TokenType tokenType = TOKEN_ERROR;  // 终态对应的Token类型
    map<char, int> trans;
};

class DFA_Subset {
public:
    vector<DFAState> dfaStates;
    map<set<StateID>, int> idMap;

    void buildDFA(NFA_Builder& nfa, NFASegment& merged) {
        dfaStates.clear();
        idMap.clear();
        queue<int> q;

        set<StateID> startSet;
        startSet.insert(merged.start);
        set<StateID> startClosure = nfa.epsilonClosure(startSet);
        idMap[startClosure] = 0;
        dfaStates.emplace_back();
        dfaStates[0].nfaStates = startClosure;

        // 标记终态和Token类型（优先级：先出现的优先级高）
        for (StateID s : startClosure) {
            if (nfa.finalStates.count(s)) {
                dfaStates[0].isFinal = true;
                dfaStates[0].tokenType = nfa.finalStates[s];
                break;
            }
        }
        q.push(0);

        set<char> chars = nfa.getAllChars();

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (char c : chars) {
                set<StateID> to = nfa.epsilonClosure(nfa.move(dfaStates[u].nfaStates, c));
                if (to.empty()) continue;

                if (!idMap.count(to)) {
                    idMap[to] = dfaStates.size();
                    dfaStates.emplace_back();
                    dfaStates.back().nfaStates = to;

                    // 标记终态和Token类型
                    for (StateID s : to) {
                        if (nfa.finalStates.count(s)) {
                            dfaStates.back().isFinal = true;
                            dfaStates.back().tokenType = nfa.finalStates[s];
                            break;
                        }
                    }
                    q.push(idMap[to]);
                }
                dfaStates[u].trans[c] = idMap[to];
            }
        }
    }
};

// ===================== 4. DFA最小化器（带Token类型） =====================
class DFA_Minimize {
public:
    vector<DFAState> minDfa;
    map<int, int> groupId;

    void minimize(DFA_Subset& dfa) {
        int n = dfa.dfaStates.size();
        if (n == 0) return;

        // 初始划分：按Token类型分组（不同Token类型不能合并）
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
            for (int x : p) groupId[x] = idx;
            minDfa.emplace_back();
            minDfa.back().isFinal = dfa.dfaStates[*p.begin()].isFinal;
            minDfa.back().tokenType = dfa.dfaStates[*p.begin()].tokenType;
            idx++;
        }

        for (int i = 0; i < n; i++) {
            int g = groupId[i];
            for (auto& t : dfa.dfaStates[i].trans) {
                char c = t.first;
                int to = t.second;
                minDfa[g].trans[c] = groupId[to];
            }
        }
    }
};

// ===================== 5. 词法分析器驱动 =====================
class Lexer {
private:
    DFA_Minimize& dfa;
    string src;
    int pos = 0;
    int line = 1;
    int col = 1;
    vector<Token> tokens;
    vector<string> errors;

    // 跳过空白符
    void skipWhitespace() {
        while (pos < src.size() && isspace(src[pos])) {
            if (src[pos] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            pos++;
        }
    }

    // 跳过注释
    void skipComment() {
        if (pos + 1 < src.size() && src[pos] == '/' && src[pos+1] == '/') {
            pos += 2;
            col += 2;
            while (pos < src.size() && src[pos] != '\n') {
                pos++;
                col++;
            }
        } else if (pos + 1 < src.size() && src[pos] == '/' && src[pos+1] == '*') {
            int startLine = line;
            int startCol = col;
            pos += 2;
            col += 2;
            while (pos + 1 < src.size()) {
                if (src[pos] == '*' && src[pos+1] == '/') {
                    pos += 2;
                    col += 2;
                    return;
                }
                if (src[pos] == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                pos++;
            }
            errors.push_back("行" + to_string(startLine) + "列" + to_string(startCol) + " 错误：多行注释未闭合");
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

        // 检查DFA是否为空
        if (dfa.minDfa.empty()) {
            errors.push_back("错误：DFA为空");
            return tokens;
        }

        while (pos < src.size()) {
            skipWhitespace();
            if (pos >= src.size()) break;
            skipComment();
            if (pos >= src.size()) break;

            int startPos = pos;
            int startLine = line;
            int startCol = col;
            int currentState = 0;
            int lastFinalState = -1;
            int lastFinalPos = pos;

            // 最长匹配
            while (pos < src.size()) {
                // 检查状态是否有效
                if (currentState < 0 || currentState >= (int)dfa.minDfa.size()) {
                    break;
                }

                char c = src[pos];
                // 字符类映射
                char input = c;
                if (isalpha(c)) input = LETTER;
                else if (isdigit(c)) input = DIGIT;

                if (dfa.minDfa[currentState].trans.count(input)) {
                    int nextState = dfa.minDfa[currentState].trans[input];
                    // 检查下一个状态是否有效
                    if (nextState < 0 || nextState >= (int)dfa.minDfa.size()) {
                        break;
                    }
                    currentState = nextState;
                    pos++;
                    if (c == '\n') {
                        line++;
                        col = 1;
                    } else {
                        col++;
                    }

                    // 记录最后一个终态
                    if (dfa.minDfa[currentState].isFinal) {
                        lastFinalState = currentState;
                        lastFinalPos = pos;
                    }
                } else {
                    break;
                }
            }

            // 处理匹配结果
            if (lastFinalState != -1 && lastFinalState < (int)dfa.minDfa.size()) {
                string value = src.substr(startPos, lastFinalPos - startPos);
                TokenType type = dfa.minDfa[lastFinalState].tokenType;
                tokens.emplace_back(type, value, startLine, startCol);
                pos = lastFinalPos;
                // 恢复行号列号
                line = startLine;
                col = startCol;
                for (int i = startPos; i < lastFinalPos; i++) {
                    if (src[i] == '\n') {
                        line++;
                        col = 1;
                    } else {
                        col++;
                    }
                }
            } else {
                // 非法字符
                errors.push_back("行" + to_string(startLine) + "列" + to_string(startCol) + " 错误：非法字符 '" + string(1, src[pos]) + "'");
                pos++;
                col++;
            }
        }

        return tokens;
    }

    vector<string> getErrors() {
        return errors;
    }
};

// ===================== 主函数 =====================
int main() {
    // 1. 定义所有PL/0词法规则（优先级从高到低）
    vector<pair<string, TokenType>> rules = {
        // 关键字（最高优先级）
        {"const", KEYWORD},
        {"var", KEYWORD},
        {"procedure", KEYWORD},
        {"begin", KEYWORD},
        {"end", KEYWORD},
        {"if", KEYWORD},
        {"then", KEYWORD},
        {"while", KEYWORD},
        {"do", KEYWORD},
        {"call", KEYWORD},
        {"odd", KEYWORD},
        {"read", KEYWORD},
        {"write", KEYWORD},
        // 双字符运算符
        {":=", DOUBLE_OP},
        {"<=", DOUBLE_OP},
        {">=", DOUBLE_OP},
        {"<>", DOUBLE_OP},
        // 单字符运算符（使用转义字符）
        {"\\+", SINGLE_OP},
        {"-", SINGLE_OP},
        {"\\*", SINGLE_OP},
        {"/", SINGLE_OP},
        {"=", SINGLE_OP},
        {"#", SINGLE_OP},
        {"<", SINGLE_OP},
        {">", SINGLE_OP},
        {",", SINGLE_OP},
        {";", SINGLE_OP},
        {"\\(", SINGLE_OP},
        {"\\)", SINGLE_OP},
        {"\\.", SINGLE_OP},
        // 标识符和数字（最低优先级）
        {"L(L|D)*", IDENT},
        {"D+", NUMBER}
    };

    // 2. 生成总NFA
    RegexParser parser;
    NFA_Builder nfaBuilder;
    vector<NFASegment> segments;

    for (auto& rule : rules) {
        string postfix = parser.infixToPostfix(rule.first);
        NFASegment seg = nfaBuilder.buildSegment(postfix, rule.second);
        segments.push_back(seg);
    }

    // 合并所有片段
    NFASegment merged = nfaBuilder.mergeAllSegments(segments);
    cout << "总NFA状态数：" << stateCount << endl;

    // 3. 生成总DFA
    DFA_Subset dfaBuilder;
    dfaBuilder.buildDFA(nfaBuilder, merged);
    cout << "总DFA状态数：" << dfaBuilder.dfaStates.size() << endl;

    // 4. 最小化总DFA
    DFA_Minimize dfaMin;
    dfaMin.minimize(dfaBuilder);
    cout << "最小化总DFA状态数：" << dfaMin.minDfa.size() << endl;
    cout << "✅ 单DFA生成完成！" << endl;

    // 5. 测试PL/0源码
    string testCode = R"(
// PL/0测试程序
const MAX=100;
var a,b,c;
procedure p;
begin
    a := a + b;
end;
begin
    read(a,b);
    while a < b do
    begin
        call p;
        write(a);
    end;
end.
)";

    cout << "\n==================== 词法分析结果 ====================" << endl;
    Lexer lexer(dfaMin, testCode);
    vector<Token> tokens = lexer.analyse();

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
        cout << "(" << typeStr << ", \"" << t.value << "\", 行" << t.line << "列" << t.col << ")" << endl;
    }

    cout << "\n==================== 错误信息 ====================" << endl;
    vector<string> errors = lexer.getErrors();
    if (errors.empty()) {
        cout << "无词法错误" << endl;
    } else {
        for (auto& e : errors) {
            cout << e << endl;
        }
    }

    return 0;
}