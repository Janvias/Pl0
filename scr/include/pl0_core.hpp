/**
 * @file pl0_core.hpp
 * @brief PL/0 Compiler Core Definitions
 * @details This file contains core data structures and enumerations
 *          used throughout the PL/0 compiler, including TokenType,
 *          SymbolKind, OpCode, and related structs.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_CORE_HPP
#define PL0_CORE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace PL0 {

//============================================================================
// TokenType 枚举
//============================================================================

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    DELIMITER,
    ERROR,
    END_OF_FILE
};

//============================================================================
// SymbolKind 枚举
//============================================================================

enum class SymbolKind {
    CONSTANT,
    VARIABLE,
    PROCEDURE
};

//============================================================================
// 常量定义
//============================================================================

/**
 * @brief 标识符最大长度
 */
static constexpr size_t MAX_IDENTIFIER_LENGTH = 8;

/**
 * @brief 数字最大长度
 */
static constexpr size_t MAX_NUMBER_LENGTH = 8;

/**
 * @brief 四元式起始地址
 */
static constexpr int START_ADDRESS = 100;

//============================================================================
// OpCode 枚举
//============================================================================

enum class OpCode {
    // 算术运算
    ADD,
    SUB,
    MUL,
    DIV,
    
    // 比较运算
    ODD,
    EQ,
    NEQ,
    LT,
    LTE,
    GT,
    GTE,
    
    // 赋值
    ASSIGN,
    
    // 跳转
    JUMP,
    JZ,
    JNZ,
    
    // 函数调用
    CALL,
    RET,
    
    // 系统调用
    SYSS,
    SYSC,
    READ,
    WRITE
};

//============================================================================
// Token 结构体
//============================================================================

struct Token {
    TokenType type;
    std::string value;
    int line;

    Token() : type(TokenType::ERROR), value(""), line(0) {}
    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {}
};

//============================================================================
// Symbol 结构体
//============================================================================

struct Symbol {
    std::string name;
    SymbolKind kind;
    int value;
    int level;
    int address;
    int size;

    Symbol() : name(""), kind(SymbolKind::VARIABLE),
               value(0), level(0), address(0), size(1) {}
};

//============================================================================
// Quadruple 结构体
//============================================================================

struct Quadruple {
    OpCode op;
    std::string arg1;
    std::string arg2;
    std::string result;
    int index;

    Quadruple() : op(OpCode::ADD), arg1(""), arg2(""), result(""), index(0) {}
    Quadruple(OpCode o, const std::string& a1, const std::string& a2,
              const std::string& r, int idx)
        : op(o), arg1(a1), arg2(a2), result(r), index(idx) {}
};

} // namespace PL0

#endif // PL0_CORE_HPP
