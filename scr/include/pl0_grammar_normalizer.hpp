/**
 * @file pl0_grammar_normalizer.hpp
 * @brief PL/0 Grammar Normalization — left-factoring + left-recursion elimination
 * @details Analyzes input grammar productions, detects common prefixes and
 *          left-recursive patterns, and transforms them into equivalent
 *          non-left-recursive, factored form suitable for LL(1) parsing.
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
// Grammar Production (string-based)
//============================================================================

struct NormProduction {
    std::string lhs;                   // Left-hand side
    std::vector<std::string> rhs;      // Right-hand side symbols

    NormProduction() {}
    NormProduction(const std::string& l, const std::vector<std::string>& r)
        : lhs(l), rhs(r) {}
};

//============================================================================
// Grammar Normalizer
//============================================================================

class GrammarNormalizer {
public:
    GrammarNormalizer();

    // Left-factoring: A → αβ₁ | αβ₂  →  A → αA', A' → β₁ | β₂
    std::vector<NormProduction> extractLeftFactor(
        const std::vector<NormProduction>& productions);

    // Direct left-recursion elimination: A → Aα | β  →  A → βA', A' → αA' | ε
    std::vector<NormProduction> eliminateLeftRecursion(
        const std::vector<NormProduction>& productions);

    // Apply both transformations
    std::vector<NormProduction> normalize(
        const std::vector<NormProduction>& productions);

    // Diagnostic output
    void printProductions(std::ostream& os,
                          const std::vector<NormProduction>& prods) const;

private:
    int freshCounter_;  // for generating unique new non-terminal names

    std::string freshNonTerminal(const std::string& base);
    std::string findCommonPrefix(const std::vector<std::vector<std::string>>& rhsList,
                                  size_t& prefixLen) const;
    bool startsWith(const std::vector<std::string>& rhs,
                    const std::string& symbol) const;
};

} // namespace PL0

#endif // PL0_GRAMMAR_NORMALIZER_HPP
