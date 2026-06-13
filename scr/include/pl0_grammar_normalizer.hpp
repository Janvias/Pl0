/**
 * @file pl0_grammar_normalizer.hpp
 * @brief PL/0语法规范化 — 左因子提取 + 左递归消除
 * @details 分析输入文法产生式，检测公共前缀和左递归模式，
 *          将其转换为等价的无左递归、左因子形式，适用于LL(1)解析
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_GRAMMAR_NORMALIZER_HPP
#define PL0_GRAMMAR_NORMALIZER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

namespace PL0 {

//============================================================================
// 文法产生式（字符串形式）
//============================================================================

struct NormProduction {
    std::string lhs;                   // 左部（非终结符）
    std::vector<std::string> rhs;      // 右部符号列表

    NormProduction() {}
    NormProduction(const std::string& l, const std::vector<std::string>& r)
        : lhs(l), rhs(r) {}
};

//============================================================================
// 文法规范化器
//============================================================================

class GrammarNormalizer {
public:
    GrammarNormalizer();

    // 左因子提取：A → αβ₁ | αβ₂  →  A → αA', A' → β₁ | β₂
    std::vector<NormProduction> extractLeftFactor(
        const std::vector<NormProduction>& productions);

    // 直接左递归消除：A → Aα | β  →  A → βA', A' → αA' | ε
    std::vector<NormProduction> eliminateLeftRecursion(
        const std::vector<NormProduction>& productions);

    // 应用两种变换
    std::vector<NormProduction> normalize(
        const std::vector<NormProduction>& productions);

    // 诊断输出
    void printProductions(std::ostream& os,
                          const std::vector<NormProduction>& prods) const;

private:
    int freshCounter_;  // 用于生成唯一的新非终结符名

    std::string freshNonTerminal(const std::string& base);  // 生成新非终结符
    std::string findCommonPrefix(const std::vector<std::vector<std::string>>& rhsList,
                                  size_t& prefixLen) const;  // 查找公共前缀
    bool startsWith(const std::vector<std::string>& rhs,
                    const std::string& symbol) const;  // 判断右部是否以符号开头
};

} // namespace PL0

#endif // PL0_GRAMMAR_NORMALIZER_HPP
