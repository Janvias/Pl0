/**
 * @file compiler.cpp
 * @brief Compiler Implementation
 * @details Coordinates lexical analysis, dual-mode syntax analysis
 *          (LL(1) + LR(1)), and code generation for PL/0 programs.
 */

#include "../include/pl0_compiler.hpp"
#include <ctime>

namespace PL0 {

//============================================================================
// Compiler 实现
//============================================================================

Compiler::Compiler()
    : parserMode_(ParserMode::DUAL), verbose_(false), success_(false) {}

Compiler::~Compiler() {}

bool Compiler::compile(const std::string& inputFile) {
    inputFile_ = inputFile;
    success_ = false;
    validationResult_ = ValidationResult();

    // 初始化符号表和代码生成器
    symTable_ = std::make_unique<SymbolTable>();
    codeGen_ = std::make_unique<CodeGenerator>();

    // 结果变量
    bool ll1Success = false;
    std::string ll1Error;
    std::vector<Quadruple> ll1Quads;

    bool lr1Success = false;
    std::string lr1Error;
    std::vector<Quadruple> lr1Quads;

    // 根据模式执行语法分析
    bool runLL1 = (parserMode_ == ParserMode::LL1_ONLY ||
                   parserMode_ == ParserMode::DUAL);
    bool runLR1 = (parserMode_ == ParserMode::LR1_ONLY ||
                   parserMode_ == ParserMode::DUAL);

    std::vector<Token> tokens;

    // --- LL(1) 递归下降分析 ---
    if (runLL1) {
        if (verbose_) {
            std::cout << "Phase 1: LL(1) Syntax Analysis (Recursive Descent)...\n";
        }

        symTable_->clear();
        codeGen_->clear();

        auto ll1Lexer = std::make_unique<Lexer>(inputFile);
        if (ll1Lexer->hasError()) {
            ll1Success = false;
            ll1Error = ll1Lexer->getErrorMessage();
        } else {
            ll1Parser_ = std::make_unique<Parser>(
                ll1Lexer.get(), symTable_.get(), codeGen_.get());

            ll1Success = ll1Parser_->parse();
            if (!ll1Success) {
                ll1Error = ll1Parser_->getErrorMessage();
            }
            ll1Quads = codeGen_->getQuadruples();
        }

        if (verbose_) {
            std::cout << "LL(1) Parser: " << (ll1Success ? "SUCCESS" : "FAILED") << "\n";
            if (!ll1Success) {
                std::cout << "  Error: " << ll1Error << "\n";
            }
            std::cout << "  Quadruples: " << ll1Quads.size() << "\n";
        }
    }

    // --- Token预提取（LR(1)需要） ---
    if (runLR1) {
        if (verbose_) {
            std::cout << "\nPhase 2: Token extraction for LR(1)...\n";
        }

        lexer_ = std::make_unique<Lexer>(inputFile);
        if (lexer_->hasError()) {
            errorMessage_ = lexer_->getErrorMessage();
            if (!runLL1) return false;
        } else {
            // Read tokens; break on END_OF_FILE or consecutive ERROR tokens
            Token token;
            int errorStreak = 0;
            while (true) {
                token = lexer_->getNextToken();
                if (token.type == TokenType::END_OF_FILE) break;

                // Platform quirk: eof() may not trigger, causing garbage
                // ERROR tokens after real EOF. Detect and break.
                if (token.type == TokenType::ERROR) {
                    errorStreak++;
                    if (errorStreak >= 3) break;
                    continue; // skip garbage tokens
                }
                errorStreak = 0;
                tokens.push_back(token);
            }
            tokens.push_back(Token(TokenType::END_OF_FILE, "EOF", 0));
        }

        if (verbose_) {
            std::cout << "Tokens extracted: " << tokens.size() << "\n";
        }
    }

    // --- LR(1) 移进归约分析 ---
    if (runLR1 && !tokens.empty()) {
        if (verbose_) {
            std::cout << "\nPhase 3: LR(1) Syntax Analysis (Shift-Reduce)...\n";
            std::cout << "Building LR(1) canonical collection...\n";
        }

        lr1Parser_ = std::make_unique<LR1Parser>();

        if (verbose_) {
            std::cout << "LR(1) States: " << lr1Parser_->getStates().size()
                      << " computed.\n";
            std::cout << "Parsing with LR(1)...\n";
        }

        lr1Success = lr1Parser_->parse(tokens);
        if (!lr1Success) {
            lr1Error = lr1Parser_->getErrorMessage();
        }
        lr1Quads = lr1Parser_->getQuadruples();

        if (verbose_) {
            std::cout << "LR(1) Parser: " << (lr1Success ? "SUCCESS" : "FAILED") << "\n";
            if (!lr1Success) {
                std::cout << "  Error: " << lr1Error << "\n";
            }
            std::cout << "  Quadruples: " << lr1Quads.size() << "\n";
        }
    }

    // --- 结果对比与验证 ---
    if (runLL1 && runLR1 && !tokens.empty()) {
        validationResult_ = compareParserResults(
            ll1Success, ll1Error, ll1Quads,
            lr1Success, lr1Error, lr1Quads);

        if (verbose_) {
            printValidationResult(std::cout);
        }

        if (parserMode_ == ParserMode::DUAL) {
            // DUAL mode: success if BOTH parsers agree on parse outcome.
            // Quad differences are informational (different but equivalent
            // intermediate code generation strategies) — not an error.
            success_ = validationResult_.bothSucceeded;
            if (!success_) {
                std::ostringstream oss;
                oss << "Parser disagreement detected:\n";
                if (!ll1Success) oss << "  LL(1): " << ll1Error << "\n";
                if (!lr1Success) oss << "  LR(1): " << lr1Error << "\n";
                errorMessage_ = oss.str();
            }
            // Note: if both succeeded but quads differ, still report as
            // diagnostic info (stored in validationResult_) but treat as pass
        } else if (parserMode_ == ParserMode::LL1_ONLY) {
            success_ = ll1Success;
            if (!ll1Success) errorMessage_ = ll1Error;
        } else {
            success_ = lr1Success;
            if (!lr1Success) errorMessage_ = lr1Error;
        }
    } else if (runLL1) {
        success_ = ll1Success;
        if (!ll1Success) errorMessage_ = ll1Error;
    } else if (runLR1) {
        success_ = lr1Success;
        if (!lr1Success) errorMessage_ = lr1Error;
    }

    // 对于 LR-only 模式，使用 LR(1) quads
    if (parserMode_ == ParserMode::LR1_ONLY && !lr1Quads.empty()) {
        codeGen_->clear();
        for (const auto& q : lr1Quads) {
            codeGen_->emit(q.op, q.arg1, q.arg2, q.result);
        }
    }

    return success_;
}

void Compiler::printResults(std::ostream& os) {
    printHeader(os);

    if (lexer_) {
        lexer_->printTokens(os);
        lexer_->printStatistics(os);
        lexer_->printClassificationTable(os);
    }
    printStateTransitionDiagram(os);
    printRecognitionFlowchart(os);

    os << "\nLexical analysis completed.\n";

    os << "\n========================================\n";
    os << "Phase 2: Syntax and Semantic Analysis...\n";

    if (parserMode_ == ParserMode::DUAL) {
        os << "Parser Mode: DUAL (LL(1) + LR(1) Cross-Validation)\n";
    } else if (parserMode_ == ParserMode::LL1_ONLY) {
        os << "Parser Mode: LL(1) Recursive Descent Only\n";
    } else {
        os << "Parser Mode: LR(1) Shift-Reduce Only\n";
    }

    printValidationResult(os);

    os << "\n========================================\n";
    os << "Phase 3: Intermediate Code Generation...\n";
    codeGen_->print(os);
    symTable_->print(os);

    os << "\n========================================\n";
    os << "Compilation " << (success_ ? "completed successfully." : "failed.") << "\n";
    os << "========================================\n";
}

void Compiler::printValidationResult(std::ostream& os) {
    os << validationResult_.diagnosticInfo;
}

bool Compiler::writeLexReport(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create lexical report file: " << filename << "\n";
        return false;
    }

