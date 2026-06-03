#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <iomanip>
#include <stack>

using namespace std;

// 词法记号类型
enum TokenType {
    TOK_EOF, TOK_IDENT, TOK_NUMBER,
    // 关键字
    TOK_CONST, TOK_VAR, TOK_PROCEDURE, TOK_BEGIN, TOK_END,
    TOK_IF, TOK_THEN, TOK_WHILE, TOK_DO, TOK_CALL,
    TOK_ODD,
    // 运算符和分隔符
    TOK_PLUS, TOK_MINUS, TOK_MULT, TOK_DIV,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_LPAREN, TOK_RPAREN, TOK_COMMA, TOK_SEMICOLON,
    TOK_ASSIGN, TOK_PERIOD
};

// 记号结构体
struct Token {
    TokenType type;
    string value;
    int line;

    Token(TokenType t, const string& v, int l) : type(t), value(v), line(l) {}
};

// 四元式结构体
struct Quadruple {
    int op;         // 运算符（使用TokenType枚举）
    string arg1;    // 第一个参数
    string arg2;    // 第二个参数
    string result;  // 结果

    Quadruple(int o, const string& a1, const string& a2, const string& r)
        : op(o), arg1(a1), arg2(a2), result(r) {}
};

// 词法分析器类
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
        while (isspace(currentChar) && !file.eof()) {
            nextChar();
        }
    }

    Token identifierOrKeyword() {
        string id;
        while (isalnum(currentChar) && !file.eof()) {
            id += currentChar;
            nextChar();
        }

        if (keywords.find(id) != keywords.end()) {
            return Token(keywords[id], id, currentLine);
        }
        return Token(TOK_IDENT, id, currentLine);
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
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            exit(1);
        }

        // 初始化关键字映射
        keywords["const"] = TOK_CONST;
        keywords["var"] = TOK_VAR;
        keywords["procedure"] = TOK_PROCEDURE;
        keywords["begin"] = TOK_BEGIN;
        keywords["end"] = TOK_END;
        keywords["if"] = TOK_IF;
        keywords["then"] = TOK_THEN;
        keywords["while"] = TOK_WHILE;
        keywords["do"] = TOK_DO;
        keywords["call"] = TOK_CALL;
        keywords["odd"] = TOK_ODD;

        nextChar();
    }

    ~Lexer() {
        if (file.is_open()) {
            file.close();
        }
    }

    Token nextToken() {
        skipWhitespace();

        if (file.eof()) {
            return Token(TOK_EOF, "", currentLine);
        }

        if (isalpha(currentChar)) {
            return identifierOrKeyword();
        }

        if (isdigit(currentChar)) {
            return number();
        }

        // 处理运算符和分隔符
        char c = currentChar;
        nextChar();

        switch (c) {
            case '+': return Token(TOK_PLUS, "+", currentLine);
            case '-': return Token(TOK_MINUS, "-", currentLine);
            case '*': return Token(TOK_MULT, "*", currentLine);
            case '/': return Token(TOK_DIV, "/", currentLine);
            case '=': return Token(TOK_EQ, "=", currentLine);
            case '#': return Token(TOK_NEQ, "#", currentLine);
            case '<':
                if (currentChar == '=') {
                    nextChar();
                    return Token(TOK_LE, "<=", currentLine);
                }
                return Token(TOK_LT, "<", currentLine);
            case '>':
                if (currentChar == '=') {
                    nextChar();
                    return Token(TOK_GE, ">=", currentLine);
                }
                return Token(TOK_GT, ">", currentLine);
            case '(': return Token(TOK_LPAREN, "(", currentLine);
            case ')': return Token(TOK_RPAREN, ")", currentLine);
            case ',': return Token(TOK_COMMA, ",", currentLine);
            case ';': return Token(TOK_SEMICOLON, ";", currentLine);
            case '.': return Token(TOK_PERIOD, ".", currentLine);
            case ':':
                if (currentChar == '=') {
                    nextChar();
                    return Token(TOK_ASSIGN, ":=", currentLine);
                }
                cerr << "第" << currentLine << "行: 未知字符: :" << endl;
                exit(1);
            default:
                cerr << "第" << currentLine << "行: 未知字符: " << c << endl;
                exit(1);
        }
    }
};

