/**
 * @file lr1_grammar.cpp
 * @brief PL/0 LR(1) Grammar Definition and FIRST/FOLLOW Sets
 * @details Defines all 46 productions of the PL/0 grammar for LR(1) parsing,
 *          and computes FIRST and FOLLOW sets for all grammar symbols.
 * @author PL/0 Compiler Project
 * @date 2026-06-09
 */

#include "../include/pl0_lr1_parser.hpp"
#include <map>
#include <vector>

namespace PL0 {

//============================================================================
// 文法初始化
//============================================================================

void LR1Parser::initGrammar() {
    using T = LR1Terminal;
    using N = LR1NonTerminal;

    productions_.clear();

    // Production 0: P' -> P  (augmented start)
    productions_.push_back({0, N::P, {LR1Symbol(N::P)}});

    // Production 1: P -> B .
    productions_.push_back({1, N::P, {LR1Symbol(N::B), LR1Symbol(T::tDOT)}});

    // Production 2: B -> DL S
    productions_.push_back({2, N::B, {LR1Symbol(N::DL), LR1Symbol(N::S)}});

    // Declarations: DL -> DL C | DL V | DL PR | epsilon
    productions_.push_back({3, N::DL, {LR1Symbol(N::DL), LR1Symbol(N::C)}});
    productions_.push_back({4, N::DL, {LR1Symbol(N::DL), LR1Symbol(N::V)}});
    productions_.push_back({5, N::DL, {LR1Symbol(N::DL), LR1Symbol(N::PR)}});
    productions_.push_back({6, N::DL, {}});

    // C -> const I = N CL ;
    productions_.push_back({7, N::C, {
        LR1Symbol(T::tCONST), LR1Symbol(T::tIDENT),
        LR1Symbol(T::tEQ), LR1Symbol(T::tNUMBER),
        LR1Symbol(N::CL), LR1Symbol(T::tSEMICOLON)
    }});

    // CL -> , I = N CL | epsilon
    productions_.push_back({8, N::CL, {
        LR1Symbol(T::tCOMMA), LR1Symbol(T::tIDENT),
        LR1Symbol(T::tEQ), LR1Symbol(T::tNUMBER), LR1Symbol(N::CL)
    }});
    productions_.push_back({9, N::CL, {}});

    // V -> var I VL ;
    productions_.push_back({10, N::V, {
        LR1Symbol(T::tVAR), LR1Symbol(T::tIDENT),
        LR1Symbol(N::VL), LR1Symbol(T::tSEMICOLON)
    }});

    // VL -> , I VL | epsilon
    productions_.push_back({11, N::VL, {
        LR1Symbol(T::tCOMMA), LR1Symbol(T::tIDENT), LR1Symbol(N::VL)
    }});
    productions_.push_back({12, N::VL, {}});

    // PR -> procedure I ; B ;
    productions_.push_back({13, N::PR, {
        LR1Symbol(T::tPROCEDURE), LR1Symbol(T::tIDENT),
        LR1Symbol(T::tSEMICOLON), LR1Symbol(N::B),
        LR1Symbol(T::tSEMICOLON)
    }});

    // Statements
    productions_.push_back({14, N::S, {
        LR1Symbol(T::tIDENT), LR1Symbol(T::tASSIGN), LR1Symbol(N::E)
    }});
    productions_.push_back({15, N::S, {
        LR1Symbol(T::tBEGIN), LR1Symbol(N::S), LR1Symbol(N::L),
        LR1Symbol(T::tEND)
    }});
    productions_.push_back({16, N::L, {
        LR1Symbol(T::tSEMICOLON), LR1Symbol(N::S), LR1Symbol(N::L)
    }});
    productions_.push_back({17, N::L, {}});
    productions_.push_back({18, N::S, {
        LR1Symbol(T::tIF), LR1Symbol(N::CO),
        LR1Symbol(T::tTHEN), LR1Symbol(N::S)
    }});
    productions_.push_back({19, N::S, {
        LR1Symbol(T::tWHILE), LR1Symbol(N::CO),
        LR1Symbol(T::tDO), LR1Symbol(N::S)
    }});
    productions_.push_back({20, N::S, {
        LR1Symbol(T::tCALL), LR1Symbol(T::tIDENT)
    }});
    productions_.push_back({21, N::S, {
        LR1Symbol(T::tREAD), LR1Symbol(T::tLPAREN),
        LR1Symbol(T::tIDENT), LR1Symbol(T::tRPAREN)
    }});
    productions_.push_back({22, N::S, {
        LR1Symbol(T::tWRITE), LR1Symbol(T::tLPAREN),
        LR1Symbol(N::E), LR1Symbol(N::A), LR1Symbol(T::tRPAREN)
    }});

    // A -> , E A | epsilon
    productions_.push_back({23, N::A, {
        LR1Symbol(T::tCOMMA), LR1Symbol(N::E), LR1Symbol(N::A)
    }});
    productions_.push_back({24, N::A, {}});

    // S -> epsilon (empty statement)
    productions_.push_back({25, N::S, {}});

    // Conditions
    productions_.push_back({26, N::CO, {
        LR1Symbol(T::tODD), LR1Symbol(N::E)
    }});

    // CO -> E relop E (one production per relop)
    productions_.push_back({27, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tEQ), LR1Symbol(N::E)
    }});
    productions_.push_back({28, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tNEQ), LR1Symbol(N::E)
    }});
    productions_.push_back({29, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tLT), LR1Symbol(N::E)
    }});
    productions_.push_back({30, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tLTE), LR1Symbol(N::E)
    }});
    productions_.push_back({31, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tGT), LR1Symbol(N::E)
    }});
    productions_.push_back({32, N::CO, {
        LR1Symbol(N::E), LR1Symbol(T::tGTE), LR1Symbol(N::E)
    }});

    // Expressions
    productions_.push_back({33, N::E, {
        LR1Symbol(T::tPLUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({34, N::E, {
        LR1Symbol(T::tMINUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({35, N::E, {
        LR1Symbol(N::T), LR1Symbol(N::ET)
    }});

    // ET -> + T ET | - T ET | epsilon
    productions_.push_back({36, N::ET, {
        LR1Symbol(T::tPLUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({37, N::ET, {
        LR1Symbol(T::tMINUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({38, N::ET, {}});

    // T -> F TT
    productions_.push_back({39, N::T, {
        LR1Symbol(N::F), LR1Symbol(N::TT)
    }});

    // TT -> * F TT | / F TT | epsilon
    productions_.push_back({40, N::TT, {
        LR1Symbol(T::tSTAR), LR1Symbol(N::F), LR1Symbol(N::TT)
    }});
    productions_.push_back({41, N::TT, {
        LR1Symbol(T::tSLASH), LR1Symbol(N::F), LR1Symbol(N::TT)
    }});
    productions_.push_back({42, N::TT, {}});

    // Factors
    productions_.push_back({43, N::F, {LR1Symbol(T::tIDENT)}});
    productions_.push_back({44, N::F, {LR1Symbol(T::tNUMBER)}});
    productions_.push_back({45, N::F, {
        LR1Symbol(T::tLPAREN), LR1Symbol(N::E), LR1Symbol(T::tRPAREN)
    }});
}

//============================================================================
// FIRST Set Computation
//============================================================================

void LR1Parser::computeFirstSets() {
    // FIRST(t) = {t} for all terminals
    for (int i = 0; i <= static_cast<int>(LR1Terminal::tEPSILON); i++) {
        LR1Terminal t = static_cast<LR1Terminal>(i);
        firstSets_[LR1Symbol(t)] = {t};
    }

    // Initialize FIRST for all non-terminals as empty
    for (int i = 0; i <= static_cast<int>(LR1NonTerminal::A); i++) {
        LR1NonTerminal nt = static_cast<LR1NonTerminal>(i);
        firstSets_[LR1Symbol(nt)] = {};
    }

    // Iterate until fixed point
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions_) {
            if (prod.id == 0) continue; // skip augmented start
            LR1Symbol lhs(prod.lhs);
            auto& firstLHS = firstSets_[lhs];

            if (prod.rhs.empty()) {
                if (firstLHS.insert(LR1Terminal::tEPSILON).second)
                    changed = true;
                continue;
            }

            bool allCanBeEmpty = true;
            for (const auto& sym : prod.rhs) {
                const auto& firstSym = firstSets_[sym];
                for (const auto& t : firstSym) {
                    if (t != LR1Terminal::tEPSILON) {
                        if (firstLHS.insert(t).second)
                            changed = true;
                    }
                }
                if (firstSym.find(LR1Terminal::tEPSILON) == firstSym.end()) {
                    allCanBeEmpty = false;
                    break;
                }
            }
            if (allCanBeEmpty) {
                if (firstLHS.insert(LR1Terminal::tEPSILON).second)
                    changed = true;
            }
        }
    }
}

//============================================================================
// FOLLOW Set Computation
//============================================================================

void LR1Parser::computeFollowSets() {
    using T = LR1Terminal;

    // Initialize all non-terminal FOLLOW sets as empty
    for (int i = 0; i <= static_cast<int>(LR1NonTerminal::A); i++) {
        followSets_[static_cast<LR1NonTerminal>(i)] = {};
    }

    // Start symbol gets EOF
    followSets_[LR1NonTerminal::P].insert(T::tEOF);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions_) {
            if (prod.id == 0) continue;

            for (size_t i = 0; i < prod.rhs.size(); i++) {
                if (prod.rhs[i].isTerminal) continue;

                LR1NonTerminal B = prod.rhs[i].nonTerminal;
                auto& followB = followSets_[B];

                // Compute FIRST(beta)
                std::set<T> firstBeta;
                bool allCanBeEmpty = true;
                for (size_t j = i + 1; j < prod.rhs.size(); j++) {
                    const auto& firstSym = firstSets_[prod.rhs[j]];
                    for (const auto& t : firstSym) {
                        if (t != T::tEPSILON) firstBeta.insert(t);
                    }
                    if (firstSym.find(T::tEPSILON) == firstSym.end()) {
                        allCanBeEmpty = false;
                        break;
                    }
                }

                // FOLLOW(B) |= FIRST(beta) - {epsilon}
                for (const auto& t : firstBeta) {
                    if (followB.insert(t).second) changed = true;
                }

                // If beta =>* epsilon: FOLLOW(B) |= FOLLOW(A)
                if (allCanBeEmpty) {
                    for (const auto& t : followSets_[prod.lhs]) {
                        if (followB.insert(t).second) changed = true;
                    }
                }
            }
        }
    }
}

//============================================================================
// FIRST of a symbol sequence
//============================================================================

std::set<LR1Terminal> LR1Parser::computeFirst(
    const std::vector<LR1Symbol>& symbols, size_t startPos) {

    using T = LR1Terminal;
    std::set<T> result;
    bool allCanBeEmpty = true;

    for (size_t i = startPos; i < symbols.size(); i++) {
        const auto& firstSym = firstSets_[symbols[i]];
        for (const auto& t : firstSym) {
            if (t != T::tEPSILON) result.insert(t);
        }
        if (firstSym.find(T::tEPSILON) == firstSym.end()) {
            allCanBeEmpty = false;
            break;
        }
    }

    if (allCanBeEmpty) result.insert(T::tEPSILON);
    return result;
}

//============================================================================
// 产生式索引（用于快速闭包查找）
//============================================================================

std::map<LR1NonTerminal, std::vector<int>> buildProductionIndex(
    const std::vector<LR1Production>& productions) {

    std::map<LR1NonTerminal, std::vector<int>> index;
    for (const auto& prod : productions) {
        index[prod.lhs].push_back(prod.id);
    }
    return index;
}

} // namespace PL0
