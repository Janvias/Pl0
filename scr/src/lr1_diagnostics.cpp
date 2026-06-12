/**
 * @file lr1_diagnostics.cpp
 * @brief LR(1) Parser Diagnostics — String Conversion, Output, Validation
 * @details Provides human-readable names for grammar symbols, prints the
 *          LR(1) parse table and canonical collection, and implements the
 *          dual-parser comparison logic for LL(1)/LR(1) cross-validation.
 * @author PL/0 Compiler Project
 * @date 2026-06-09
 */

#include "../include/pl0_lr1_parser.hpp"
#include <sstream>
#include <algorithm>

namespace PL0 {

//============================================================================
// 终结符名称转换
//============================================================================

std::string LR1Parser::terminalToString(LR1Terminal t) const {
    switch (t) {
        case LR1Terminal::tIDENT:     return "ident";
        case LR1Terminal::tNUMBER:    return "number";
        case LR1Terminal::tPLUS:      return "+";
        case LR1Terminal::tMINUS:     return "-";
        case LR1Terminal::tSTAR:      return "*";
        case LR1Terminal::tSLASH:     return "/";
        case LR1Terminal::tEQ:        return "=";
        case LR1Terminal::tNEQ:       return "#";
        case LR1Terminal::tLT:        return "<";
        case LR1Terminal::tLTE:       return "<=";
        case LR1Terminal::tGT:        return ">";
        case LR1Terminal::tGTE:       return ">=";
        case LR1Terminal::tASSIGN:    return ":=";
        case LR1Terminal::tLPAREN:    return "(";
        case LR1Terminal::tRPAREN:    return ")";
        case LR1Terminal::tDOT:       return ".";
        case LR1Terminal::tCOMMA:     return ",";
        case LR1Terminal::tSEMICOLON: return ";";
        case LR1Terminal::tCONST:     return "const";
        case LR1Terminal::tVAR:       return "var";
        case LR1Terminal::tPROCEDURE: return "procedure";
        case LR1Terminal::tBEGIN:     return "begin";
        case LR1Terminal::tEND:       return "end";
        case LR1Terminal::tIF:        return "if";
        case LR1Terminal::tTHEN:      return "then";
        case LR1Terminal::tWHILE:     return "while";
        case LR1Terminal::tDO:        return "do";
        case LR1Terminal::tCALL:      return "call";
        case LR1Terminal::tREAD:      return "read";
        case LR1Terminal::tWRITE:     return "write";
        case LR1Terminal::tODD:       return "odd";
        case LR1Terminal::tEOF:       return "$";
        case LR1Terminal::tEPSILON:   return "epsilon";
        default: return "?";
    }
}

//============================================================================
// Non-Terminal Name Conversion
//============================================================================

std::string LR1Parser::nonTerminalToString(LR1NonTerminal nt) const {
    switch (nt) {
        case LR1NonTerminal::P:  return "P";
        case LR1NonTerminal::B:  return "B";
        case LR1NonTerminal::DL: return "DL";
        case LR1NonTerminal::C:  return "C";
        case LR1NonTerminal::CL: return "CL";
        case LR1NonTerminal::V:  return "V";
        case LR1NonTerminal::VL: return "VL";
        case LR1NonTerminal::PR: return "PR";
        case LR1NonTerminal::S:  return "S";
        case LR1NonTerminal::L:  return "L";
        case LR1NonTerminal::CO: return "CO";
        case LR1NonTerminal::E:  return "E";
        case LR1NonTerminal::ET: return "ET";
        case LR1NonTerminal::T:  return "T";
        case LR1NonTerminal::TT: return "TT";
        case LR1NonTerminal::F:  return "F";
        case LR1NonTerminal::A:  return "A";
        default: return "?";
    }
}

//============================================================================
// 符号名称转换
//============================================================================

std::string LR1Parser::symbolToString(const LR1Symbol& sym) const {
    if (sym.isTerminal) return terminalToString(sym.terminal);
    return nonTerminalToString(sym.nonTerminal);
}

//============================================================================
// 产生式字符串表示
//============================================================================

std::string LR1Parser::productionToString(int prodId) const {
    if (prodId < 0 || prodId >= static_cast<int>(productions_.size()))
        return "INVALID";

    const auto& prod = productions_[prodId];
    std::ostringstream oss;
    oss << nonTerminalToString(prod.lhs) << " ->";
    if (prod.rhs.empty()) {
        oss << " epsilon";
    } else {
        for (const auto& sym : prod.rhs) {
            oss << " " << symbolToString(sym);
        }
    }
    return oss.str();
}

//============================================================================
// 分析表输出
//============================================================================

void LR1Parser::printParseTable(std::ostream& os) const {
    os << "\n===== LR(1) PARSE TABLE =====\n";
    os << "States: " << states_.size() << "\n";
    os << "Terminals: 31\n";
    os << "Productions: " << productions_.size() << "\n\n";

    os << "ACTION Table:\n";
    for (const auto& entry : actionTable_) {
        int state = entry.first.first;
        LR1Terminal term = entry.first.second;
        const LRAction& action = entry.second;

        os << "  [" << state << ", " << terminalToString(term) << "] -> ";
        if (action.type == LRActionType::SHIFT) {
            os << "S" << action.value;
        } else if (action.type == LRActionType::REDUCE) {
            os << "R" << action.value << "  ("
               << productionToString(action.value) << ")";
        } else if (action.type == LRActionType::ACCEPT) {
            os << "ACC";
        }
        os << "\n";
    }

    os << "\nGOTO Table:\n";
    for (const auto& entry : gotoTable_) {
        int state = entry.first.first;
        LR1NonTerminal nt = entry.first.second;
        int nextState = entry.second;
        os << "  [" << state << ", " << nonTerminalToString(nt)
           << "] -> " << nextState << "\n";
    }
}

//============================================================================
// 规范集合输出
//============================================================================

void LR1Parser::printStates(std::ostream& os) const {
    os << "\n===== LR(1) CANONICAL COLLECTION =====\n";
    os << "Number of states: " << states_.size() << "\n\n";

    for (size_t i = 0; i < states_.size(); i++) {
        os << "State " << i << ":\n";
        for (const auto& item : states_[i]) {
            const auto& prod = productions_[item.productionId];
            os << "  [" << nonTerminalToString(prod.lhs) << " ->";
            for (int j = 0; j < static_cast<int>(prod.rhs.size()); j++) {
                if (j == item.dot) os << " .";
                os << " " << symbolToString(prod.rhs[j]);
            }
            if (item.dot >= static_cast<int>(prod.rhs.size())) os << " .";
            os << ", " << terminalToString(item.lookahead) << "]\n";
        }
        os << "\n";
    }
}

//============================================================================
// 双分析器验证
//============================================================================

ValidationResult compareParserResults(
    bool ll1Success, const std::string& ll1Error,
    const std::vector<Quadruple>& ll1Quads,
    bool lr1Success, const std::string& lr1Error,
    const std::vector<Quadruple>& lr1Quads) {

    ValidationResult result;
    result.ll1Error = ll1Error;
    result.lr1Error = lr1Error;
    result.ll1Quads = ll1Quads;
    result.lr1Quads = lr1Quads;
    result.bothSucceeded = ll1Success && lr1Success;

    if (ll1Success != lr1Success) {
        result.resultsMatch = false;
        std::ostringstream diag;
        diag << "====== DUAL PARSER VALIDATION RESULT ======\n";
        diag << "PARSER DISAGREEMENT DETECTED!\n\n";
        if (ll1Success) {
            diag << "LL(1) Parser: SUCCESS\n";
            diag << "LR(1) Parser: FAILED\n";
            diag << "LR(1) Error: " << lr1Error << "\n";
            diag << "Diagnosis: LL(1) accepted, LR(1) rejected.\n";
        } else {
            diag << "LL(1) Parser: FAILED\n";
            diag << "LL(1) Error: " << ll1Error << "\n";
            diag << "LR(1) Parser: SUCCESS\n";
            diag << "Diagnosis: LR(1) accepted, LL(1) rejected.\n";
        }
        diag << "============================================\n";
        result.diagnosticInfo = diag.str();
        return result;
    }

    if (!ll1Success) {
        result.resultsMatch = true;
        std::ostringstream diag;
        diag << "====== DUAL PARSER VALIDATION RESULT ======\n";
        diag << "Both parsers detected syntax errors.\n";
        diag << "LL(1): " << ll1Error << "\n";
        diag << "LR(1): " << lr1Error << "\n";
        diag << "Result: Both parsers agree on failure.\n";
        diag << "============================================\n";
        result.diagnosticInfo = diag.str();
        return result;
    }

    // Both succeeded — compare quadruple counts and operations
    result.resultsMatch = (ll1Quads.size() == lr1Quads.size());
    if (result.resultsMatch) {
        for (size_t i = 0; i < ll1Quads.size(); i++) {
            if (ll1Quads[i].op != lr1Quads[i].op) {
                result.resultsMatch = false;
                break;
            }
        }
    }

    std::ostringstream diag;
    diag << "====== DUAL PARSER VALIDATION RESULT ======\n";
    if (result.resultsMatch) {
        diag << "BOTH PARSERS AGREE: Program parsed successfully.\n";
        diag << "LL(1) Quads: " << ll1Quads.size() << "\n";
        diag << "LR(1) Quads: " << lr1Quads.size() << "\n";
        diag << "Result: Verification PASSED - Output consistent.\n";
    } else {
        diag << "QUADRUPLES DIFFER: Parsing succeeded but outputs differ.\n";
        diag << "LL(1) Quads: " << ll1Quads.size() << "\n";
        diag << "LR(1) Quads: " << lr1Quads.size() << "\n";
        diag << "Detailed comparison:\n";
        size_t maxLen = std::max(ll1Quads.size(), lr1Quads.size());
        for (size_t i = 0; i < maxLen; i++) {
            diag << "  [" << i << "] ";
            if (i < ll1Quads.size())
                diag << "LL(1): " << static_cast<int>(ll1Quads[i].op);
            else
                diag << "LL(1): --";
            diag << " | ";
            if (i < lr1Quads.size())
                diag << "LR(1): " << static_cast<int>(lr1Quads[i].op);
            else
                diag << "LR(1): --";
            if (i < ll1Quads.size() && i < lr1Quads.size() &&
                ll1Quads[i].op != lr1Quads[i].op)
                diag << "  <-- MISMATCH";
            diag << "\n";
        }
    }
    diag << "============================================\n";
    result.diagnosticInfo = diag.str();

    return result;
}

} // namespace PL0
