/**
 * @file pl0_compiler.hpp
 * @brief PL/0 Compiler Main Class
 * @details This class serves as the main coordinator for the PL/0 compiler,
 *          integrating the lexer, parser, symbol table, and code generator.
 *          It manages the compilation process from source file to output.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_COMPILER_HPP
#define PL0_COMPILER_HPP

#include "pl0_core.hpp"
#include "pl0_lexer.hpp"
#include "pl0_symtable.hpp"
#include "pl0_codegen.hpp"
#include "pl0_parser.hpp"
#include "pl0_lr1_parser.hpp"

namespace PL0 {

/**
 * @class Compiler
 * @brief Main compiler class that coordinates all compilation phases
 * @details The Compiler class orchestrates lexical analysis, syntax analysis,
 *          semantic analysis, and code generation for PL/0 source programs.
 *          Supports dual-mode parsing: LL(1) only, LR(1) only, or both for
 *          cross-validation.
 */
class Compiler {
public:
    Compiler();
    ~Compiler();

    // 编译流程
    bool compile(const std::string& inputFile);

    // 解析模式配置
    void setParserMode(ParserMode mode) { parserMode_ = mode; }
    ParserMode getParserMode() const { return parserMode_; }
    const ValidationResult& getValidationResult() const { return validationResult_; }

    // 输出控制
    void setVerbose(bool verbose) { verbose_ = verbose; }
    void printResults(std::ostream& os = std::cout);

    // 文件输出
    bool writeLexReport(const std::string& filename);
    bool writeQuadReport(const std::string& filename);
    bool writeCache(const std::string& filename);

    // 状态查询
    bool isSuccess() const { return success_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    std::string inputFile_;
    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<SymbolTable> symTable_;
    std::unique_ptr<CodeGenerator> codeGen_;
    std::unique_ptr<Parser> ll1Parser_;
    std::unique_ptr<LR1Parser> lr1Parser_;

    ParserMode parserMode_;
    bool verbose_;
    bool success_;
    std::string errorMessage_;
    ValidationResult validationResult_;

    // 私有方法
    void printHeader(std::ostream& os);
    void printStateTransitionDiagram(std::ostream& os);
    void printRecognitionFlowchart(std::ostream& os);
    void printValidationResult(std::ostream& os);
};

} // namespace PL0

#endif // PL0_COMPILER_HPP
