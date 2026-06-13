/**
 * @file pl0_error_suggestor.hpp
 * @brief PL/0错误建议器 — 模式匹配的错误诊断和修复建议
 * @details 维护常见PL/0语法/语义错误的知识库，
 *          在检测到错误时生成结构化的修复建议
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
// 错误修复条目
//============================================================================

struct ErrorFix {
    std::string pattern;             // 错误消息中匹配的子串
    std::string description;         // 问题描述（人类可读）
    std::vector<std::string> fixes;  // 修复建议列表（编号）
};

//============================================================================
// 错误建议器
//============================================================================

class ErrorSuggestor {
public:
    ErrorSuggestor();

    // 获取错误消息的建议
    std::vector<std::string> getSuggestions(const std::string& errorMsg) const;

    // 格式化输出建议
    void printSuggestions(std::ostream& os, const std::string& errorMsg) const;

private:
    std::vector<ErrorFix> knowledgeBase_;  // 知识库

    void initKnowledgeBase();  // 初始化知识库
    bool matchPattern(const std::string& msg, const std::string& pattern) const;  // 模式匹配
};

} // namespace PL0

#endif // PL0_ERROR_SUGGESTOR_HPP
