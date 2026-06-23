/**
 * @file lr1_items.cpp
 * @brief LR(1)项目闭包与规范项目集族构造
 * @details 实现LR(1)闭包运算、GOTO函数、规范LR(1)项目集族构造，
 *          以及分析表生成（ACTION表和GOTO表）。
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
// LR(1)闭包计算
//============================================================================
// 闭包运算规则：
// 1. 对于项目[A -> α·Bβ, a]，计算FIRST(βa)作为B的向前看符号集合
// 2. 对于B的每个产生式B -> γ，将项目[B -> ·γ, b]加入闭包（b∈FIRST(βa)）
// 3. 重复直到闭包不再扩大

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
// 规范LR(1)项目集族构造
//============================================================================
// 算法步骤：
// 1. 初始化：从项目[P' -> ·P, $]的闭包开始
// 2. 对每个状态，计算对所有文法符号的GOTO
// 3. 如果GOTO产生新状态，则将其加入项目集族
// 4. 重复直到没有新状态产生
// 
// 同时构建ACTION表（终结符转移）和GOTO表（非终结符转移）

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
//============================================================================
// SHIFT和GOTO条目在规范项目集族构造过程中已填充
// 本函数负责填充REDUCE和ACCEPT动作：
// - 对于项目[A -> α·, a]，添加REDUCE动作（归约到产生式A -> α）
// - 对于项目[P' -> P·, $]，添加ACCEPT动作

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
