/**
 * @file regex_parser.cpp
 * @brief 正则表达式解析器实现
 * @details 实现正则表达式的预处理、连接符插入和调度场算法
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "regex_parser.h"
#include <stack>
#include <cctype>

//============================================================================
// 插入显式连接符
//============================================================================
// 在需要连接的位置插入'.'运算符
// 例如：ab -> a.b, a[b-c] -> a.[b-c]
//============================================================================

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
                // 当前字符是字母数字、右括号、闭包或字符类结束，且下一个字符是字母数字、左括号或字符类开始
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

//============================================================================
// 扩展{n}量词
//============================================================================
// 将X{n}展开为X重复n次
// 支持字符类[a-z]{n}、分组(...)和单字符操作数
//============================================================================

std::string RegexParser::expandQuantifiers(const std::string& regex) {
    std::string result;
    result.reserve(regex.size() * 2);

    size_t i = 0;
    while (i < regex.size()) {
        if (regex[i] == '{') {
            // 解析{n}中的数字
            size_t j = i + 1;
            int count = 0;
            while (j < regex.size() && std::isdigit(regex[j])) {
                count = count * 10 + (regex[j] - '0');
                j++;
            }
            // 必须后跟'}'
            if (j < regex.size() && regex[j] == '}' && count > 0) {
                // 查找{n}之前的操作数
                if (!result.empty() && result.back() == ']') {
                    // 字符类：找到匹配的'['
                    size_t opStart = result.rfind('[');
                    if (opStart != std::string::npos) {
                        std::string charClass = result.substr(opStart);
                        result.erase(opStart);
                        for (int k = 0; k < count; k++) {
                            result += charClass;
                        }
                        i = j + 1;
                        continue;
                    }
                }
                if (!result.empty() && result.back() == ')') {
                    // 分组：找到匹配的'('（简化处理：找最后一个'('）
                    size_t opStart = result.rfind('(');
                    if (opStart != std::string::npos) {
                        std::string group = result.substr(opStart);
                        result.erase(opStart);
                        for (int k = 0; k < count; k++) {
                            result += group;
                        }
                        i = j + 1;
                        continue;
                    }
                }
                if (!result.empty() && (std::isalnum(static_cast<unsigned char>(result.back())) ||
                         result.back() == '*' || result.back() == '+')) {
                    // 单字符或* / +修饰符
                    char last = result.back();
                    result.pop_back();
                    for (int k = 0; k < count; k++) {
                        result += last;
                    }
                    i = j + 1;
                    continue;
                }
                // 无法识别操作数，将{作为普通字符处理
            }
            // 不是有效的{n}，将{作为普通字符处理
            result += '{';
            i++;
        } else {
            result += regex[i];
            i++;
        }
    }

    return result;
}

//============================================================================
// 运算符优先级
//============================================================================
// *、+：优先级3（最高）
// .：优先级2（连接）
// |：优先级1（选择）
//============================================================================

int RegexParser::precedence(char op) {
    switch (op) {
        case '*': case '+': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}

//============================================================================
// 调度场算法转换为后缀
//============================================================================
// 使用调度场算法将中缀正则表达式转换为后缀形式
// 算法步骤：
// 1. 遍历中缀表达式
// 2. 字符直接输出到后缀
// 3. 运算符入栈，根据优先级处理
// 4. 括号用于分组
//============================================================================

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
            // 字母数字直接输出
            postfix += infix[i];
            i++;
        } else if (infix[i] == '(') {
            // 左括号入栈
            opStack.push(infix[i]);
            i++;
        } else if (infix[i] == ')') {
            // 右括号：弹出栈中运算符直到遇到左括号
            while (!opStack.empty() && opStack.top() != '(') {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.pop();  // 弹出左括号
            i++;
        } else {
            // 运算符：弹出栈中优先级更高或相同的运算符
            while (!opStack.empty() && opStack.top() != '(' && 
                   precedence(opStack.top()) >= precedence(infix[i])) {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.push(infix[i]);
            i++;
        }
    }
    
    // 弹出栈中剩余运算符
    while (!opStack.empty()) {
        postfix += opStack.top();
        opStack.pop();
    }
    
    return postfix;
}
