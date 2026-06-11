/**
 * @file main.cpp
 * @brief PL/0 Compiler C++ Main Entry Point
 * @description 现代C++实现的PL/0编译器主程序
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 * @version 3.0 (C++ Edition)
 */

#include <iostream>
#include <string>
#include <memory>
#include "include/pl0_compiler.hpp"

using namespace PL0;

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <source_file> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --verbose    Enable verbose output\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  -m, --mode MODE  Parser mode: ll1, lr1, dual (default: dual)\n";
    std::cout << "  --ll1            Use LL(1) parser only\n";
    std::cout << "  --lr1            Use LR(1) parser only\n";
    std::cout << "  --dual           Use both parsers with validation (default)\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " test.pl0\n";
    std::cout << "  " << programName << " test.pl0 -v --ll1\n";
    std::cout << "  " << programName << " test.pl0 -m lr1\n";
}

std::string getOutputFilename(const std::string& input, const std::string& suffix) {
    size_t dotPos = input.rfind('.');
    if (dotPos != std::string::npos) {
        return input.substr(0, dotPos) + suffix;
    }
    return input + suffix;
}

int main(int argc, char* argv[]) {
    // 参数解析
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile;
    bool verbose = false;
    ParserMode mode = ParserMode::DUAL;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--ll1") {
            mode = ParserMode::LL1_ONLY;
        } else if (arg == "--lr1") {
            mode = ParserMode::LR1_ONLY;
        } else if (arg == "--dual") {
            mode = ParserMode::DUAL;
        } else if (arg == "-m" || arg == "--mode") {
            if (i + 1 < argc) {
                std::string modeStr = argv[++i];
                if (modeStr == "ll1") mode = ParserMode::LL1_ONLY;
                else if (modeStr == "lr1") mode = ParserMode::LR1_ONLY;
                else if (modeStr == "dual") mode = ParserMode::DUAL;
                else {
                    std::cerr << "Unknown mode: " << modeStr << "\n";
                    printUsage(argv[0]);
                    return 1;
                }
            } else {
                std::cerr << "Missing mode argument for -m\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg[0] != '-') {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }

    // 创建并运行编译器
    try {
        Compiler compiler;
        compiler.setVerbose(verbose);
        compiler.setParserMode(mode);

        if (!compiler.compile(inputFile)) {
            std::cerr << "Compilation failed: " << compiler.getErrorMessage() << "\n";
            return 1;
        }

        // 输出结果
        compiler.printResults();

        // 生成报告文件
        compiler.writeLexReport(getOutputFilename(inputFile, "_lex.txt"));
        compiler.writeQuadReport(getOutputFilename(inputFile, "_quad.txt"));
        compiler.writeCache(getOutputFilename(inputFile, "_cache.txt"));

        std::cout << "\n========================================\n";
        std::cout << "Compilation completed successfully.\n";
        std::cout << "========================================\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
