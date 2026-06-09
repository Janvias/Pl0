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

/**
 * @class Parser
 * @brief Recursive descent parser for PL/0 grammar
 * @details The Parser class implements the PL/0 grammar using recursive descent.
 *          It coordinates with the Lexer, SymbolTable, and CodeGenerator
 *          to perform syntax and semantic analysis.
 */
class Parser {
public:
    Parser(Lexer* lexer, SymbolTable* symTable, CodeGenerator* codeGen);
    ~Parser();

    // 语法分析
    bool parse();

    // 错误处理
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    Lexer* lexer_;
    SymbolTable* symTable_;
    CodeGenerator* codeGen_;
    Token currentToken_;
    bool hasError_;
    std::string errorMessage_;

    // 递归下降分析函数
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

    // 辅助函数
    void match(TokenType expected);
    void error(const std::string& message);
    
    // 错误恢复
    void sync();
};

} // namespace PL0

#endif // PL0_PARSER_HPP
