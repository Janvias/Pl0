/**
 * @file parser.cpp
 * @brief PL/0 编译器语法分析器实现
 * @details 本文件实现了 PL/0 编译器的递归下降语法分析器。
 *          该解析器采用 LL(1) 文法，使用单一向前看符号（lookahead=1）来确定产生式规则，无需回溯。
 * 
 * LL(1) 文法特性：
 * - 单一向前看符号（currentToken_）决定产生式选择
 * - 无左递归（通过右递归转换消除）
 * - 可选产生式的 FIRST 集互不相交
 * - FOLLOW 集正确处理，用于错误检测
 */

#include "../include/pl0_parser.hpp"
#include "../include/pl0_ast.hpp"
#include <stdexcept>

// AST 构建宏定义 - 条件性调用 AST 构建器
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

/**
 * @brief 构造函数 - 初始化语法分析器
 * @param lexer 词法分析器指针，用于获取 token
 * @param symTable 符号表指针，用于存储和查询符号
 * @param codeGen 代码生成器指针，用于生成四元式
 */
Parser::Parser(Lexer* lexer, SymbolTable* symTable, CodeGenerator* codeGen)
    : lexer_(lexer), symTable_(symTable), codeGen_(codeGen),
      hasError_(false) {
    // 获取第一个 token，启动词法分析
    currentToken_ = lexer_->getNextToken();
}

/**
 * @brief 析构函数 - 清理资源
 */
Parser::~Parser() {}

/**
 * @brief 开始解析入口函数
 * @return 解析是否成功（true 表示成功，false 表示有错误）
 */
bool Parser::parse() {
    try {
        // 从 program 开始解析（文法的起始符号）
        program();
        return !hasError_;
    } catch (const std::exception& e) {
        // 捕获异常并报告错误
        error(e.what());
        return false;
    }
}

/**
 * @brief 解析程序（program）
 * @details 产生式：program → block .
 *          程序入口，负责初始化主作用域和系统调用
 */
void Parser::program() {
    AST_BEGIN(ASTNodeType::PROGRAM, "");
    
    // 进入主程序作用域
    symTable_->enterScope("main");
    // 生成系统启动指令
    codeGen_->emit(OpCode::SYSS);

    // 解析程序体（块）
    block();

    // 检查程序结束符 '.'
    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ".") {
        error("程序末尾缺少 '.'");
    }

    // 生成系统结束指令
    codeGen_->emit(OpCode::SYSC);
    // 退出主程序作用域
    symTable_->exitScope();
    
    AST_END();
}

/**
 * @brief 解析块（block）
 * @details 产生式：block → [const declaration][var declaration][procedure declaration]statement
 *          块是 PL/0 的基本程序单元，包含声明部分和语句部分
 */
void Parser::block() {
    AST_BEGIN(ASTNodeType::BLOCK, "");
    // 进入块作用域
    symTable_->enterScope("block");

    // 循环解析声明部分：const、var、procedure
    while (currentToken_.type == TokenType::KEYWORD) {
        if (currentToken_.value == "const") {
            // 解析常量声明
            constDeclaration();
        } else if (currentToken_.value == "var") {
            // 解析变量声明
            varDeclaration();
        } else if (currentToken_.value == "procedure") {
            // 解析过程声明
            procedureDeclaration();
        } else {  
            // 遇到非声明关键字，退出循环
            break;
        }
    }

    // 解析语句部分
    statement();
    
    // 退出块作用域
    symTable_->exitScope();
    AST_END();
}

/**
 * @brief 解析常量声明（const declaration）
 * @details 产生式：const declaration → const id = num {, id = num};
 *          处理常量定义，将常量符号添加到符号表
 */
