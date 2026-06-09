/**
 * @file symtable.cpp
 * @brief Symbol Table Implementation
 */

#include "../include/pl0_symtable.hpp"

namespace PL0 {

//============================================================================
// SymbolTable 实现
//============================================================================

SymbolTable::SymbolTable() : tempCount_(0) {
    enterScope("global");
}

SymbolTable::~SymbolTable() {
    // 析构时自动清理
}

void SymbolTable::enterScope(const std::string& name) {
    Scope scope;
    scope.name = name;
    scopeStack_.push_back(scope);
}

void SymbolTable::exitScope() {
    if (scopeStack_.size() > 1) {
        scopeStack_.pop_back();
    }
}

bool SymbolTable::addSymbol(const Symbol& symbol) {
    if (scopeStack_.empty()) return false;
    auto& currentScope = scopeStack_.back();
    if (currentScope.symbols.find(symbol.name) != currentScope.symbols.end()) {
        return false;
    }
    currentScope.symbols[symbol.name] = symbol;
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    for (auto it = scopeStack_.rbegin(); it != scopeStack_.rend(); ++it) {
        auto found = it->symbols.find(name);
        if (found != it->symbols.end()) {
            return &found->second;
        }
    }
    return nullptr;
}

Symbol* SymbolTable::lookupInCurrentScope(const std::string& name) {
    if (scopeStack_.empty()) return nullptr;
    auto& currentScope = scopeStack_.back();
    auto found = currentScope.symbols.find(name);
    if (found != currentScope.symbols.end()) {
        return &found->second;
    }
    return nullptr;
}

std::string SymbolTable::getNewTemp() {
    tempCount_++;
    return "T" + std::to_string(tempCount_);
}

void SymbolTable::print(std::ostream& os) const {
    os << "\n===== SYMBOL TABLE =====\n";
    os << std::left << std::setw(15) << "Name"
       << std::setw(10) << "Kind"
       << std::setw(8) << "Value"
       << std::setw(8) << "Level"
       << std::setw(10) << "Address" << "\n";
    os << "--------------- ---------- -------- -------- ----------\n";

    for (const auto& scope : scopeStack_) {
        if (scope.name != "global") {
            os << "--- Scope: " << scope.name << " ---\n";
        }
        for (const auto& pair : scope.symbols) {
            const Symbol& s = pair.second;
            std::string kindStr;
            switch (s.kind) {
                case SymbolKind::CONSTANT:   kindStr = "const"; break;
                case SymbolKind::VARIABLE:  kindStr = "var"; break;
                case SymbolKind::PROCEDURE:  kindStr = "procedure"; break;
            }
            os << std::left << std::setw(15) << s.name
               << std::setw(10) << kindStr
               << std::setw(8) << s.value
               << std::setw(8) << s.level
               << std::setw(10) << s.address << "\n";
        }
    }
}

void SymbolTable::clear() {
    scopeStack_.clear();
    enterScope("global");
    tempCount_ = 0;
}

} // namespace PL0