    file << "========================================\n";
    file << "       PL/0 Lexer Analysis Report\n";
    file << "========================================\n";
    file << "Source file: " << inputFile_ << "\n";
    file << "Generated: " << __DATE__ << " " << __TIME__ << "\n";
    file << "========================================\n";

    if (lexer_) {
        lexer_->printTokens(file);
        lexer_->printStatistics(file);
        lexer_->printClassificationTable(file);
    }
    printStateTransitionDiagram(file);
    printRecognitionFlowchart(file);

    file << "\n========================================\n";
    file << "Analysis completed.\n";
    file << "========================================\n";

    file.close();
    std::cout << "Lexical analysis report written to: " << filename << "\n";
    return true;
}

bool Compiler::writeQuadReport(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create quad report file: " << filename << "\n";
        return false;
    }

    file << "========================================\n";
    file << "       PL/0 Quadruple Report\n";
    file << "========================================\n";
    file << "Source file: " << inputFile_ << "\n";
    if (parserMode_ == ParserMode::DUAL) {
        file << "Parser Mode: DUAL (LL(1) + LR(1))\n";
    } else if (parserMode_ == ParserMode::LL1_ONLY) {
        file << "Parser Mode: LL(1) Only\n";
    } else {
        file << "Parser Mode: LR(1) Only\n";
    }
    file << "========================================\n";

    codeGen_->print(file);

    // In DUAL mode, also print LR(1) quads for comparison
    if (parserMode_ == ParserMode::DUAL && lr1Parser_) {
        file << "\n--- LR(1) Parser Quadruples (for comparison) ---\n";
        file << "Count: " << lr1Parser_->getQuadruples().size() << "\n";
        for (const auto& q : lr1Parser_->getQuadruples()) {
            file << "(" << q.index << ")("
                 << codeGen_->opCodeToString(q.op)
                 << "," << q.arg1 << "," << q.arg2 << "," << q.result << ")\n";
        }
    }

    file.close();
    std::cout << "Quadruples written to: " << filename << "\n";
    return true;
}

