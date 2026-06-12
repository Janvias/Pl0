/**
 * @file parser.cpp
 * @brief Parser Implementation
 * @details This file implements a recursive descent parser for the PL/0 compiler.
 *          The parser uses LL(1) grammar with single lookahead token to determine
 *          production rules without backtracking.
 * 
 * LL(1) Grammar Characteristics:
 * - Single lookahead token (currentToken_) determines production selection
 * - No left recursion (eliminated by right-recursion transformation)
 * - FIRST sets are disjoint for alternative productions
 * - FOLLOW sets properly handled for error detection
 */

#include "../include/pl0_parser.hpp"
#include "../include/pl0_ast.hpp"
#include <stdexcept>

#define AST_BEGIN(t, v) if (astBuilder_) astBuilder_->beginNode(t, v)
#define AST_END()       if (astBuilder_) astBuilder_->endNode()
#define AST_LEAF(t, v)  if (astBuilder_) astBuilder_->addLeaf(t, v)

namespace PL0 {

//============================================================================
// Parser 实现 - 递归下降分析法 (Recursive Descent Parsing)
//============================================================================
// 递归下降分析法是一种自顶向下的语法分析方法，核心特点：
// 1. 每个非终结符对应一个函数
// 2. 通过单一向前看符号(lookahead=1)决定推导方向
// 3. 无需回溯，时间复杂度为O(n)
// 4. 要求文法为LL(1)文法

Parser::Parser(Lexer* lexer, SymbolTable* symTable, CodeGenerator* codeGen)
    : lexer_(lexer), symTable_(symTable), codeGen_(codeGen),
      hasError_(false) {
    currentToken_ = lexer_->getNextToken();
}

Parser::~Parser() {}

bool Parser::parse() {
    try {
        program();
        return !hasError_;
    } catch (const std::exception& e) {
        error(e.what());
        return false;
    }
}

void Parser::program() {
    AST_BEGIN(ASTNodeType::PROGRAM, "");
    symTable_->enterScope("main");
    codeGen_->emit(OpCode::SYSS);

    block();

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ".") {
        error("Expected '.' at end of program");
    }

    codeGen_->emit(OpCode::SYSC);
    symTable_->exitScope();
    AST_END();
}

void Parser::block() {
    AST_BEGIN(ASTNodeType::BLOCK, "");
    symTable_->enterScope("block");

    while (currentToken_.type == TokenType::KEYWORD) {
        if (currentToken_.value == "const") {
            constDeclaration();
        } else if (currentToken_.value == "var") {
            varDeclaration();
        } else if (currentToken_.value == "procedure") {
            procedureDeclaration();
        } else {
            break;
        }
    }

    statement();
    symTable_->exitScope();
    AST_END();
}

void Parser::constDeclaration() {
    AST_BEGIN(ASTNodeType::CONST_DECL, "");
    match(TokenType::KEYWORD); // const
    do {
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected identifier in const declaration");
            break;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        match(TokenType::IDENTIFIER);

        if (currentToken_.type != TokenType::OPERATOR || currentToken_.value != "=") {
            error("Expected '=' in const declaration");
            break;
        }
        match(TokenType::OPERATOR);

        if (currentToken_.type != TokenType::NUMBER) {
            error("Expected number in const declaration");
            break;
        }
        int value = std::stoi(currentToken_.value);

        Symbol sym;
        sym.name = name;
        sym.kind = SymbolKind::CONSTANT;
        sym.value = value;
        sym.level = symTable_->getCurrentLevel();
        symTable_->addSymbol(sym);

        match(TokenType::NUMBER);
    } while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ",");

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("Expected ';' in const declaration");
    }
    match(TokenType::DELIMITER);
    AST_END();
}

