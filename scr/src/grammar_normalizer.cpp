/**
 * @file grammar_normalizer.cpp
 * @brief Grammar Normalizer — left-factoring + left-recursion elimination
 * @details Transforms grammar productions into normalized form by:
 *          1. Extracting common left factors from alternative productions
 *          2. Eliminating direct left recursion (A → Aα | β → A → βA', A' → αA' | ε)
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "../include/pl0_grammar_normalizer.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace PL0 {

//============================================================================
// Constructor
//============================================================================

GrammarNormalizer::GrammarNormalizer() : freshCounter_(0) {}

//============================================================================
// 新非终结符名称生成器
//============================================================================

std::string GrammarNormalizer::freshNonTerminal(const std::string& base) {
    freshCounter_++;
    return base + std::to_string(freshCounter_);
}

//============================================================================
// 查找公共前缀
//============================================================================

std::string GrammarNormalizer::findCommonPrefix(
    const std::vector<std::vector<std::string>>& rhsList,
    size_t& prefixLen) const {

    if (rhsList.size() < 2) return "";

    // Find longest common prefix across all RHS
    const auto& first = rhsList[0];
    size_t maxLen = first.size();

    for (size_t i = 1; i < rhsList.size(); i++) {
        maxLen = std::min(maxLen, rhsList[i].size());
    }

    if (maxLen == 0) return "";

    // 检查前缀长度
    size_t commonLen = 0;
    for (size_t pos = 0; pos < maxLen; pos++) {
        bool allMatch = true;
        for (size_t i = 1; i < rhsList.size(); i++) {
            if (rhsList[i][pos] != first[pos]) {
                allMatch = false;
                break;
            }
        }
        if (!allMatch) break;
        commonLen = pos + 1;
    }

    if (commonLen == 0) return "";

    prefixLen = commonLen;
    return first[0];  // return first symbol of prefix (for single-symbol case)
}

//============================================================================
// 辅助函数：检查RHS是否以给定符号开头
//============================================================================

bool GrammarNormalizer::startsWith(const std::vector<std::string>& rhs,
                                    const std::string& symbol) const {
    return !rhs.empty() && rhs[0] == symbol;
}

//============================================================================
// 左因子提取
//============================================================================

std::vector<NormProduction> GrammarNormalizer::extractLeftFactor(
    const std::vector<NormProduction>& productions) {

    std::vector<NormProduction> result;

    // 按LHS分组产生式
    std::map<std::string, std::vector<NormProduction>> grouped;
    for (const auto& p : productions) {
        grouped[p.lhs].push_back(p);
    }

    for (auto& [lhs, prods] : grouped) {
        if (prods.size() < 2) {
            result.insert(result.end(), prods.begin(), prods.end());
            continue;
        }

        // 检查首个符号是否形成公共前缀
        std::map<std::string, std::vector<NormProduction>> byFirst;
        for (const auto& p : prods) {
            if (!p.rhs.empty()) {
                byFirst[p.rhs[0]].push_back(p);
            } else {
                // Epsilon production — keep as-is
                result.push_back(p);
            }
        }

        bool factored = false;
        for (auto& [prefix, prefixProds] : byFirst) {
            if (prefixProds.size() >= 2) {
                // 发现左因子 —— 提取
                factored = true;
                std::string newNT = lhs + "_f" + std::to_string(++freshCounter_);

                // A → prefix A'
                result.push_back({lhs, {prefix, newNT}});

                // A' → β₁ | β₂ | ...
                for (const auto& p : prefixProds) {
                    std::vector<std::string> suffix(
                        p.rhs.begin() + 1, p.rhs.end());
                    if (suffix.empty()) suffix.push_back("ε");
                    result.push_back({newNT, suffix});
                }
            } else {
                // 只有一条具有此前缀的产生式 —— 原样保留
                result.insert(result.end(), prefixProds.begin(),
                              prefixProds.end());
            }
        }

        // 保留非因式化的产生式
        if (!factored) {
            result.insert(result.end(), prods.begin(), prods.end());
        }
    }

    return result;
}

//============================================================================
// 左递归消除（直接）
//============================================================================

std::vector<NormProduction> GrammarNormalizer::eliminateLeftRecursion(
    const std::vector<NormProduction>& productions) {

    std::vector<NormProduction> result;

    // 按LHS分组产生式
    std::map<std::string, std::vector<NormProduction>> grouped;
    for (const auto& p : productions) {
        grouped[p.lhs].push_back(p);
    }

    for (auto& [lhs, prods] : grouped) {
        std::vector<std::vector<std::string>> recursive;    // A → Aα
        std::vector<std::vector<std::string>> nonRecursive; // A → β

        for (const auto& p : prods) {
            if (startsWith(p.rhs, lhs)) {
                // Left-recursive: A → A α
                std::vector<std::string> alpha(
                    p.rhs.begin() + 1, p.rhs.end());
                recursive.push_back(alpha);
            } else {
                nonRecursive.push_back(p.rhs);
            }
        }

        if (!recursive.empty()) {
            std::string newNT = lhs + "'";

            // A → β A'
            for (const auto& beta : nonRecursive) {
                std::vector<std::string> newRhs = beta;
                newRhs.push_back(newNT);
                result.push_back({lhs, newRhs});
            }

            // If no non-recursive productions, grammar is ill-formed
            if (nonRecursive.empty()) {
                // 添加错误标记
                result.push_back({lhs, {"ε"}});
            }

            // A' → α A' | ε
            for (const auto& alpha : recursive) {
                std::vector<std::string> newRhs = alpha;
                newRhs.push_back(newNT);
                result.push_back({newNT, newRhs});
            }
            result.push_back({newNT, {"ε"}});
        } else {
            // 没有左递归
            result.insert(result.end(), prods.begin(), prods.end());
        }
    }

    return result;
}

//============================================================================
// 组合规范化
//============================================================================

std::vector<NormProduction> GrammarNormalizer::normalize(
    const std::vector<NormProduction>& productions) {

    // 首先应用左递归消除，然后进行左因子提取
    auto step1 = eliminateLeftRecursion(productions);
    auto step2 = extractLeftFactor(step1);
    return step2;
}

//============================================================================
// 打印产生式
//============================================================================

void GrammarNormalizer::printProductions(
    std::ostream& os,
    const std::vector<NormProduction>& prods) const {

    os << "\n===== NORMALIZED GRAMMAR PRODUCTIONS =====\n\n";

    // Group by LHS for readability
    std::map<std::string, std::vector<NormProduction>> grouped;
    for (const auto& p : prods) {
        grouped[p.lhs].push_back(p);
    }

    for (const auto& [lhs, group] : grouped) {
        os << std::left << std::setw(8) << lhs << " → ";
        bool first = true;
        for (const auto& p : group) {
            if (!first) os << std::string(10, ' ') << "| ";
            first = false;
            if (p.rhs.empty() || (p.rhs.size() == 1 && p.rhs[0] == "ε")) {
                os << "ε";
            } else {
                for (size_t i = 0; i < p.rhs.size(); i++) {
                    if (i > 0) os << " ";
                    os << p.rhs[i];
                }
            }
            os << "\n";
        }
    }
    os << "\nTotal: " << prods.size() << " productions\n";
}

} // namespace PL0
