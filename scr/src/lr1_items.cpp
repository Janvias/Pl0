/**
 * @file lr1_items.cpp
 * @brief LR(1) Item Closure and Canonical Collection Construction
 * @details Implements the LR(1) closure operation, GOTO function,
 *          canonical LR(1) collection construction, and parse table
 *          generation (ACTION + GOTO tables).
 * @author PL/0 Compiler Project
 * @date 2026-06-09
 */

#include "../include/pl0_lr1_parser.hpp"
#include <map>
#include <set>
#include <deque>
#include <vector>

namespace PL0 {

// Forward declaration from lr1_grammar.cpp
std::map<LR1NonTerminal, std::vector<int>> buildProductionIndex(
    const std::vector<LR1Production>& productions);

//============================================================================
// LR(1) Closure Computation
//============================================================================

LR1State LR1Parser::computeClosure(const LR1State& items) {
    using T = LR1Terminal;

    // Build production index once and cache it (static)
    static auto prodIndex = buildProductionIndex(productions_);

    LR1State closure = items;
    std::vector<LR1Item> worklist(items.begin(), items.end());

    while (!worklist.empty()) {
        LR1Item item = worklist.back();
        worklist.pop_back();

        const auto& prod = productions_[item.productionId];
        if (item.dot >= static_cast<int>(prod.rhs.size())) continue;

        const LR1Symbol& nextSym = prod.rhs[item.dot];
        if (nextSym.isTerminal) continue;

        LR1NonTerminal B = nextSym.nonTerminal;

        // Compute FIRST(beta a) where beta follows B
        std::set<T> lookaheads;
        bool betaCanBeEmpty = true;

        for (size_t j = item.dot + 1; j < prod.rhs.size(); j++) {
            const auto& firstSym = firstSets_[prod.rhs[j]];
            for (const auto& t : firstSym) {
                if (t != T::tEPSILON) lookaheads.insert(t);
            }
            if (firstSym.find(T::tEPSILON) == firstSym.end()) {
                betaCanBeEmpty = false;
                break;
            }
        }
        if (betaCanBeEmpty) {
            lookaheads.insert(item.lookahead);
        }

        // Add productions of B for each computed lookahead
        auto it = prodIndex.find(B);
        if (it == prodIndex.end()) continue;

        for (int pid : it->second) {
            for (const auto& la : lookaheads) {
                LR1Item newItem(pid, 0, la);
                if (closure.insert(newItem).second) {
                    worklist.push_back(newItem);
                }
            }
        }
    }

    return closure;
}

//============================================================================
// Canonical LR(1) Collection Construction
// Builds all LR(1) states and populates SHIFT/GOTO entries simultaneously
//============================================================================

void LR1Parser::buildCanonicalCollection() {
    using T = LR1Terminal;

    // Start with closure of [P' -> .P, $]
    LR1State startKernel;
    startKernel.insert(LR1Item(0, 0, LR1Terminal::tEOF));
    LR1State startState = computeClosure(startKernel);

    // Map each state to its index for deduplication
    std::map<LR1State, int> stateToIndex;
    stateToIndex[startState] = 0;
    states_.push_back(std::move(startState));

    // Worklist-based state exploration
    std::deque<int> worklist;
    worklist.push_back(0);

    while (!worklist.empty()) {
        int stateIdx = worklist.front();
        worklist.pop_front();

        // Copy state to avoid reference invalidation when vector grows
        LR1State currentState = states_[stateIdx];

        // Collect grammar symbols that appear after the dot
        std::set<LR1Symbol> symbolsToTry;
        for (const auto& item : currentState) {
            const auto& prod = productions_[item.productionId];
            if (item.dot < static_cast<int>(prod.rhs.size())) {
                symbolsToTry.insert(prod.rhs[item.dot]);
            }
        }

        // Compute GOTO for each such symbol
        for (const auto& sym : symbolsToTry) {
            LR1State kernel;
            for (const auto& item : currentState) {
                const auto& prod = productions_[item.productionId];
                if (item.dot < static_cast<int>(prod.rhs.size()) &&
                    prod.rhs[item.dot] == sym) {
                    kernel.insert(LR1Item(item.productionId, item.dot + 1,
                                          item.lookahead));
                }
            }
            if (kernel.empty()) continue;

            LR1State gotoState = computeClosure(kernel);

            // Check if the state already exists
            int targetIdx;
            auto it = stateToIndex.find(gotoState);
            if (it == stateToIndex.end()) {
                targetIdx = static_cast<int>(states_.size());
                stateToIndex[gotoState] = targetIdx;
                states_.push_back(std::move(gotoState));
                worklist.push_back(targetIdx);
            } else {
                targetIdx = it->second;
            }

            // Record transition
            if (sym.isTerminal && sym.terminal != T::tEPSILON) {
                auto key = std::make_pair(stateIdx, sym.terminal);
                if (actionTable_.find(key) == actionTable_.end()) {
                    actionTable_[key] = LRAction(LRActionType::SHIFT, targetIdx);
                }
            } else if (!sym.isTerminal) {
                auto key = std::make_pair(stateIdx, sym.nonTerminal);
                gotoTable_[key] = targetIdx;
            }
        }
    }
}

//============================================================================
// 分析表构造（REDUCE + ACCEPT 动作）
// SHIFT and GOTO entries were populated during canonical collection
//============================================================================

void LR1Parser::buildParseTables() {
    using T = LR1Terminal;

    for (size_t stateIdx = 0; stateIdx < states_.size(); stateIdx++) {
        const auto& state = states_[stateIdx];

        for (const auto& item : state) {
            const auto& prod = productions_[item.productionId];

            if (item.dot >= static_cast<int>(prod.rhs.size())) {
                if (item.productionId == 0) {
                    // P' -> P . => ACCEPT on EOF
                    auto key = std::make_pair(static_cast<int>(stateIdx), T::tEOF);
                    actionTable_[key] = LRAction(LRActionType::ACCEPT, 0);
                } else {
                    // REDUCE on the item's lookahead
                    auto key = std::make_pair(static_cast<int>(stateIdx),
                                              item.lookahead);
                    if (actionTable_.find(key) == actionTable_.end()) {
                        actionTable_[key] = LRAction(LRActionType::REDUCE,
                                                      item.productionId);
                    }
                    // SHIFT/REDUCE conflict: SHIFT wins (already in table)
                }
            }
        }
    }
}

} // namespace PL0
