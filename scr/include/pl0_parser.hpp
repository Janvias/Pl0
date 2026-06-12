/**
 * @file pl0_parser.hpp
 * @brief PL/0 Compiler Parser Class
 * @details This class implements recursive descent parsing for PL/0 grammar.
 *          It processes tokens from the lexer, performs syntax analysis,
 *          builds the symbol table, and generates intermediate code.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_PARSER_HPP
#define PL0_PARSER_HPP

#include "pl0_core.hpp"
#include "pl0_lexer.hpp"
#include "pl0_symtable.hpp"
#include "pl0_codegen.hpp"

namespace PL0 {

// Forward declaration
class ASTBuilder;

/**
 * @class Parser
 * @brief Recursive descent parser for PL/0 grammar
 * @details Implements LL(1) recursive descent parsing. Optionally builds
 *          an AST via an injected ASTBuilder for visualization.
 */
class Parser {
public:
    Parser(Lexer* lexer, SymbolTable* symTable, CodeGenerator* codeGen);
    ~Parser();

    // Parse
    bool parse();

    // AST building (optional — set before calling parse())
    void setASTBuilder(ASTBuilder* ast) { astBuilder_ = ast; }

    // Error handling
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    Lexer* lexer_;
    SymbolTable* symTable_;
    CodeGenerator* codeGen_;
    ASTBuilder* astBuilder_ = nullptr;
    Token currentToken_;
    bool hasError_;
    std::string errorMessage_;

    // Recursive descent functions
    void program();
    void block();
    void constDeclaration();
    void varDeclaration();
    void procedureDeclaration();
    void statement();
    void condition();
    void expression();
    void term();
    void factor();

    // Helpers
    void match(TokenType expected);
    void error(const std::string& message);
    void sync();
};

} // namespace PL0

#endif // PL0_PARSER_HPP
