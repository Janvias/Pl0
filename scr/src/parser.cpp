/**
 * @file parser.cpp
 * @brief Parser Implementation
 */

#include "../include/pl0_parser.hpp"
#include <stdexcept>

namespace PL0 {

//============================================================================
// Parser 实现
//============================================================================

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
    symTable_->enterScope("main");
    codeGen_->emit(OpCode::SYSS);

    block();

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ".") {
        error("Expected '.' at end of program");
    }

    codeGen_->emit(OpCode::SYSC);
    symTable_->exitScope();
}

void Parser::block() {
    symTable_->enterScope("block");

    // 声明部分
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

    // 语句部分
    statement();

    symTable_->exitScope();
}

void Parser::constDeclaration() {
    match(TokenType::KEYWORD); // const
    do {
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected identifier in const declaration");
            break;
        }
        std::string name = currentToken_.value;
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
}

void Parser::varDeclaration() {
    match(TokenType::KEYWORD); // var

    // 解析第一个变量
    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("Expected identifier in var declaration");
        return;
    }

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
            return;
        }

        sym.name = currentToken_.value;
        sym.address = symTable_->getCurrentLevel();
        symTable_->addSymbol(sym);

        match(TokenType::IDENTIFIER);
    }

    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("Expected ';' in var declaration");
        return;
    }
    match(TokenType::DELIMITER);
}

void Parser::procedureDeclaration() {
    match(TokenType::KEYWORD); // procedure

    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("Expected procedure name");
        return;
    }

    std::string name = currentToken_.value;
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
}

void Parser::statement() {
    // 处理 begin...end 块
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "begin") {
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
        return;
    }

    // 处理赋值语句
    if (currentToken_.type == TokenType::IDENTIFIER) {
        std::string name = currentToken_.value;
        match(TokenType::IDENTIFIER);

        if (currentToken_.type != TokenType::OPERATOR || currentToken_.value != ":=") {
            error("Expected ':=' in assignment");
            return;
        }
        match(TokenType::OPERATOR);

        expression();
        codeGen_->emit(OpCode::ASSIGN, "", "", name);
        return;
    }

    // 处理 if 语句
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "if") {
        match(TokenType::KEYWORD);
        condition();

        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "then") {
            error("Expected 'then'");
            return;
        }
        match(TokenType::KEYWORD);
        statement();
        return;
    }

    // 处理 while 语句
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "while") {
        match(TokenType::KEYWORD);
        std::string startLabel = "L" + std::to_string(codeGen_->getQuadruples().size());
        condition();

        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "do") {
            error("Expected 'do'");
            return;
        }
        match(TokenType::KEYWORD);
        statement();

        codeGen_->emit(OpCode::JUMP, startLabel);
        return;
    }

    // 处理 call 语句
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "call") {
        match(TokenType::KEYWORD);
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected procedure name after 'call'");
            return;
        }
        std::string name = currentToken_.value;
        codeGen_->emit(OpCode::CALL, name);
        match(TokenType::IDENTIFIER);
        return;
    }

    // 处理 read 语句
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "read") {
        match(TokenType::KEYWORD);
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != "(") {
            error("Expected '(' after 'read'");
            return;
        }
        match(TokenType::DELIMITER);
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("Expected identifier in read statement");
            return;
        }
        std::string name = currentToken_.value;
        codeGen_->emit(OpCode::READ, "", "", name);
        match(TokenType::IDENTIFIER);
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("Expected ')' after read argument");
            return;
        }
        match(TokenType::DELIMITER);
        return;
    }

    // 处理 write 语句
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "write") {
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
            return;
        }
        match(TokenType::DELIMITER);
        return;
    }
}

void Parser::condition() {
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "odd") {
        match(TokenType::KEYWORD);
        expression();
        codeGen_->emit(OpCode::ODD);
    } else {
        expression();
        std::string op = currentToken_.value;
        if (currentToken_.type != TokenType::OPERATOR) {
            error("Expected comparison operator");
            return;
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
            return;
        }

        match(TokenType::OPERATOR);
        expression();
        codeGen_->emit(opCode);
    }
}

void Parser::expression() {
    std::string op;
    if (currentToken_.type == TokenType::OPERATOR &&
        (currentToken_.value == "+" || currentToken_.value == "-")) {
        op = currentToken_.value;
        match(TokenType::OPERATOR);
    }

    term();

    if (!op.empty() && op == "-") {
        codeGen_->emit(OpCode::SUB, "0", "", "");
    }

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

void Parser::term() {
    factor();

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

void Parser::factor() {
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
    } else if (currentToken_.type == TokenType::NUMBER) {
        int value = std::stoi(currentToken_.value);
        codeGen_->emit(OpCode::ADD, std::to_string(value), "", "");
        match(TokenType::NUMBER);
    } else if (currentToken_.type == TokenType::DELIMITER && currentToken_.value == "(") {
        match(TokenType::DELIMITER);
        expression();
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