void Parser::varDeclaration() {
    AST_BEGIN(ASTNodeType::VAR_DECL, "");
    match(TokenType::KEYWORD); // var

    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("Expected identifier in var declaration");
        AST_END();
        return;
    }

    AST_LEAF(ASTNodeType::IDENTIFIER, currentToken_.value);
    Symbol sym;
    sym.name = currentToken_.value;
    sym.kind = SymbolKind::VARIABLE;
    sym.level = symTable_->getCurrentLevel();
    sym.address = symTable_->getCurrentLevel();
    symTable_->addSymbol(sym);

    match(TokenType::IDENTIFIER);

    // 处理逗号分隔的变量列表
    while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ",") {
        match(TokenType::DELIMITER);

        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected identifier after ','");
            AST_END();
            return;
        }

        AST_LEAF(ASTNodeType::IDENTIFIER, currentToken_.value);
        sym.name = currentToken_.value;
        sym.address = symTable_->getCurrentLevel();
        symTable_->addSymbol(sym);

        match(TokenType::IDENTIFIER);
    }

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("Expected ';' in var declaration");
        AST_END();
        return;
    }
    match(TokenType::DELIMITER);
    AST_END();
}

void Parser::procedureDeclaration() {
    AST_BEGIN(ASTNodeType::PROC_DECL, "");
    match(TokenType::KEYWORD); // procedure

    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("Expected procedure name");
        AST_END();
        return;
    }

    std::string name = currentToken_.value;
    AST_LEAF(ASTNodeType::IDENTIFIER, name);
    Symbol sym;
    sym.name = name;
    sym.kind = SymbolKind::PROCEDURE;
    sym.level = symTable_->getCurrentLevel();
    sym.address = codeGen_->getQuadruples().size() + 100;
    symTable_->addSymbol(sym);

    match(TokenType::IDENTIFIER);

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("Expected ';' after procedure name");
    }
    match(TokenType::DELIMITER);

    block();

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("Expected ';' after procedure body");
    }
    match(TokenType::DELIMITER);

    codeGen_->emit(OpCode::RET);
    AST_END();
}

/**
 * @brief 语句解析 - LL(1)分支选择示例
 * 
 * LL(1)文法确保机制：通过检查当前token（lookahead=1）唯一确定产生式
 * 
 * statement 的 FIRST 集：{begin, id, if, while, call, read, write}
 * 各分支的 FIRST 集互不相交，确保无回溯选择：
 * - FIRST(begin ... end) = {begin}
 * - FIRST(id := expr) = {id}
 * - FIRST(if ...) = {if}
 * - FIRST(while ...) = {while}
 * - FIRST(call ...) = {call}
 * - FIRST(read ...) = {read}
 * - FIRST(write ...) = {write}
 */
