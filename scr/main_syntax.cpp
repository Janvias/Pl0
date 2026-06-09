/**
 * @file main_syntax.cpp
 * @brief PL/0 Syntax Analyzer (LL(1) + SLR(1))
 */

#include "include/syntax_analyzer.hpp"
#include <iostream>
#include <fstream>

using namespace std;

// Tokenize source code
vector<pair<string, string>> tokenize(const string& code) {
    vector<pair<string, string>> tokens;
    const set<string> keywords = {"const", "var", "procedure", "begin", "end", 
                                  "if", "then", "while", "do", "call", "read", "write", "odd"};
    
    for (size_t i = 0; i < code.length(); ) {
        char ch = code[i];
        
        // Skip whitespace
        if (isspace(ch)) { ++i; continue; }
        
        // Skip comments
        if (ch == '/' && i+1 < code.length() && code[i+1] == '/') {
            while (i < code.length() && code[i] != '\n') ++i;
            continue;
        }
        
        // Identifier/Keyword
        if (isalpha(ch)) {
            string id;
            while (i < code.length() && isalnum(code[i])) id += code[i++];
            tokens.emplace_back(keywords.count(id) ? id : "Ident", id);
            continue;
        }
        
        // Number
        if (isdigit(ch)) {
            string num;
            while (i < code.length() && isdigit(code[i])) num += code[i++];
            tokens.emplace_back("Number", num);
            continue;
        }
        
        // Two-char operators
        if ((ch == ':' || ch == '<' || ch == '>') && i+1 < code.length()) {
            char next = code[i+1];
            if ((ch == ':' && next == '=') || (ch == '<' && (next == '=' || next == '>')) || (ch == '>' && next == '=')) {
                string op(1, ch); op += next;
                tokens.emplace_back(op, op);
                i += 2; continue;
            }
        }
        
        // Single-char symbols
        if (string("+-*/=#<>;,.()").find(ch) != string::npos) {
            tokens.emplace_back(string(1, ch), string(1, ch));
            ++i; continue;
        }
        
        ++i;
    }
    return tokens;
}

// Read file content
bool readFile(const string& filename, string& content) {
    ifstream fin(filename);
    if (!fin) return false;
    content.clear();
    string line;
    while (getline(fin, line)) content += line + "\n";
    return true;
}

// Analyze and output result
void analyze(const string& code, const string& outFile, LL1Parser& ll1, SLRParser& slr, bool llOnly = false, bool slrOnly = false) {
    ofstream fout(outFile);
    auto tokens = tokenize(code);
    
    if (!slrOnly) {
        fout << "===== LL(1) Analysis =====\n";
        ll1.parse(tokens);
        for (const auto& l : ll1.getLog()) fout << "  " << l << "\n";
    }
    
    if (!llOnly) {
        fout << "\n===== SLR(1) Analysis =====\n";
        slr.parse(tokens);
        for (const auto& l : slr.getLog()) fout << "  " << l << "\n";
    }
    
    cout << "Result saved to: " << outFile << "\n";
}

// Show menu
void showMenu() {
    cout << "\nPL/0 Syntax Analyzer\n";
    cout << "=====================\n";
    cout << "1. LL(1) Analysis\n";
    cout << "2. SLR(1) Analysis\n";
    cout << "3. Grammar Info\n";
    cout << "4. Test Cases\n";
    cout << "5. LL+SLR Analysis\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

// Test cases
const vector<pair<string, string>> testCases = {
    {"Valid Program", "const n=10;\nvar x,y;\nbegin\n  x:=n;\n  y:=x+5;\n  if y>10 then write(y)\nend."},
    {"Missing Semicolon", "const n=10\nvar x;\nbegin\n  x:=n\nend."},
    {"Unmatched Parenthesis", "var x;\nbegin\n  x:=(1+2\nend."}
};

int main(int argc, char* argv[]) {
    cout << "PL/0 Syntax Analyzer (LL(1)+SLR(1))\n";
    cout << "Usage: syntax_analyzer.exe <input.txt> [output.txt]\n";
    
    LL1Parser ll1;
    SLRParser slr;
    
    // Command line mode
    if (argc >= 2) {
        string code;
        if (!readFile(argv[1], code)) {
            cout << "Error: Cannot open " << argv[1] << "\n";
            return 1;
        }
        string outFile = (argc >= 3) ? argv[2] : "result.txt";
        analyze(code, outFile, ll1, slr);
        return 0;
    }
    
    // Interactive mode
    int choice;
    do {
        showMenu();
        cin >> choice;
        cin.ignore();
        
        string inFile, outFile, code;
        switch (choice) {
            case 1:
                cout << "\nInput file: "; getline(cin, inFile);
                if (!readFile(inFile, code)) { cout << "Cannot open file\n"; break; }
                cout << "Output file: "; getline(cin, outFile);
                analyze(code, outFile, ll1, slr, true, false);
                break;
            case 2:
                cout << "\nInput file: "; getline(cin, inFile);
                if (!readFile(inFile, code)) { cout << "Cannot open file\n"; break; }
                cout << "Output file: "; getline(cin, outFile);
                analyze(code, outFile, ll1, slr, false, true);
                break;
            case 3:
                SyntaxVisualizer::printGrammar(ll1.getGrammar());
                SyntaxVisualizer::printStates(slr);
                break;
            case 4:
                cout << "\nOutput file: "; getline(cin, outFile);
                {
                    ofstream fout(outFile);
                    for (size_t i = 0; i < testCases.size(); ++i) {
                        fout << "\n===== Test: " << testCases[i].first << " =====\n";
                        fout << testCases[i].second << "\n";
                        auto tokens = tokenize(testCases[i].second);
                        
                        fout << "\n[LL(1)]\n";
                        ll1.parse(tokens);
                        for (const auto& l : ll1.getLog()) fout << "  " << l << "\n";
                        
                        fout << "\n[SLR(1)]\n";
                        slr.parse(tokens);
                        for (const auto& l : slr.getLog()) fout << "  " << l << "\n";
                    }
                }
                cout << "Result saved to: " << outFile << "\n";
                break;
            case 5:
                cout << "\nInput file: "; getline(cin, inFile);
                if (!readFile(inFile, code)) { cout << "Cannot open file\n"; break; }
                cout << "Output file: "; getline(cin, outFile);
                analyze(code, outFile, ll1, slr);
                break;
            case 0:
                cout << "\nGoodbye!\n";
                break;
            default:
                cout << "\nInvalid choice\n";
        }
        
        if (choice != 0) {
            cout << "\nPress Enter to continue...";
            cin.get();
        }
    } while (choice != 0);
    
    return 0;
}