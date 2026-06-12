/**
 * @file main.cpp
 * @brief PL/0 Compiler Entry Point
 * @description Modern C++ PL/0 compiler with dual LL(1)+LR(1) validation,
 *              AST visualization, error suggestions, and grammar normalization.
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 * @version 4.0 (Bonus Features Edition)
 */

#include <iostream>
#include <string>
#include "include/pl0_compiler.hpp"

using namespace PL0;

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
    std::cout << "Other:\n";
    std::cout << "  -v, --verbose    Verbose output\n";
    std::cout << "  -h, --help       This help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog << " test.pl0 --ast --dot ast.dot\n";
    std::cout << "  " << prog << " test.pl0 --suggest --dual\n";
}

std::string getOutputFilename(const std::string& input, const std::string& suffix) {
    size_t dot = input.rfind('.');
    if (dot != std::string::npos) return input.substr(0, dot) + suffix;
    return input + suffix;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string inputFile, dotFile;
    bool verbose = false;
    ParserMode mode = ParserMode::DUAL;
    bool ast = false, suggest = false, normalize = false;

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
        else if (a[0] != '-') inputFile = a;
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }

    // --ast implies --dot if no explicit --dot file given
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

        // Results
        compiler.printResults();

        // AST DOT output
        if (ast && compiler.isASTEnabled()) {
            compiler.writeASTDOT(dotFile);
            std::cout << "AST DOT: " << dotFile << "\n";
            std::cout << "Render:  dot -Tpng " << dotFile << " -o "
                      << getOutputFilename(dotFile, ".png") << "\n";
        }

        // Grammar normalization report
        if (normalize) {
            std::cout << compiler.getNormalizedGrammar();
        }

        // Reports
        compiler.writeQuadReport(getOutputFilename(inputFile, "_quad.txt"));

        std::cout << "\nCompilation completed successfully.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
