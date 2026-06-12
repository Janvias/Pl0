/**
 * @file pl0_compiler.hpp
 * @brief PL/0 Compiler Main Class
 * @details Coordinates lexical analysis, dual-mode syntax analysis
 *          (LL(1) + LR(1)), semantic analysis, and code generation.
 *          Supports AST visualization, error suggestions, and grammar
 *          normalization as bonus features.
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_COMPILER_HPP
#define PL0_COMPILER_HPP

#include "pl0_core.hpp"
#include "pl0_lexer.hpp"
#include "pl0_symtable.hpp"
#include "pl0_codegen.hpp"
#include "pl0_parser.hpp"
#include "pl0_lr1_parser.hpp"
#include "pl0_ast.hpp"
#include "pl0_error_suggestor.hpp"
#include "pl0_grammar_normalizer.hpp"

namespace PL0 {

class Compiler {
public:
    Compiler();
    ~Compiler();

    // --- Compilation ---
    bool compile(const std::string& inputFile);

    // --- Parser mode ---
    void setParserMode(ParserMode mode) { parserMode_ = mode; }
    ParserMode getParserMode() const { return parserMode_; }
    const ValidationResult& getValidationResult() const { return validationResult_; }

    // --- Bonus features ---
    void enableAST(bool enable)     { astEnabled_ = enable; }
    void enableSuggest(bool enable) { suggestEnabled_ = enable; }
    void enableNormalize(bool enable) { normalizeEnabled_ = enable; }

    bool isASTEnabled()     const { return astEnabled_; }
    bool isSuggestEnabled() const { return suggestEnabled_; }
    bool isNormalizeEnabled() const { return normalizeEnabled_; }

    // AST output
    const ASTBuilder* getAST() const { return astBuilder_.get(); }
    std::string getASTDOT() const;

    // Grammar normalization (operates on the 46 LR(1) productions)
    std::string getNormalizedGrammar() const;

    // Error suggestions for the last error
    std::string getErrorSuggestions() const;

    // --- Output ---
    void setVerbose(bool verbose) { verbose_ = verbose; }
    void printResults(std::ostream& os = std::cout);

    // File output
    bool writeLexReport(const std::string& filename);
    bool writeQuadReport(const std::string& filename);
    bool writeCache(const std::string& filename);
    bool writeASTDOT(const std::string& filename);

    // Status
    bool isSuccess() const { return success_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    std::string inputFile_;
    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<SymbolTable> symTable_;
    std::unique_ptr<CodeGenerator> codeGen_;
    std::unique_ptr<Parser> ll1Parser_;
    std::unique_ptr<LR1Parser> lr1Parser_;

    // Bonus feature components
    std::unique_ptr<ASTBuilder> astBuilder_;
    ErrorSuggestor errorSuggestor_;
    GrammarNormalizer grammarNormalizer_;

    ParserMode parserMode_;
    bool verbose_;
    bool success_;
    std::string errorMessage_;
    ValidationResult validationResult_;

    // Feature toggles
    bool astEnabled_;
    bool suggestEnabled_;
    bool normalizeEnabled_;

    // Private methods
    void printHeader(std::ostream& os);
    void printStateTransitionDiagram(std::ostream& os);
    void printRecognitionFlowchart(std::ostream& os);
    void printValidationResult(std::ostream& os);
    void printErrorSuggestions(std::ostream& os);
};

} // namespace PL0

#endif // PL0_COMPILER_HPP