void Parser::constDeclaration() {
    AST_BEGIN(ASTNodeType::CONST_DECL, "");
    // 匹配 'const' 关键字
    match(TokenType::KEYWORD);
    
    do {
        // 检查标识符是否存在
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("常量声明中缺少标识符");
            break;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        match(TokenType::IDENTIFIER);

        // 检查赋值符号 '='
        if (currentToken_.type != TokenType::OPERATOR || currentToken_.value != "=") {
            error("常量声明中缺少 '='");
            break;
        }
        match(TokenType::OPERATOR);

        // 检查常量值（数字）
        if (currentToken_.type != TokenType::NUMBER) {
            error("常量声明中缺少数字");
            break;
        }
        int value = std::stoi(currentToken_.value);

        // 创建常量符号并添加到符号表
        Symbol sym;
        sym.name = name;
        sym.kind = SymbolKind::CONSTANT;
        sym.value = value;
        sym.level = symTable_->getCurrentLevel();
        symTable_->addSymbol(sym);

        match(TokenType::NUMBER);
    } while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ",");

    // 检查分号结束符
    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("常量声明末尾缺少 ';'");
    }
    match(TokenType::DELIMITER);
    AST_END();
}

/**
 * @brief 解析变量声明（var declaration）
 * @details 产生式：var declaration → var id {, id};
 *          处理变量定义，将变量符号添加到符号表
 */
void Parser::varDeclaration() {
    AST_BEGIN(ASTNodeType::VAR_DECL, "");
    // 匹配 'var' 关键字
    match(TokenType::KEYWORD);

    // 检查第一个标识符
    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("变量声明中缺少标识符");
        AST_END();
        return;
    }

    // 创建第一个变量符号
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
            error("',' 后缺少标识符");
            AST_END();
            return;
        }

        // 创建后续变量符号
        AST_LEAF(ASTNodeType::IDENTIFIER, currentToken_.value);
        sym.name = currentToken_.value;
        sym.address = symTable_->getCurrentLevel();
        symTable_->addSymbol(sym);

        match(TokenType::IDENTIFIER);
    }

    // 检查分号结束符
    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("变量声明末尾缺少 ';'");
        AST_END();
        return;
    }
    match(TokenType::DELIMITER);
    AST_END();
}

/**
 * @brief 解析过程声明（procedure declaration）
 * @details 产生式：procedure declaration → procedure id; block;
 *          处理过程定义，将过程符号添加到符号表，并生成返回指令
 */