void Parser::statement() {
    // 处理 begin...end 块
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "begin") {
        AST_BEGIN(ASTNodeType::BEGIN_END, "");
        match(TokenType::KEYWORD);
        statement();
        while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ";") {
            match(TokenType::DELIMITER);
            if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "end") {
                break;
            }
            statement();
        }
        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "end") {
            error("Expected 'end'");
        }
        match(TokenType::KEYWORD);
        AST_END();
        return;
    }

    // Handle assignment statement - FIRST(id) = {id}
    if (currentToken_.type == TokenType::IDENTIFIER) {
        std::string name = currentToken_.value;
        AST_BEGIN(ASTNodeType::ASSIGN_STMT, name);
        match(TokenType::IDENTIFIER);

        if (currentToken_.type != TokenType::OPERATOR || currentToken_.value != ":=") {
            error("Expected ':=' in assignment");
            return;
        }
        match(TokenType::OPERATOR);

        expression();
        codeGen_->emit(OpCode::ASSIGN, "", "", name);
        AST_END();
        return;
    }

    // Handle if statement - FIRST(if) = {if}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "if") {
        AST_BEGIN(ASTNodeType::IF_STMT, "");
        match(TokenType::KEYWORD);
        condition();

        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "then") {
            error("Expected 'then'");
            AST_END();
            return;
        }
        match(TokenType::KEYWORD);
        statement();
        AST_END();
        return;
    }

    // Handle while statement - FIRST(while) = {while}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "while") {
        AST_BEGIN(ASTNodeType::WHILE_STMT, "");
        match(TokenType::KEYWORD);
        std::string startLabel = "L" + std::to_string(codeGen_->getQuadruples().size());
        condition();

        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "do") {
            error("Expected 'do'");
            AST_END();
            return;
        }
        match(TokenType::KEYWORD);
        statement();
        codeGen_->emit(OpCode::JUMP, startLabel);
        AST_END();
        return;
    }

    // Handle call statement - FIRST(call) = {call}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "call") {
        AST_BEGIN(ASTNodeType::CALL_STMT, "");
        match(TokenType::KEYWORD);
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected procedure name after 'call'");
            AST_END(); return;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        codeGen_->emit(OpCode::CALL, name);
        match(TokenType::IDENTIFIER);
        AST_END();
        return;
    }

    // 处理 read 语句 - FIRST(read) = {read}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "read") {
        AST_BEGIN(ASTNodeType::READ_STMT, "");
        match(TokenType::KEYWORD);
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != "(") {
            error("Expected '(' after 'read'"); AST_END(); return;
        }
        match(TokenType::DELIMITER);
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected identifier in read statement"); AST_END(); return;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        codeGen_->emit(OpCode::READ, "", "", name);
        match(TokenType::IDENTIFIER);
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("Expected ')' after read argument"); AST_END(); return;
        }
        match(TokenType::DELIMITER);
        AST_END();
        return;
    }

    // 处理 write 语句 - FIRST(write) = {write}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "write") {
        AST_BEGIN(ASTNodeType::WRITE_STMT, "");
        match(TokenType::KEYWORD);
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != "(") {
            error("Expected '(' after 'write'");
            return;
        }
        match(TokenType::DELIMITER);
        expression();
        codeGen_->emit(OpCode::WRITE);
        while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ",") {
            match(TokenType::DELIMITER);
            expression();
            codeGen_->emit(OpCode::WRITE);
        }
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("Expected ')' after write arguments");
            AST_END(); return;
        }
        match(TokenType::DELIMITER);
        AST_END();
        return;
    }
}

void Parser::condition() {
    AST_BEGIN(ASTNodeType::CONDITION, "");
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "odd") {
        AST_LEAF(ASTNodeType::BINOP, "odd");
        match(TokenType::KEYWORD);
        expression();
        codeGen_->emit(OpCode::ODD);
    } else {
        expression();
        std::string op = currentToken_.value;
        if (currentToken_.type != TokenType::OPERATOR) {
            error("Expected comparison operator");
            AST_END(); return;
        }

        OpCode opCode;
        if (op == "=") opCode = OpCode::EQ;
        else if (op == "#") opCode = OpCode::NEQ;
        else if (op == "<") opCode = OpCode::LT;
        else if (op == "<=") opCode = OpCode::LTE;
        else if (op == ">") opCode = OpCode::GT;
        else if (op == ">=") opCode = OpCode::GTE;
        else {
            error("Unknown comparison operator");
            AST_END(); return;
        }

        AST_LEAF(ASTNodeType::BINOP, op);
        match(TokenType::OPERATOR);
        expression();
        codeGen_->emit(opCode);
    }
    AST_END();
}

/**
 * @brief 表达式解析 - 左递归消除示例
 * 
 * LL(1)文法要求：必须消除左递归
 * 
 * 原始左递归文法：
 *   expression → expression + term | expression - term | term
 * 
 * 右递归改写（消除左递归后）：
 *   expression → term { + term | - term }
 * 
 * 实现方式：先解析第一个term，然后用循环处理后续的二元运算符
 * 
 * FIRST(expression) = {+, -, id, num, (}
 */
