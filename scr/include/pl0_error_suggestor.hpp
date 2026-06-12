/**
 * @file pl0_error_suggestor.hpp
 * @brief PL/0 Error Suggestion — pattern-matched error diagnosis and fixes
 * @details Maintains a knowledge base of common PL/0 syntax/semantic errors
 *          and generates structured fix suggestions when errors are detected.
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_ERROR_SUGGESTOR_HPP
#define PL0_ERROR_SUGGESTOR_HPP

#include <string>
#include <vector>
#include <map>

namespace PL0 {

//============================================================================
// Error Fix Entry
//============================================================================

struct ErrorFix {
    std::string pattern;             // Substring to match in error message
    std::string description;         // Human-readable problem description
    std::vector<std::string> fixes;  // Suggested fixes (numbered list)
};

//============================================================================
// Error Suggestor
//============================================================================

class ErrorSuggestor {
public:
    ErrorSuggestor();

    // Get suggestions for an error message string
    std::vector<std::string> getSuggestions(const std::string& errorMsg) const;

    // Print suggestions in a formatted block
    void printSuggestions(std::ostream& os, const std::string& errorMsg) const;

private:
    std::vector<ErrorFix> knowledgeBase_;

    void initKnowledgeBase();
    bool matchPattern(const std::string& msg, const std::string& pattern) const;
};

} // namespace PL0

#endif // PL0_ERROR_SUGGESTOR_HPP