void Parser::procedureDeclaration() {
    AST_BEGIN(ASTNodeType::PROC_DECL, "");
    // 匹配 'procedure' 关键字
    match(TokenType::KEYWORD);

    // 检查过程名
    if (currentToken_.type != TokenType::IDENTIFIER) {
        error("缺少过程名");
        AST_END();
        return;
    }

    std::string name = currentToken_.value;
    AST_LEAF(ASTNodeType::IDENTIFIER, name);
    
    // 创建过程符号
    Symbol sym;
    sym.name = name;
    sym.kind = SymbolKind::PROCEDURE;
    sym.level = symTable_->getCurrentLevel();
    // 过程地址设置为当前四元式数量 + 100（预留空间）
    sym.address = codeGen_->getQuadruples().size() + 100;
    symTable_->addSymbol(sym);

    match(TokenType::IDENTIFIER);

    // 检查过程名后的分号
    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("过程名后缺少 ';'");
    }
    match(TokenType::DELIMITER);

    // 解析过程体（块）
    block();

    // 检查过程体后的分号
    if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ";") {
        error("过程体后缺少 ';'");
    }
    match(TokenType::DELIMITER);

    // 生成过程返回指令
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
    // 处理 begin...end 复合语句块
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "begin") {
        AST_BEGIN(ASTNodeType::BEGIN_END, "");
        match(TokenType::KEYWORD);
        statement();
        // 处理语句间的分号分隔
        while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ";") {
            match(TokenType::DELIMITER);
            // 遇到 end 则结束循环
            if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "end") {
                break;
            }
            statement();
        }
        // 检查结束符 end
        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "end") {
            error("缺少 'end'");
        }
        match(TokenType::KEYWORD);
        AST_END();
        return;
    }

    // 处理赋值语句 - FIRST(id) = {id}
    if (currentToken_.type == TokenType::IDENTIFIER) {
        std::string name = currentToken_.value;
        AST_BEGIN(ASTNodeType::ASSIGN_STMT, name);
        match(TokenType::IDENTIFIER);

        // 检查赋值运算符 ':='
        if (currentToken_.type != TokenType::OPERATOR || currentToken_.value != ":=") {
            error("赋值语句中缺少 ':='");
            return;
        }
        match(TokenType::OPERATOR);

        // 解析赋值表达式
        expression();
        codeGen_->emit(OpCode::ASSIGN, "", "", name);
        AST_END();
        return;
    }

    // 处理 if 条件语句 - FIRST(if) = {if}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "if") {
        AST_BEGIN(ASTNodeType::IF_STMT, "");
        match(TokenType::KEYWORD);
        // 解析条件表达式
        condition();

        // 检查 then 关键字
        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "then") {
            error("缺少 'then'");
            AST_END();
            return;
        }
        match(TokenType::KEYWORD);
        // 解析 if 体语句
        statement();
        AST_END();
        return;
    }

    // 处理 while 循环语句 - FIRST(while) = {while}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "while") {
        AST_BEGIN(ASTNodeType::WHILE_STMT, "");
        match(TokenType::KEYWORD);
        // 记录循环起始标签
        std::string startLabel = "L" + std::to_string(codeGen_->getQuadruples().size());
        // 解析循环条件
        condition();

        // 检查 do 关键字
        if (currentToken_.type != TokenType::KEYWORD || currentToken_.value != "do") {
            error("缺少 'do'");
            AST_END();
            return;
        }
        match(TokenType::KEYWORD);
        // 解析循环体语句
        statement();
        // 生成跳转到循环开始的指令
        codeGen_->emit(OpCode::JUMP, startLabel);
        AST_END();
        return;
    }

    // 处理过程调用语句 - FIRST(call) = {call}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "call") {
        AST_BEGIN(ASTNodeType::CALL_STMT, "");
        match(TokenType::KEYWORD);
        // 检查过程名
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("'call' 后缺少过程名");
            AST_END(); 
            return;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        // 生成过程调用指令
        codeGen_->emit(OpCode::CALL, name);
        match(TokenType::IDENTIFIER);
        AST_END();
        return;
    }

    // 处理 read 输入语句 - FIRST(read) = {read}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "read") {
        AST_BEGIN(ASTNodeType::READ_STMT, "");
        match(TokenType::KEYWORD);
        // 检查左括号
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != "(") {
            error("'read' 后缺少 '('"); 
            AST_END(); 
            return;
        }
        match(TokenType::DELIMITER);
        // 检查变量标识符
        if (currentToken_.type != TokenType::IDENTIFIER) {
            error("read 语句中缺少标识符"); 
            AST_END(); 
            return;
        }
        std::string name = currentToken_.value;
        AST_LEAF(ASTNodeType::IDENTIFIER, name);
        // 生成读指令
        codeGen_->emit(OpCode::READ, "", "", name);
        match(TokenType::IDENTIFIER);
        // 检查右括号
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("read 参数后缺少 ')'"); 
            AST_END(); 
            return;
        }
        match(TokenType::DELIMITER);
        AST_END();
        return;
    }

    // 处理 write 输出语句 - FIRST(write) = {write}
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "write") {
        AST_BEGIN(ASTNodeType::WRITE_STMT, "");
        match(TokenType::KEYWORD);
        // 检查左括号
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != "(") {
            error("'write' 后缺少 '('");
            return;
        }
        match(TokenType::DELIMITER);
        // 解析输出表达式
        expression();
        codeGen_->emit(OpCode::WRITE);
        // 处理多个输出参数
        while (currentToken_.type == TokenType::DELIMITER && currentToken_.value == ",") {
            match(TokenType::DELIMITER);
            expression();
            codeGen_->emit(OpCode::WRITE);
        }
        // 检查右括号
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("write 参数后缺少 ')'");
            AST_END(); 
            return;
        }
        match(TokenType::DELIMITER);
        AST_END();
        return;
    }
}

/**
 * @brief 解析条件表达式（condition）
 * @details 产生式：condition → odd expression | expression relop expression
 *          处理 odd 判断和关系表达式（=, #, <, <=, >, >=）
 */