void Parser::expression() {
    std::string op;
    if (currentToken_.type == TokenType::OPERATOR &&
        (currentToken_.value == "+" || currentToken_.value == "-")) {
        op = currentToken_.value;
        match(TokenType::OPERATOR);
    }

    // 先解析第一个 term（非递归开头）
    term();

    if (!op.empty() && op == "-") {
        codeGen_->emit(OpCode::SUB, "0", "", "");
    }

    // 循环处理后续的二元运算符（右递归结构）
    while (currentToken_.type == TokenType::OPERATOR &&
           (currentToken_.value == "+" || currentToken_.value == "-")) {
        op = currentToken_.value;
        match(TokenType::OPERATOR);
        term();
        if (op == "+") {
            codeGen_->emit(OpCode::ADD);
        } else {
            codeGen_->emit(OpCode::SUB);
        }
    }
}

/**
 * @brief 项解析 - 左递归消除示例
 * 
 * 原始左递归文法：
 *   term → term * factor | term / factor | factor
 * 
 * 右递归改写：
 *   term → factor { * factor | / factor }
 * 
 * FIRST(term) = {id, num, (}
 */
void Parser::term() {
    // 先解析第一个 factor（非递归开头）
    factor();

    // 循环处理后续的乘除运算符（右递归结构）
    while (currentToken_.type == TokenType::OPERATOR &&
           (currentToken_.value == "*" || currentToken_.value == "/")) {
        std::string op = currentToken_.value;
        match(TokenType::OPERATOR);
        factor();
        if (op == "*") {
            codeGen_->emit(OpCode::MUL);
        } else {
            codeGen_->emit(OpCode::DIV);
        }
    }
}

/**
 * @brief 因子解析 - FIRST集分支选择示例
 * 
 * factor 产生式：
 *   factor → id | num | ( expression )
 * 
 * FIRST(factor) = {id, num, (}
 * 
 * 各分支的 FIRST 集互不相交：
 * - FIRST(id) = {id}
 * - FIRST(num) = {num}
 * - FIRST((expression)) = {(}
 * 
 * 通过检查当前 token 类型即可唯一确定选择哪个产生式
 */
void Parser::factor() {
    // FIRST(id) = {id}
    if (currentToken_.type == TokenType::IDENTIFIER) {
        std::string name = currentToken_.value;
        Symbol* sym = symTable_->lookup(name);
        if (!sym) {
            error("Undefined identifier: " + name);
        }
        match(TokenType::IDENTIFIER);
        if (sym && sym->kind == SymbolKind::CONSTANT) {
            codeGen_->emit(OpCode::ADD, std::to_string(sym->value), "", "");
        }
    // FIRST(num) = {num}
    } else if (currentToken_.type == TokenType::NUMBER) {
        int value = std::stoi(currentToken_.value);
        codeGen_->emit(OpCode::ADD, std::to_string(value), "", "");
        match(TokenType::NUMBER);
    // FIRST((expression)) = {(}
    } else if (currentToken_.type == TokenType::DELIMITER && currentToken_.value == "(") {
        match(TokenType::DELIMITER);
        expression();
        // FOLLOW 集检查：期望 ')'
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("Expected ')'");
        }
        match(TokenType::DELIMITER);
    } else {
        error("Unexpected token in expression");
    }
}

void Parser::match(TokenType expected) {
    if (currentToken_.type == expected) {
        currentToken_ = lexer_->getNextToken();
    }
}

void Parser::error(const std::string& message) {
    hasError_ = true;
    errorMessage_ = "(Syntax Error, Line: " + std::to_string(currentToken_.line) +
                    ") " + message;
    std::cerr << errorMessage_ << std::endl;
    sync();
}

void Parser::sync() {
    while (currentToken_.type != TokenType::DELIMITER && 
           currentToken_.type != TokenType::END_OF_FILE &&
           currentToken_.type != TokenType::KEYWORD) {
        currentToken_ = lexer_->getNextToken();
    }
}

} // namespace PL0