bool Compiler::writeCache(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create cache file: " << filename << "\n";
        return false;
    }

    file << "========================================\n";
    file << "       PL/0 Compiler Cache\n";
    file << "========================================\n";
    file << "Source file: " << inputFile_ << "\n";
    file << "Generated: " << __DATE__ << " " << __TIME__ << "\n";
    file << "========================================\n\n";

    file << validationResult_.diagnosticInfo;
    file << "\n";

    codeGen_->print(file);
    symTable_->print(file);

    // In DUAL mode, also print LR(1) parse table
    if (parserMode_ == ParserMode::DUAL && lr1Parser_) {
        file << "\n========================================\n";
        file << "    LR(1) Parser Diagnostic Information\n";
        file << "========================================\n";
        lr1Parser_->printStates(file);
    }

    file.close();
    std::cout << "Cache file written to: " << filename << "\n";
    return true;
}

void Compiler::printHeader(std::ostream& os) {
    os << "========================================\n";
    os << "         PL/0 Compiler (C++ Edition)\n";
    os << "========================================\n";
    os << "Source file: " << inputFile_ << "\n";
    os << "Generated: " << __DATE__ << " " << __TIME__ << "\n";
    os << "========================================\n\n";
}

void Compiler::printStateTransitionDiagram(std::ostream& os) {
    os << "\n===== STATE TRANSITION DIAGRAM =====\n";
    os << "Finite State Machine for PL/0 Lexical Analysis\n";
    os << "================================================\n\n";
    os << "States:\n";
    os << "  S0: Start state\n";
    os << "  S1: Identifier/Keyword (letter)\n";
    os << "  S2: Number (digit)\n";
    os << "  S3: Operator (+, -, *, /, =, #, <, >)\n";
    os << "  S4: Delimiter (;, ,, ., (, ))\n";
    os << "  S5: Two-char operator (<=, >=, :=)\n";
    os << "  S6: Comment (// or /*)\n";
    os << "  SF: Final state (token recognized)\n\n";
    os << "Transition Table:\n";
    os << "+-------+--------+--------+--------+--------+--------+--------+--------+\n";
    os << "| State | letter | digit  | op_char| del_char|  :     |  /     |  EOF   |\n";
    os << "+-------+--------+--------+--------+--------+--------+--------+--------+\n";
    os << "|   S0  |  S1    |  S2    |  S3    |  S4    |  S4    |  S3    |  SF    |\n";
    os << "|   S1  |  S1    |  S1    |  SF    |  SF    |  SF    |  SF    |  SF    |\n";
    os << "|   S2  | ERROR |  S2    |  SF    |  SF    |  SF    |  SF    |  SF    |\n";
    os << "|   S3  |  SF    |  SF    |  SF    |  SF    |  S5    |  S6    |  SF    |\n";
    os << "|   S4  |  SF    |  SF    |  SF    |  SF    |  S5    |  SF    |  SF    |\n";
    os << "|   S5  |  SF    |  SF    |  SF    |  SF    |  SF    |  SF    |  SF    |\n";
    os << "|   S6  |  S6    |  S6    |  S6    |  S6    |  S6    |  S6*   | ERROR |\n";
    os << "+-------+--------+--------+--------+--------+--------+--------+--------+\n";
    os << "Note: S6* transitions to SF when closing */ is found\n";
}

