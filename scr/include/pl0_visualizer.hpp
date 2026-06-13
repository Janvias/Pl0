/**
 * @file pl0_visualizer.hpp
 * @brief 可视化输出控制器
 * @details 生成Graphviz DOT格式的NFA、DFA、最小化DFA、
 *          Token分类和词法识别流程图，以及文本分析报告
 * @author PL/0 Compiler Project
 * @date 2026-06-12
 */

#ifndef PL0_VISUALIZER_HPP
#define PL0_VISUALIZER_HPP

#include "pl0_core.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <set>
#include <map>

// 前向声明（全局命名空间中的dfa/类型）
struct NFAState;
class NFA;
struct DFAState;
class DFA;

namespace PL0 {

//============================================================================
// 词法分析统计信息
//============================================================================

struct LexerStats {
    int keywordCount    = 0;    // 关键字数量
    int identifierCount = 0;    // 标识符数量
    int numberCount     = 0;    // 数字数量
    int operatorCount   = 0;    // 运算符数量
    int delimiterCount  = 0;    // 分隔符数量
    int errorCount      = 0;    // 错误数量
    int totalTokens     = 0;    // Token总数
};

//============================================================================
// 可视化器
//============================================================================

class Visualizer {
public:
    // --- 自动机可视化 ---

    /** 生成NFA状态转换图的DOT格式 */
    static std::string generateNFADOT(const NFA* nfa,
                                      const std::string& title = "NFA");

    /** 生成DFA状态转换图的DOT格式 */
    static std::string generateDFADOT(const DFA* dfa,
                                      const std::string& title = "DFA");

    // --- Token可视化 ---

    /** 生成Token分类树的DOT格式 */
    static std::string generateClassificationDOT(
        const std::vector<Token>& tokens,
        const std::string& title = "TokenClassification");

    /** 生成词法识别流程图的DOT格式 */
    static std::string generateRecognitionFlowchartDOT(
        const std::string& title = "LexicalRecognitionFlowchart");

    // --- 报告生成 ---

    /** 生成纯文本词法分析报告 */
    static std::string generateAnalysisReport(
        const std::vector<Token>& tokens,
        const LexerStats& stats,
        const std::string& sourceFile);

    /** 从Token向量计算统计信息 */
    static LexerStats computeStats(const std::vector<Token>& tokens);

    // --- 文件I/O辅助函数 ---

    /** 将字符串内容写入文件，返回成功状态 */
    static bool writeToFile(const std::string& filename,
                            const std::string& content);

private:
    /** 对字符串进行DOT标签安全转义 */
    static std::string dotEscape(const std::string& s);

    /** 对单个字符进行DOT边标签转义 */
    static std::string charLabel(char c);
};

} // namespace PL0

#endif // PL0_VISUALIZER_HPP
