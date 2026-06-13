/**
 * @file pl0_symtable.hpp
 * @brief PL/0编译器符号表类
 * @details 本类管理PL/0编译器的符号表，
 *          支持作用域符号查找、插入和管理，
 *          实现词法作用域，支持常量、变量和过程
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_SYMBOL_TABLE_HPP
#define PL0_SYMBOL_TABLE_HPP

#include "pl0_core.hpp"

namespace PL0 {

/**
 * @class SymbolTable
 * @brief 带作用域管理的符号表
 * @details SymbolTable类维护作用域栈，每个作用域包含符号哈希表，
 *          支持进入/退出作用域、添加符号和跨作用域查找符号
 */
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    // 作用域管理
    void enterScope(const std::string& name);  // 进入新作用域
    void exitScope();                          // 退出当前作用域
    int getCurrentLevel() const { return scopeStack_.size() - 1; }  // 获取当前层次

    // 符号操作
    bool addSymbol(const Symbol& symbol);      // 添加符号
    Symbol* lookup(const std::string& name);   // 跨作用域查找符号
    Symbol* lookupInCurrentScope(const std::string& name);  // 在当前作用域查找

    // 临时变量
    std::string getNewTemp();  // 生成新临时变量名
    int getTempCount() const { return tempCount_; }  // 获取临时变量计数

    // 辅助函数
    void print(std::ostream& os) const;  // 输出符号表
    void clear();                        // 清空符号表

private:
    // 作用域结构
    struct Scope {
        std::string name;                          // 作用域名
        std::unordered_map<std::string, Symbol> symbols;  // 符号映射表
    };

    std::vector<Scope> scopeStack_;  // 作用域栈
    int tempCount_;                  // 临时变量计数
};

} // namespace PL0

#endif // PL0_SYMBOL_TABLE_HPP