// 语法分析器类（递归下降法 + 四元式生成）
class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    bool hasError;

    // 四元式相关
    vector<Quadruple> quads;       // 四元式列表
    int tempCount;                 // 临时变量计数器
    stack<int> jumpStack;          // 跳转回填栈

    // 错误处理
    void error(const string& message) {
        cerr << "第" << currentToken.line << "行错误: " << message << endl;
        hasError = true;
    }

    // 匹配当前记号并获取下一个
    void match(TokenType expected) {
        if (currentToken.type == expected) {
            currentToken = lexer.nextToken();
        } else {
            string expectedStr = tokenTypeToString(expected);
            string gotStr = tokenTypeToString(currentToken.type);
            error("期望 '" + expectedStr + "'，但得到 '" + gotStr + "'");
            
            // 错误恢复：跳过当前记号，继续分析
            currentToken = lexer.nextToken();
        }
    }

    // 记号类型转字符串
    string tokenTypeToString(TokenType type) {
        switch (type) {
            case TOK_EOF: return "EOF";
            case TOK_IDENT: return "标识符";
            case TOK_NUMBER: return "数字";
            case TOK_CONST: return "const";
            case TOK_VAR: return "var";
            case TOK_PROCEDURE: return "procedure";
            case TOK_BEGIN: return "begin";
            case TOK_END: return "end";
            case TOK_IF: return "if";
            case TOK_THEN: return "then";
            case TOK_WHILE: return "while";
            case TOK_DO: return "do";
            case TOK_CALL: return "call";
            case TOK_ODD: return "odd";
            case TOK_PLUS: return "+";
            case TOK_MINUS: return "-";
            case TOK_MULT: return "*";
            case TOK_DIV: return "/";
            case TOK_EQ: return "=";
            case TOK_NEQ: return "#";
            case TOK_LT: return "<";
            case TOK_GT: return ">";
            case TOK_LE: return "<=";
            case TOK_GE: return ">=";
            case TOK_LPAREN: return "(";
            case TOK_RPAREN: return ")";
            case TOK_COMMA: return ",";
            case TOK_SEMICOLON: return ";";
            case TOK_ASSIGN: return ":=";
            case TOK_PERIOD: return ".";
            default: return "未知";
        }
    }

    // 生成新的临时变量
    string newTemp() {
        return "t" + to_string(tempCount++);
    }

    // 生成四元式
    int genQuad(int op, const string& arg1, const string& arg2, const string& result) {
        quads.emplace_back(op, arg1, arg2, result);
        return quads.size() - 1;  // 返回四元式编号
    }

    // 回填四元式的result字段（跳转目标）
    void backpatch(int quadNo, int target) {
        if (quadNo >= 0 && quadNo < quads.size()) {
            quads[quadNo].result = to_string(target);
        }
    }

    // 回填栈顶的四元式
    void backpatchTop(int target) {
        if (!jumpStack.empty()) {
            int quadNo = jumpStack.top();
            jumpStack.pop();
            backpatch(quadNo, target);
        }
    }

    // 语法规则函数（递归下降 + 四元式生成）
    void program() {
        cout << "进入: program" << endl;
        block();
        match(TOK_PERIOD);
        genQuad(TOK_EOF, "", "", "");  // 生成程序结束四元式
        cout << "退出: program" << endl;
    }

    void block() {
        cout << "进入: block" << endl;
        if (currentToken.type == TOK_CONST) {
            constDeclaration();
        }
        if (currentToken.type == TOK_VAR) {
            varDeclaration();
        }
        while (currentToken.type == TOK_PROCEDURE) {
            procedureDeclaration();
        }
        statement();
        cout << "退出: block" << endl;
    }

    void constDeclaration() {
        cout << "进入: constDeclaration" << endl;
        match(TOK_CONST);
        constDefinition();
        while (currentToken.type == TOK_COMMA) {
            match(TOK_COMMA);
            constDefinition();
        }
        match(TOK_SEMICOLON);
        cout << "退出: constDeclaration" << endl;
    }

    void constDefinition() {
        cout << "进入: constDefinition" << endl;
        string name = currentToken.value;
        match(TOK_IDENT);
        match(TOK_EQ);
        string value = currentToken.value;
        match(TOK_NUMBER);
        // 常量定义不生成四元式（编译期替换）
        cout << "定义常量: " << name << " = " << value << endl;
        cout << "退出: constDefinition" << endl;
    }

    void varDeclaration() {
        cout << "进入: varDeclaration" << endl;
        match(TOK_VAR);
        cout << "定义变量: " << currentToken.value << endl;
        match(TOK_IDENT);
        while (currentToken.type == TOK_COMMA) {
            match(TOK_COMMA);
            cout << "定义变量: " << currentToken.value << endl;
            match(TOK_IDENT);
        }
        match(TOK_SEMICOLON);
        cout << "退出: varDeclaration" << endl;
    }

    void procedureDeclaration() {
        cout << "进入: procedureDeclaration" << endl;
        match(TOK_PROCEDURE);
        string procName = currentToken.value;
        match(TOK_IDENT);
        match(TOK_SEMICOLON);
        cout << "定义过程: " << procName << endl;
        block();
        match(TOK_SEMICOLON);
        cout << "退出: procedureDeclaration" << endl;
    }

    void statement() {
        cout << "进入: statement" << endl;
        switch (currentToken.type) {
            case TOK_IDENT: {
                // 赋值语句: ident := expression
                string name = currentToken.value;
                match(TOK_IDENT);
                match(TOK_ASSIGN);
                string exprResult = expression();
                genQuad(TOK_ASSIGN, exprResult, "", name);
                break;
            }
            case TOK_CALL: {
                // 过程调用: call ident
                match(TOK_CALL);
                string procName = currentToken.value;
                match(TOK_IDENT);
                genQuad(TOK_CALL, procName, "", "");
                break;
            }
            case TOK_BEGIN: {
                // 复合语句: begin statement {; statement} end
                match(TOK_BEGIN);
                statement();
                while (currentToken.type == TOK_SEMICOLON) {
                    match(TOK_SEMICOLON);
                    statement();
                }
                match(TOK_END);
                break;
            }
            case TOK_IF: {
                // 条件语句: if condition then statement
                match(TOK_IF);
                int falseJump = condition();  // 条件为假时的跳转
                match(TOK_THEN);
                statement();
                backpatch(falseJump, quads.size());  // 回填假出口
                break;
            }
            case TOK_WHILE: {
                // 循环语句: while condition do statement
                match(TOK_WHILE);
                int loopStart = quads.size();  // 记录循环开始位置
                int falseJump = condition();   // 条件为假时的跳转
                match(TOK_DO);
                statement();
                genQuad(TOK_GE, "", "", to_string(loopStart));  // 无条件跳回循环开始
                backpatch(falseJump, quads.size());  // 回填假出口
                break;
            }
            default:
                // 空语句，不生成四元式
                break;
        }
        cout << "退出: statement" << endl;
    }

    // 条件表达式翻译，返回假出口四元式编号
    int condition() {
        cout << "进入: condition" << endl;
        int falseJump;

        if (currentToken.type == TOK_ODD) {
            // odd expression
            match(TOK_ODD);
            string exprResult = expression();
            string temp = newTemp();
            genQuad(TOK_ODD, exprResult, "", temp);
            // 条件为假时跳转到下一条指令之后
            falseJump = genQuad(TOK_EQ, temp, "0", "0");
        } else {
            // expression relop expression
            string left = expression();
            int relop = currentToken.type;
            match(relop);
            string right = expression();
            
            // 生成条件跳转：条件不成立时跳转
            falseJump = genQuad(relop, left, right, "0");
        }

        cout << "退出: condition" << endl;
        return falseJump;
    }

    // 表达式翻译，返回结果临时变量名
    string expression() {
        cout << "进入: expression" << endl;
        string result;

        if (currentToken.type == TOK_PLUS || currentToken.type == TOK_MINUS) {
            int op = currentToken.type;
            match(op);
            string termResult = term();
            
            if (op == TOK_MINUS) {
                // 一元负号
                result = newTemp();
                genQuad(TOK_MINUS, "0", termResult, result);
            } else {
                // 一元正号，直接返回term结果
                result = termResult;
            }
        } else {
            result = term();
        }

        while (currentToken.type == TOK_PLUS || currentToken.type == TOK_MINUS) {
            int op = currentToken.type;
            match(op);
            string right = term();
            string temp = newTemp();
            genQuad(op, result, right, temp);
            result = temp;
        }

        cout << "退出: expression" << endl;
        return result;
    }

    // 项翻译，返回结果临时变量名
    string term() {
        cout << "进入: term" << endl;
        string result = factor();

        while (currentToken.type == TOK_MULT || currentToken.type == TOK_DIV) {
            int op = currentToken.type;
            match(op);
            string right = factor();
            string temp = newTemp();
            genQuad(op, result, right, temp);
            result = temp;
        }

        cout << "退出: term" << endl;
        return result;
    }

    // 因子翻译，返回结果临时变量名
    string factor() {
        cout << "进入: factor" << endl;
        string result;

        switch (currentToken.type) {
            case TOK_IDENT:
                result = currentToken.value;
                match(TOK_IDENT);
                break;
            case TOK_NUMBER:
                result = currentToken.value;
                match(TOK_NUMBER);
                break;
            case TOK_LPAREN:
                match(TOK_LPAREN);
                result = expression();
                match(TOK_RPAREN);
                break;
            default:
                error("期望标识符、数字或左括号");
                result = "";
                break;
        }

        cout << "退出: factor" << endl;
        return result;
    }

