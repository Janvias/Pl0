#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "../pl0_regex/pl0_regex.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"

using namespace std;

// 读取输入文件
bool readInput(const string& filename, string& regex, vector<string>& testStrings) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return false;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        size_t colonPos = line.find(':');
        if (colonPos == string::npos) continue;
        
        string key = line.substr(0, colonPos);
        string value = line.substr(colonPos + 1);
        
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "正则表达式") {
            regex = value;
        } else if (key == "测试字符串") {
            stringstream ss(value);
            string token;
            while (getline(ss, token, ',')) {
                if (!token.empty()) {
                    testStrings.push_back(token);
                }
            }
        }
    }
    
    file.close();
    return !regex.empty();
}

// 输出DFA信息
void printDFA(DFA* dfa, ostream& os) {
    os << "==================== DFA 信息 ====================" << endl;
    os << "字母表: ";
    for (char c : dfa->alphabet) {
        os << c << " ";
    }
    os << endl << endl;
    
    os << "状态总数: " << dfa->states.size() << endl;
    os << "起始状态: " << dfa->startState->id << endl;
    os << "接受状态: ";
    for (DFAState* state : dfa->acceptStates) {
        os << state->id << " ";
    }
    os << endl << endl;
    
    os << "状态转移表:" << endl;
    os << "状态\t";
    for (char c : dfa->alphabet) {
        os << c << "\t";
    }
    os << endl;
    
    for (DFAState* state : dfa->states) {
        os << state->id << (state->isAccept ? "*" : "") << "\t";
        for (char c : dfa->alphabet) {
            if (state->transitions.count(c)) {
                os << state->transitions[c]->id << "\t";
            } else {
                os << "-\t";
            }
        }
        os << endl;
    }
    os << endl;
}

// 输出测试结果
void printTestResults(DFA* dfa, const vector<string>& testStrings, ostream& os) {
    if (testStrings.empty()) return;
    
    os << "==================== 测试结果 ====================" << endl;
    for (const string& s : testStrings) {
        os << "字符串 \"" << s << "\": " << (dfa->accepts(s) ? "接受" : "拒绝") << endl;
    }
    os << endl;
}

int dfa_main(int argc, char* argv[]) {
    string inputFilename = "input.txt";
    string outputFilename = "output.txt";
    
    if (argc >= 2) {
        inputFilename = argv[1];
    }
    if (argc >= 3) {
        outputFilename = argv[2];
    }
    
    // 方式1：从输入文件读取正则
    string regex;
    vector<string> testStrings;
    
    if (!readInput(inputFilename, regex, testStrings)) {
        // 方式2：使用内置PL/0正则（示例：标识符正则）
        cout << "未找到输入文件，使用内置PL/0标识符正则进行演示" << endl;
        regex = PL0Regex::REG_IDENT;
        testStrings = {"x", "var123", "123var", "a_b", "PROC"};
    }
    
    cout << "输入正则表达式: " << regex << endl << endl;
    
    // 正则预处理
    string regexWithConcat = RegexParser::insertConcat(regex);
    cout << "插入拼接符后: " << regexWithConcat << endl;
    
    string postfix = RegexParser::shuntingYard(regexWithConcat);
    cout << "后缀表达式: " << postfix << endl << endl;
    
    // 生成NFA
    cout << "正在生成NFA..." << endl;
    NFA* nfa = NFA::thompsonConstruction(postfix);
    cout << "NFA生成完成，状态数: " << nfa->states.size() << endl << endl;
    
    // 生成DFA
    cout << "正在将NFA转换为DFA..." << endl;
    DFA* dfa = DFA::nfaToDFA(nfa);
    cout << "DFA生成完成，状态数: " << dfa->states.size() << endl << endl;
    
    // 最小化DFA
    cout << "正在最小化DFA..." << endl;
    DFA* minDFA = dfa->minimize();
    cout << "DFA最小化完成，状态数: " << minDFA->states.size() << endl << endl;
    
    // 输出结果
    ofstream outputFile(outputFilename);
    
    printDFA(minDFA, cout);
    printTestResults(minDFA, testStrings, cout);
    
    if (outputFile.is_open()) {
        outputFile << "输入正则表达式: " << regex << endl << endl;
        printDFA(minDFA, outputFile);
        printTestResults(minDFA, testStrings, outputFile);
        outputFile.close();
        cout << "结果已保存到文件: " << outputFilename << endl;
    }
    
    // 释放内存
    delete nfa;
    delete dfa;
    delete minDFA;
    
    return 0;
}