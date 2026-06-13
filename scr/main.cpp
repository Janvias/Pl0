/**
 * @file main.cpp
 * @brief PL/0编译器入口点
 * @details 现代C++ PL/0编译器，支持双LL(1)+LR(1)验证、
 *          AST可视化、错误建议和语法规范化
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 * @version 4.0 (扩展功能版)
 */

#include <iostream>
#include <string>
#include "include/pl0_compiler.hpp"

using namespace PL0;

//============================================================================
// 打印使用说明
//============================================================================

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " <source_file> [options]\n\n";
    std::cout << "Parser Modes:\n";
    std::cout << "  --ll1            LL(1) recursive descent only\n";
    std::cout << "  --lr1            LR(1) shift-reduce only\n";
    std::cout << "  --dual           Dual parser cross-validation (default)\n";
    std::cout << "  -m, --mode MODE  ll1 / lr1 / dual\n\n";
    std::cout << "Bonus Features:\n";
    std::cout << "  --ast            Build AST (abstract syntax tree)\n";
    std::cout << "  --dot FILE       Output AST in Graphviz DOT format\n";
    std::cout << "  --suggest        Show error fix suggestions on failure\n";
    std::cout << "  --normalize      Show grammar normalization status\n\n";
    std::cout << "DFA Visualization Dumps:\n";
    std::cout << "  --dump-nfa FILE      Output NFA state diagram (.dot)\n";
    std::cout << "  --dump-dfa FILE      Output DFA state diagram (.dot)\n";
    std::cout << "  --dump-min-dfa FILE  Output minimized DFA diagram (.dot)\n";
    std::cout << "  --dump-classify FILE Output token classification diagram (.dot)\n";
    std::cout << "  --dump-report FILE   Output lexical analysis report (.txt)\n";
    std::cout << "  --dump-all PREFIX    Output all visualizations with given prefix\n\n";
    std::cout << "Other:\n";
    std::cout << "  -v, --verbose    Verbose output\n";
    std::cout << "  -h, --help       This help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog << " test.pl0 --ast --dot ast.dot\n";
    std::cout << "  " << prog << " test.pl0 --suggest --dual\n";
}

//============================================================================
// 获取输出文件名（添加后缀）
//============================================================================

std::string getOutputFilename(const std::string& input, const std::string& suffix) {
    size_t dot = input.rfind('.');
    if (dot != std::string::npos) return input.substr(0, dot) + suffix;
    return input + suffix;
}

//============================================================================
// 主函数
//============================================================================

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string inputFile, dotFile;
    bool verbose = false;
    ParserMode mode = ParserMode::DUAL;
    bool ast = false, suggest = false, normalize = false;

    // DFA可视化输出文件名
    std::string dumpNFAFile, dumpDFAFile, dumpMinDFAFile;
    std::string dumpClassifyFile, dumpReportFile, dumpAllPrefix;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "-v" || a == "--verbose") verbose = true;
        else if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else if (a == "--ll1")  mode = ParserMode::LL1_ONLY;
        else if (a == "--lr1")  mode = ParserMode::LR1_ONLY;
        else if (a == "--dual") mode = ParserMode::DUAL;
        else if (a == "--ast")  ast = true;
        else if (a == "--suggest") suggest = true;
        else if (a == "--normalize") normalize = true;
        else if (a == "--dot") {
            if (i + 1 < argc) dotFile = argv[++i];
            else { std::cerr << "Missing filename for --dot\n"; return 1; }
        }
        else if (a == "-m" || a == "--mode") {
            if (i + 1 >= argc) { std::cerr << "Missing mode\n"; return 1; }
            std::string ms = argv[++i];
            if (ms == "ll1") mode = ParserMode::LL1_ONLY;
            else if (ms == "lr1") mode = ParserMode::LR1_ONLY;
            else if (ms == "dual") mode = ParserMode::DUAL;
            else { std::cerr << "Unknown mode: " << ms << "\n"; return 1; }
        }
        else if (a == "--dump-nfa") {
            if (i + 1 < argc) dumpNFAFile = argv[++i];
            else { std::cerr << "Missing filename for --dump-nfa\n"; return 1; }
        }
        else if (a == "--dump-dfa") {
            if (i + 1 < argc) dumpDFAFile = argv[++i];
            else { std::cerr << "Missing filename for --dump-dfa\n"; return 1; }
        }
        else if (a == "--dump-min-dfa") {
            if (i + 1 < argc) dumpMinDFAFile = argv[++i];
            else { std::cerr << "Missing filename for --dump-min-dfa\n"; return 1; }
        }
        else if (a == "--dump-classify") {
            if (i + 1 < argc) dumpClassifyFile = argv[++i];
            else { std::cerr << "Missing filename for --dump-classify\n"; return 1; }
        }
        else if (a == "--dump-report") {
            if (i + 1 < argc) dumpReportFile = argv[++i];
            else { std::cerr << "Missing filename for --dump-report\n"; return 1; }
        }
        else if (a == "--dump-all") {
            if (i + 1 < argc) dumpAllPrefix = argv[++i];
            else { std::cerr << "Missing prefix for --dump-all\n"; return 1; }
        }
        else if (a[0] != '-') inputFile = a;
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }

    // --ast选项如果没有指定--dot文件，则自动生成
    if (ast && dotFile.empty()) {
        dotFile = getOutputFilename(inputFile, ".dot");
    }

    try {
        Compiler compiler;
        compiler.setVerbose(verbose);
        compiler.setParserMode(mode);
        compiler.enableAST(ast);
        compiler.enableSuggest(suggest);
        compiler.enableNormalize(normalize);

        if (!compiler.compile(inputFile)) {
            std::cerr << "Compilation failed: " << compiler.getErrorMessage() << "\n";
            if (suggest) {
                std::cout << compiler.getErrorSuggestions();
            }
            return 1;
        }

        // 输出结果
        compiler.printResults();

        // AST DOT输出
        if (ast && compiler.isASTEnabled()) {
            compiler.writeASTDOT(dotFile);
            std::cout << "AST DOT: " << dotFile << "\n";
            std::cout << "Render:  dot -Tpng " << dotFile << " -o "
                      << getOutputFilename(dotFile, ".png") << "\n";
        }

        // 语法规范化报告
        if (normalize) {
            std::cout << compiler.getNormalizedGrammar();
        }

        // 输出四元式报告
        compiler.writeQuadReport(getOutputFilename(inputFile, "_quad.txt"));

        // DFA可视化输出
        if (!dumpNFAFile.empty())     compiler.dumpNFA(dumpNFAFile);
        if (!dumpDFAFile.empty())     compiler.dumpDFA(dumpDFAFile);
        if (!dumpMinDFAFile.empty())  compiler.dumpMinDFA(dumpMinDFAFile);
        if (!dumpClassifyFile.empty()) compiler.dumpClassification(dumpClassifyFile);
        if (!dumpReportFile.empty())  compiler.dumpReport(dumpReportFile);
        if (!dumpAllPrefix.empty())   compiler.dumpAll(dumpAllPrefix);

        std::cout << "\nCompilation completed successfully.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
