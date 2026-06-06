#include "regex_parser.h"
#include <stack>
#include <cctype>

std::string RegexParser::insertConcat(const std::string& regex) {
    std::string result;
    int i = 0;
    while (i < regex.size()) {
        if (regex[i] == '[') {
            // 字符类，找到对应的']'
            result += regex[i];
            i++;
            while (i < regex.size() && regex[i] != ']') {
                result += regex[i];
                i++;
            }
            if (i < regex.size()) {
                result += regex[i];
                i++;
            }
            // 检查是否需要添加连接符
            if (i < regex.size()) {
                char next = regex[i];
                if ((isalnum(next) || next == '(' || next == '[')) {
                    result += '.';
                }
            }
        } else {
            result += regex[i];
            if (i + 1 < regex.size()) {
                char current = regex[i];
                char next = regex[i + 1];
                
                bool needConcat = false;
                if ((isalnum(current) || current == ')' || current == '*' || current == '+' || current == ']') && 
                    (isalnum(next) || next == '(' || next == '[')) {
                    needConcat = true;
                }
                
                if (needConcat) {
                    result += '.';
                }
            }
            i++;
        }
    }
    return result;
}

int RegexParser::precedence(char op) {
    switch (op) {
        case '*': case '+': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}

std::string RegexParser::shuntingYard(const std::string& infix) {
    std::string postfix;
    std::stack<char> opStack;
    
    int i = 0;
    while (i < infix.size()) {
        if (infix[i] == '[') {
            // 字符类，直接添加到后缀表达式
            postfix += infix[i];
            i++;
            while (i < infix.size() && infix[i] != ']') {
                postfix += infix[i];
                i++;
            }
            if (i < infix.size()) {
                postfix += infix[i];
                i++;
            }
        } else if (isalnum(infix[i])) {
            postfix += infix[i];
            i++;
        } else if (infix[i] == '(') {
            opStack.push(infix[i]);
            i++;
        } else if (infix[i] == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.pop();
            i++;
        } else {
            while (!opStack.empty() && opStack.top() != '(' && 
                   precedence(opStack.top()) >= precedence(infix[i])) {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.push(infix[i]);
            i++;
        }
    }
    
    while (!opStack.empty()) {
        postfix += opStack.top();
        opStack.pop();
    }
    
    return postfix;
}