public:
    Parser(Lexer& l) : lexer(l), hasError(false), tempCount(0) {
        currentToken = lexer.nextToken();
    }

    bool parse() {
        cout << "开始语法分析..." << endl << endl;
        program();
        
        if (currentToken.type != TOK_EOF) {
            error("程序末尾有多余字符");
        }

        cout << endl << "语法分析完成。" << endl;
        if (hasError) {
            cout << "发现语法错误！" << endl;
            return false;
        } else {
            cout << "语法正确！" << endl;
            return true;
        }
    }

    // 输出四元式
    void printQuads() {
        cout << endl << "==================== 生成的四元式 ====================" << endl;
        cout << left << setw(6) << "编号" << setw(8) << "运算符" 
             << setw(12) << "参数1" << setw(12) << "参数2" << setw(12) << "结果" << endl;
        cout << "--------------------------------------------------------" << endl;

        for (int i = 0; i < quads.size(); i++) {
            const Quadruple& q = quads[i];
            cout << left << setw(6) << i 
                 << setw(8) << tokenTypeToString((TokenType)q.op)
                 << setw(12) << q.arg1 
                 << setw(12) << q.arg2 
                 << setw(12) << q.result << endl;
        }

        cout << "========================================================" << endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "用法: " << argv[0] << " <PL/0源文件>" << endl;
        return 1;
    }

    Lexer lexer(argv[1]);
    Parser parser(lexer);
    
    if (parser.parse()) {
        parser.printQuads();
        return 0;
    } else {
        return 1;
    }
}