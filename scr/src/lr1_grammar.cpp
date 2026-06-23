/**
 * @file lr1_grammar.cpp
 * @brief PL/0 LR(1)文法定义与FIRST/FOLLOW集计算
 * @details 定义PL/0语法的全部46条产生式用于LR(1)解析，
 *          并计算所有文法符号的FIRST集（首符集）和FOLLOW集（后继符集）。
 *          FIRST集用于确定产生式选择，FOLLOW集用于ε产生式的归约判断。
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
// 定义PL/0语言的46条产生式，包括：
// - 增广文法起始产生式（P' -> P）
// - 程序块结构（P -> B .）
// - 声明部分（常量声明、变量声明、过程声明）
// - 语句部分（赋值、条件、循环、过程调用、输入输出）
// - 表达式部分（算术表达式、条件表达式）

void LR1Parser::initGrammar() {
    using T = LR1Terminal;
    using N = LR1NonTerminal;

    productions_.clear();

    // 产生式0: P' -> P（增广文法的起始产生式）
    productions_.push_back({0, N::P, {LR1Symbol(N::P)}});

    // 产生式1: P -> B .
    productions_.push_back({1, N::P, {LR1Symbol(N::B), LR1Symbol(T::tDOT)}});

    // 产生式2: B -> DL S
    productions_.push_back({2, N::B, {LR1Symbol(N::DL), LR1Symbol(N::S)}});

    // 声明部分: DL -> DL C | DL V | DL PR | ε
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

    // CL -> , I = N CL | ε
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

    // VL -> , I VL | ε
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

    // 语句部分
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

    // A -> , E A | ε
    productions_.push_back({23, N::A, {
        LR1Symbol(T::tCOMMA), LR1Symbol(N::E), LR1Symbol(N::A)
    }});
    productions_.push_back({24, N::A, {}});

    // S -> ε（空语句）
    productions_.push_back({25, N::S, {}});

    // 条件部分
    productions_.push_back({26, N::CO, {
        LR1Symbol(T::tODD), LR1Symbol(N::E)
    }});

    // CO -> E relop E（每个关系运算符一条产生式）
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

    // 表达式部分
    productions_.push_back({33, N::E, {
        LR1Symbol(T::tPLUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({34, N::E, {
        LR1Symbol(T::tMINUS), LR1Symbol(N::T), LR1Symbol(N::ET)
    }});
    productions_.push_back({35, N::E, {
        LR1Symbol(N::T), LR1Symbol(N::ET)
    }});

    // ET -> + T ET | - T ET | ε
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

    // TT -> * F TT | / F TT | ε
    productions_.push_back({40, N::TT, {
        LR1Symbol(T::tSTAR), LR1Symbol(N::F), LR1Symbol(N::TT)
    }});
    productions_.push_back({41, N::TT, {
        LR1Symbol(T::tSLASH), LR1Symbol(N::F), LR1Symbol(N::TT)
    }});
    productions_.push_back({42, N::TT, {}});

    // 因子部分
    productions_.push_back({43, N::F, {LR1Symbol(T::tIDENT)}});
    productions_.push_back({44, N::F, {LR1Symbol(T::tNUMBER)}});
    productions_.push_back({45, N::F, {
        LR1Symbol(T::tLPAREN), LR1Symbol(N::E), LR1Symbol(T::tRPAREN)
    }});
}

//============================================================================
// FIRST集计算
//============================================================================
// FIRST集定义：对于文法符号X，FIRST(X)是从X推导出的串的首字符集合。
// 算法步骤：
// 1. 对所有终结符t，FIRST(t) = {t}
// 2. 对非终结符A，若有产生式A -> X1 X2 ... Xn，则将FIRST(X1)中非ε元素加入FIRST(A)
// 3. 若X1能推导出ε，则继续加入FIRST(X2)，依此类推
// 4. 迭代直到FIRST集不再变化

void LR1Parser::computeFirstSets() {
    // 对所有终结符，FIRST(t) = {t}
    for (int i = 0; i <= static_cast<int>(LR1Terminal::tEPSILON); i++) {
        LR1Terminal t = static_cast<LR1Terminal>(i);
        firstSets_[LR1Symbol(t)] = {t};
    }

    // 初始化所有非终结符的FIRST集为空
    for (int i = 0; i <= static_cast<int>(LR1NonTerminal::A); i++) {
        LR1NonTerminal nt = static_cast<LR1NonTerminal>(i);
        firstSets_[LR1Symbol(nt)] = {};
    }

    // 迭代直至不动点
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions_) {
            if (prod.id == 0) continue; // 跳过增广起始产生式
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
// FOLLOW集计算
//============================================================================
// FOLLOW集定义：对于非终结符A，FOLLOW(A)是在某个句型中紧跟在A右边的终结符集合。
// 算法步骤：
// 1. 起始符号的FOLLOW集包含$（输入结束标记）
// 2. 对产生式A -> αBβ，将FIRST(β)中除ε外的元素加入FOLLOW(B)
// 3. 若β能推导出ε，则将FOLLOW(A)加入FOLLOW(B)
// 4. 迭代直到FOLLOW集不再变化

void LR1Parser::computeFollowSets() {
    using T = LR1Terminal;

    // 初始化所有非终结符的FOLLOW集为空
    for (int i = 0; i <= static_cast<int>(LR1NonTerminal::A); i++) {
        followSets_[static_cast<LR1NonTerminal>(i)] = {};
    }

    // 起始符号的FOLLOW集包含EOF
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

                // 计算FIRST(beta)
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

                // FOLLOW(B) |= FIRST(beta) - {ε}
                for (const auto& t : firstBeta) {
                    if (followB.insert(t).second) changed = true;
                }

                // 如果beta =>* ε: FOLLOW(B) |= FOLLOW(A)
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
// 符号串的FIRST集计算
//============================================================================
// 计算符号串X1 X2 ... Xn的FIRST集
// 规则：FIRST(X1 X2 ... Xn) = FIRST(X1) ∪ FIRST(X2)（若X1->ε）∪ ...

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
// 构建从非终结符到其产生式列表的映射，加速LR(1)闭包计算过程

std::map<LR1NonTerminal, std::vector<int>> buildProductionIndex(
    const std::vector<LR1Production>& productions) {

    std::map<LR1NonTerminal, std::vector<int>> index;
    for (const auto& prod : productions) {
        index[prod.lhs].push_back(prod.id);
    }
    return index;
}

} // namespace PL0
