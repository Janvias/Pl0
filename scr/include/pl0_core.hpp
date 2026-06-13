/**
 * @file pl0_core.hpp
 * @brief PL/0编译器核心定义
 * @details 本文件包含PL/0编译器使用的核心数据结构和枚举类型，
 *          包括TokenType、SymbolKind、OpCode及相关结构体
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
// TokenType 枚举 - 词法分析Token类型
//============================================================================

enum class TokenType {
    KEYWORD,      // 关键字
    IDENTIFIER,   // 标识符
    NUMBER,       // 数字
    OPERATOR,     // 运算符
    DELIMITER,    // 分隔符
    ERROR,        // 错误Token
    END_OF_FILE   // 文件结束
};

//============================================================================
// SymbolKind 枚举 - 符号类型
//============================================================================

enum class SymbolKind {
    CONSTANT,   // 常量
    VARIABLE,   // 变量
    PROCEDURE   // 过程
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
// OpCode 枚举 - 操作码
//============================================================================

enum class OpCode {
    // 算术运算
    ADD,   // 加法
    SUB,   // 减法
    MUL,   // 乘法
    DIV,   // 除法
    
    // 比较运算
    ODD,   // 奇数判断
    EQ,    // 等于
    NEQ,   // 不等于
    LT,    // 小于
    LTE,   // 小于等于
    GT,    // 大于
    GTE,   // 大于等于
    
    // 赋值
    ASSIGN,  // 赋值
    
    // 跳转
    JUMP,  // 无条件跳转
    JZ,    // 条件为零跳转
    JNZ,   // 条件非零跳转
    
    // 函数调用
    CALL,  // 调用过程
    RET,   // 返回
    
    // 系统调用
    SYSS,  // 系统停止
    SYSC,  // 系统继续
    READ,  // 读入
    WRITE  // 输出
};

//============================================================================
// Token 结构体 - 词法单元
//============================================================================

struct Token {
    TokenType type;       // Token类型
    std::string value;    // Token值
    int line;             // 所在行号

    Token() : type(TokenType::ERROR), value(""), line(0) {}
    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {}
};

//============================================================================
// Symbol 结构体 - 符号表条目
//============================================================================

struct Symbol {
    std::string name;   // 符号名
    SymbolKind kind;    // 符号类型
    int value;          // 常量值（仅常量使用）
    int level;          // 嵌套层次
    int address;        // 相对地址
    int size;           // 大小（过程使用）

    Symbol() : name(""), kind(SymbolKind::VARIABLE),
               value(0), level(0), address(0), size(1) {}
};

//============================================================================
// Quadruple 结构体 - 四元式
//============================================================================

struct Quadruple {
    OpCode op;            // 操作码
    std::string arg1;     // 第一个操作数
    std::string arg2;     // 第二个操作数
    std::string result;   // 结果
    int index;            // 四元式序号

    Quadruple() : op(OpCode::ADD), arg1(""), arg2(""), result(""), index(0) {}
    Quadruple(OpCode o, const std::string& a1, const std::string& a2,
              const std::string& r, int idx)
        : op(o), arg1(a1), arg2(a2), result(r), index(idx) {}
};

} // namespace PL0

#endif // PL0_CORE_HPP