void Parser::condition() {
    AST_BEGIN(ASTNodeType::CONDITION, "");
    // 处理 odd 条件
    if (currentToken_.type == TokenType::KEYWORD && currentToken_.value == "odd") {
        AST_LEAF(ASTNodeType::BINOP, "odd");
        match(TokenType::KEYWORD);
        expression();
        codeGen_->emit(OpCode::ODD);
    } else {
        // 处理关系表达式
        expression();
        std::string op = currentToken_.value;
        // 检查运算符
        if (currentToken_.type != TokenType::OPERATOR) {
            error("缺少比较运算符");
            AST_END(); 
            return;
        }

        // 将运算符转换为对应的操作码
        OpCode opCode;
        if (op == "=") opCode = OpCode::EQ;
        else if (op == "#") opCode = OpCode::NEQ;
        else if (op == "<") opCode = OpCode::LT;
        else if (op == "<=") opCode = OpCode::LTE;
        else if (op == ">") opCode = OpCode::GT;
        else if (op == ">=") opCode = OpCode::GTE;
        else {
            error("未知的比较运算符");
            AST_END(); 
            return;
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
    // 处理表达式开头的正负号
    if (currentToken_.type == TokenType::OPERATOR &&
        (currentToken_.value == "+" || currentToken_.value == "-")) {
        op = currentToken_.value;
        match(TokenType::OPERATOR);
    }

    // 先解析第一个 term（非递归开头）
    term();

    // 如果是负号，生成与0相减的指令（取负）
    if (!op.empty() && op == "-") {
        codeGen_->emit(OpCode::SUB, "0", "", "");
    }

    // 循环处理后续的二元运算符（右递归结构）
    while (currentToken_.type == TokenType::OPERATOR &&
           (currentToken_.value == "+" || currentToken_.value == "-")) {
        op = currentToken_.value;
        match(TokenType::OPERATOR);
        term();
        // 根据运算符生成相应的指令
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
        // 根据运算符生成相应的指令
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
    // FIRST(id) = {id} - 处理标识符
    if (currentToken_.type == TokenType::IDENTIFIER) {
        std::string name = currentToken_.value;
        // 在符号表中查找标识符
        Symbol* sym = symTable_->lookup(name);
        if (!sym) {
            error("未定义的标识符: " + name);
        }
        match(TokenType::IDENTIFIER);
        // 如果是常量，直接加载其值
        if (sym && sym->kind == SymbolKind::CONSTANT) {
            codeGen_->emit(OpCode::ADD, std::to_string(sym->value), "", "");
        }
    // FIRST(num) = {num} - 处理数字字面量
    } else if (currentToken_.type == TokenType::NUMBER) {
        int value = std::stoi(currentToken_.value);
        codeGen_->emit(OpCode::ADD, std::to_string(value), "", "");
        match(TokenType::NUMBER);
    // FIRST((expression)) = {(} - 处理括号表达式
    } else if (currentToken_.type == TokenType::DELIMITER && currentToken_.value == "(") {
        match(TokenType::DELIMITER);
        expression();
        // FOLLOW 集检查：期望 ')'
        if (currentToken_.type != TokenType::DELIMITER || currentToken_.value != ")") {
            error("缺少 ')'");
        }
        match(TokenType::DELIMITER);
    } else {
        error("表达式中遇到意外的 token");
    }
}

/**
 * @brief token 匹配函数
 * @details 检查当前 token 是否与期望类型匹配，如果匹配则读取下一个 token
 * @param expected 期望的 token 类型
 */
void Parser::match(TokenType expected) {
    if (currentToken_.type == expected) {
        currentToken_ = lexer_->getNextToken();
    }
}

/**
 * @brief 错误处理函数
 * @details 记录错误信息，设置错误标志，并调用同步函数恢复解析
 * @param message 错误信息
 */
void Parser::error(const std::string& message) {
    hasError_ = true;
    errorMessage_ = "(语法错误, 行号: " + std::to_string(currentToken_.line) +
                    ") " + message;
    std::cerr << errorMessage_ << std::endl;
    // 调用同步函数跳过错误 token，尝试恢复解析
    sync();
}

/**
 * @brief 错误同步函数
 * @details 跳过当前 token 直到遇到分隔符、文件结束符或关键字，用于错误恢复
 */
void Parser::sync() {
    while (currentToken_.type != TokenType::DELIMITER && 
           currentToken_.type != TokenType::END_OF_FILE &&
           currentToken_.type != TokenType::KEYWORD) {
        currentToken_ = lexer_->getNextToken();
    }
}

} // namespace PL0
