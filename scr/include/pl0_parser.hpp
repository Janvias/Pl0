/**
 * @file pl0_parser.hpp
 * @brief PL/0编译器语法分析器类
 * @details 本类实现PL/0文法的递归下降解析，
 *          处理词法分析器产生的Token，执行语法分析，
 *          构建符号表，并生成中间代码
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

// 前向声明
class ASTBuilder;

/**
 * @class Parser
 * @brief PL/0文法递归下降解析器
 * @details 实现LL(1)递归下降解析，
 *          可选地通过注入的ASTBuilder构建AST用于可视化
 */
class Parser {
public:
    Parser(Lexer* lexer, SymbolTable* symTable, CodeGenerator* codeGen);
    ~Parser();

    // 解析
    bool parse();  // 执行语法分析

    // AST构建（可选 — 在调用parse()之前设置）
    void setASTBuilder(ASTBuilder* ast) { astBuilder_ = ast; }

    // 错误处理
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    Lexer* lexer_;           // 词法分析器
    SymbolTable* symTable_;  // 符号表
    CodeGenerator* codeGen_; // 代码生成器
    ASTBuilder* astBuilder_ = nullptr;  // AST构建器（可选）
    Token currentToken_;     // 当前Token
    bool hasError_;          // 是否有错误
    std::string errorMessage_;  // 错误消息

    // 递归下降函数
    void program();          // 程序
    void block();            // 分程序
    void constDeclaration(); // 常量声明
    void varDeclaration();   // 变量声明
    void procedureDeclaration();  // 过程声明
    void statement();        // 语句
    void condition();        // 条件
    void expression();       // 表达式
    void term();             // 项
    void factor();           // 因子

    // 辅助函数
    void match(TokenType expected);  // 匹配期望的Token类型
    void error(const std::string& message);  // 报告错误
    void sync();  // 错误同步
};

} // namespace PL0

#endif // PL0_PARSER_HPP
