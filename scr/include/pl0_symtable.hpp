/**
 * @file pl0_symtable.hpp
 * @brief PL/0 Compiler Symbol Table Class
 * @details This class manages the symbol table for the PL/0 compiler,
 *          supporting scoped symbol lookup, insertion, and management.
 *          It implements lexical scoping with support for constants,
 *          variables, and procedures.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_SYMBOL_TABLE_HPP
#define PL0_SYMBOL_TABLE_HPP

#include "pl0_core.hpp"

namespace PL0 {

/**
 * @class SymbolTable
 * @brief Symbol table manager with scoped lookup
 * @details The SymbolTable class maintains a stack of scopes, each containing
 *          a hash map of symbols. It supports entering/exiting scopes,
 *          adding symbols, and looking up symbols across scopes.
 */
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    // 作用域管理
    void enterScope(const std::string& name);
    void exitScope();
    int getCurrentLevel() const { return scopeStack_.size() - 1; }

    // 符号操作
    bool addSymbol(const Symbol& symbol);
    Symbol* lookup(const std::string& name);
    Symbol* lookupInCurrentScope(const std::string& name);

    // 临时变量
    std::string getNewTemp();
    int getTempCount() const { return tempCount_; }

    // 辅助函数
    void print(std::ostream& os) const;
    void clear();

private:
    struct Scope {
        std::string name;
        std::unordered_map<std::string, Symbol> symbols;
    };

    std::vector<Scope> scopeStack_;
    int tempCount_;
};

} // namespace PL0

#endif // PL0_SYMBOL_TABLE_HPP