void Compiler::printRecognitionFlowchart(std::ostream& os) {
    os << "\n===== LEXICAL RECOGNITION FLOWCHART =====\n";
    os << "Token Recognition Process Flow\n";
    os << "===============================\n\n";
    os << "                          Start\n";
    os << "                            |\n";
    os << "                            v\n";
    os << "                  Read next character\n";
    os << "                            |\n";
    os << "                            v\n";
    os << "              +------------+------------+\n";
    os << "              |                         |\n";
    os << "              v                         v\n";
    os << "         [Whitespace?]              [EOF?]\n";
    os << "              |                         |\n";
    os << "         Yes  |  No              Yes  |  No\n";
    os << "              v  v                     v\n";
    os << "         Skip char              +------+------+\n";
    os << "              |                 |             |\n";
    os << "              +------+          v             v\n";
    os << "                     |    [Letter?]      [Digit?]\n";
    os << "                     |       |             |\n";
    os << "                     |  Yes  |  No    Yes  |  No\n";
    os << "                     |       v  v          v\n";
    os << "                     |  Identifier    Number\n";
    os << "                     |       |             |\n";
    os << "                     |       v             v\n";
    os << "                     |  [Keyword?]      +---+---+\n";
    os << "                     |       |           |       |\n";
    os << "                     |  Yes  |  No   [Valid?]  [Valid?]\n";
    os << "                     |       v  v       | Yes   | No\n";
    os << "                     |  Keyword  ID   +--+--+  +--+--+\n";
    os << "                     |                 |     |  |     |\n";
    os << "                     +--------+--------+     |  |     |\n";
    os << "                              |              |  |     |\n";
    os << "                              v              v  v     v\n";
    os << "                         +---+---+       Valid  ERROR\n";
    os << "                         |       |       Num    Num\n";
    os << "                         v       v\n";
    os << "                    [Operator?]  [Delimiter?]\n";
    os << "                         |       |\n";
    os << "                    Yes  |  No   v\n";
    os << "                         v  v  Process\n";
    os << "                    Process  ERROR\n";
    os << "                         |\n";
    os << "                         v\n";
    os << "                      Output Token\n";
    os << "                         |\n";
    os << "                         v\n";
    os << "                       Repeat\n";
}

} // namespace PL0
