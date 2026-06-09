/**
 * @file compiler.cpp
 * @brief Compiler Implementation
 */

#include "../include/pl0_compiler.hpp"

namespace PL0 {

//============================================================================
// Compiler 实现
//============================================================================

Compiler::Compiler() : verbose_(false), success_(false) {}

Compiler::~Compiler() {}

bool Compiler::compile(const std::string& inputFile) {
    inputFile_ = inputFile;
    success_ = false;

    // 初始化组件
    lexer_ = std::make_unique<Lexer>(inputFile);
    if (lexer_->hasError()) {
        errorMessage_ = lexer_->getErrorMessage();
        return false;
    }

    symTable_ = std::make_unique<SymbolTable>();
    codeGen_ = std::make_unique<CodeGenerator>();
    parser_ = std::make_unique<Parser>(lexer_.get(), symTable_.get(), codeGen_.get());

    // 词法分析
    if (verbose_) {
        std::cout << "Phase 1: Lexical Analysis...\n";
    }

    // 语法和语义分析
    if (verbose_) {
        std::cout << "\nPhase 2: Syntax and Semantic Analysis...\n";
    }

    success_ = parser_->parse();
    if (!success_) {
        errorMessage_ = parser_->getErrorMessage();
    }

    return success_;
}

void Compiler::printHeader(std::ostream& os) {
    os << "========================================\n";
    os << "       PL/0 Compiler v3.0 (C++)\n";
    os << "========================================\n";
    os << "Source file: " << inputFile_ << "\n\n";
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
    os << "  S4: Delimiter (;, ,, ., (, ), :)\n";
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

void Compiler::printResults(std::ostream& os) {
    printHeader(os);

    lexer_->printTokens(os);
    lexer_->printStatistics(os);
    lexer_->printClassificationTable(os);
    printStateTransitionDiagram(os);
    printRecognitionFlowchart(os);

    os << "\nLexical analysis completed.\n";

    os << "\n========================================\n";
    os << "Phase 3: Intermediate Code Generation...\n";
    codeGen_->print(os);
    symTable_->print(os);

    os << "\n========================================\n";
    os << "Compilation " << (success_ ? "completed successfully." : "failed.") << "\n";
    os << "========================================\n";
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

    lexer_->printTokens(file);
    lexer_->printStatistics(file);
    lexer_->printClassificationTable(file);
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

    codeGen_->print(file);

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

    codeGen_->print(file);
    symTable_->print(file);

    file.close();
    std::cout << "Cache file written to: " << filename << "\n";
    return true;
}

} // namespace PL0
