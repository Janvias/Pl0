/**
 * @file visualizer.cpp
 * @brief PL/0 编译器可视化输出实现
 * @details 生成 Graphviz DOT 格式图表，包括：
 *          - NFA（非确定有限自动机）
 *          - DFA（确定有限自动机）
 *          - 最小化 DFA
 *          - Token 分类表格
 *          - 词法识别流程图
 */

#include "../include/pl0_visualizer.hpp"
#include "../dfa/nfa.h"
#include "../dfa/dfa.h"
#include <algorithm>
#include <iomanip>
#include <set>

namespace PL0 {

//============================================================================
// 辅助函数 (Helper Functions)
//============================================================================

/**
 * @brief DOT 字符串转义函数
 * @details 对字符串进行 DOT 格式转义处理，避免特殊字符导致 DOT 图描述语法错误
 * @param s 待转义的原始字符串
 * @return 转义后的字符串，可安全用于 DOT 图的标签
 * 
 * 转义规则：
 * - 双引号 (") → \" 
 * - 反斜杠 (\) → \\
 * - 换行符 (\n) → \n
 * - 制表符 (\t) → \t
 */
std::string Visualizer::dotEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\";  break;
            case '\n': out += "\\n";   break;
            case '\t': out += "\\t";   break;
            default:   out += c;       break;
        }
    }
    return out;
}

/**
 * @brief 单个字符的 DOT 标签转换
 * @param c 待转换的字符
 * @return DOT 格式的字符标签表示
 * 
 * 特殊字符处理：
 * - 空字符 '\0' → ε (epsilon)
 * - DOT 特殊字符进行转义
 */
std::string Visualizer::charLabel(char c) {
    if (c == '\0') return "&epsilon;";  // epsilon 使用 HTML 实体表示
    switch (c) {
        case '"':  return "\\\"";
        case '\\': return "\\\\";
        case '|':  return "\\|";
        case '{':  return "\\{";
        case '}':  return "\\}";
        case '<':  return "\\<";
        case '>':  return "\\>";
        default:   return std::string(1, c);
    }
}

/**
 * @brief 构建紧凑的符号化字符集标签
 * @details 自动检测完整字符类别，避免在边上列出大量单个字符。使用符号表示：
 *          - letter (Σ) 表示 [a-zA-Z] 完整字母集
 *          - digit (δ) 表示 [0-9] 完整数字集
 *          对于运算符、分隔符或不完整的子集，回退到列出单个字符
 * @param chars 字符集合
 * @return 紧凑的 DOT 标签字符串
 */
static std::string compactLabel(const std::vector<char>& chars) {
    if (chars.empty()) return "";

    // 分离 epsilon 和可见字符
    bool hasEps = false;
    std::set<char> visible;
    for (char c : chars) {
        if (c == '\0') hasEps = true;
        else visible.insert(c);
    }

    // 统计各字符类别数量
    int lowers = 0, uppers = 0, digits = 0;
    std::set<char> others;
    for (char c : visible) {
        if      (c >= 'a' && c <= 'z') lowers++;
        else if (c >= 'A' && c <= 'Z') uppers++;
        else if (c >= '0' && c <= '9') digits++;
        else                           others.insert(c);
    }

    // 构建紧凑标签部分
    std::vector<std::string> parts;

    // 处理小写字母
    if (lowers == 26) {
        parts.push_back("letter");       // 完整的 [a-z]
    } else if (lowers > 0) {
        // 部分小写字母 - 单独列出（PL/0 中很少见）
        for (char c : visible)
            if (c >= 'a' && c <= 'z')
                parts.push_back(std::string(1, c));
    }

    // 处理大写字母
    if (uppers == 26) {
        // 如果小写已标记为 "letter"，大写也包含在内
        if (lowers == 26) {
            // 已经是完整的 letter，无需额外添加
        } else {
            parts.push_back("letter");   // 仅 [A-Z]
        }
    } else if (uppers > 0 && lowers < 26) {
        for (char c : visible)
            if (c >= 'A' && c <= 'Z')
                parts.push_back(std::string(1, c));
    }
    // 如果 lowers==26 && uppers==26，"letter" 已覆盖两者

    // 处理数字
    if (digits == 10) {
        parts.push_back("digit");        // 完整的 [0-9]
    } else if (digits > 0) {
        for (char c : visible)
            if (c >= '0' && c <= '9')
                parts.push_back(std::string(1, c));
    }

    // 剩余单个字符（运算符、分隔符等）
    for (char c : others) {
        // 转义 DOT 特殊字符
        switch (c) {
            case '"': parts.push_back("\\\""); break;
            case '\\': parts.push_back("\\\\"); break;
            case '|': parts.push_back("\\|"); break;
            case '<': parts.push_back("\\<"); break;
            case '>': parts.push_back("\\>"); break;
            case '{': parts.push_back("\\{"); break;
            case '}': parts.push_back("\\}"); break;
            default:  parts.push_back(std::string(1, c)); break;
        }
    }

    // 使用逗号连接各部分
    std::string label;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) label += ", ";
        label += parts[i];
    }

    // 如果包含 epsilon，单独标注
    if (hasEps) {
        if (!label.empty()) label = "&epsilon;, " + label;
        else label = "&epsilon;";
    }

    return label;
}

/**
 * @brief 将内容写入文件
 * @param filename 目标文件名
 * @param content 要写入的内容
 * @return 写入是否成功
 */
bool Visualizer::writeToFile(const std::string& filename,
                             const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Visualizer: 无法写入文件 " << filename << "\n";
        return false;
    }
    file << content;
    file.close();
    std::cout << "输出已写入: " << filename << "\n";
    return true;
}
        
//============================================================================
// NFA DOT 图生成 (NFA DOT Generation)
//============================================================================

/**
 * @brief 生成 NFA 的 Graphviz DOT 描述
 * @details 将 NFA 转换为 DOT 格式的有向图，支持：
 *          - 状态节点显示（开始状态、接受状态）
 *          - ε 转换（使用虚线表示）
 *          - 字符转换边（使用紧凑符号标签）
 * @param nfa 指向 NFA 对象的指针
 * @param title 图的标题
 * @return DOT 格式的字符串，可直接用于 Graphviz 渲染
 */
std::string Visualizer::generateNFADOT(const NFA* nfa,
                                       const std::string& title) {
    if (!nfa || !nfa->start) return "";

    std::ostringstream oss;
    // DOT 图头部分
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=LR;\n";                    // 从左到右布局
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";                    // 标题在顶部
    oss << "    fontsize=16;\n";
    oss << "    node [shape=circle, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n\n";

    // 收集所有状态并分配显示 ID
    std::map<const NFAState*, int> stateIds;
    int id = 0;
    for (const NFAState* s : nfa->states) {
        stateIds[s] = id++;
    }

    // 不可见的起始节点，指向 NFA 起始状态
    oss << "    start [shape=point, width=0.2];\n";
    oss << "    start -> " << stateIds[nfa->start] << ";\n\n";

    // 声明状态节点：接受状态使用双圆圈
    for (const NFAState* s : nfa->states) {
        oss << "    " << stateIds[s];
        if (s == nfa->accept) {
            oss << " [peripheries=2]";  // 双圆圈表示接受状态
        }
        oss << ";\n";
    }

    // 转换边 - 按 (起点, 终点) 分组，合并相同符号
    // NFA 可能在同一对状态间有多个符号，合并为紧凑标签
    std::map<std::pair<int, int>, std::vector<char>> edgeLabels;
    for (const NFAState* s : nfa->states) {
        for (const auto& trans : s->transitions) {
            char sym = trans.first;
            for (const NFAState* target : trans.second) {
                edgeLabels[{stateIds[s], stateIds[target]}].push_back(sym);
            }
        }
    }

    oss << "\n";
    // 生成边描述
    for (const auto& edge : edgeLabels) {
        int from = edge.first.first;
        int to   = edge.first.second;
        const auto& syms = edge.second;

        // 构建紧凑符号标签
        std::string label = compactLabel(syms);
        bool hasEpsilon = (std::find(syms.begin(), syms.end(), '\0') != syms.end());

        oss << "    " << from << " -> " << to
            << " [label=\"" << label << "\"";
        if (hasEpsilon) {
            oss << ", style=dashed";  // ε 转换使用虚线
        }
        oss << "];\n";
    }

    oss << "}\n";
    return oss.str();
}

//============================================================================
// DFA DOT 图生成 (DFA DOT Generation)
//============================================================================

/**
 * @brief 生成 DFA 的 Graphviz DOT 描述
 * @details 将 DFA 转换为 DOT 格式的有向图
 * @param dfa 指向 DFA 对象的指针
 * @param title 图的标题
 * @return DOT 格式的字符串
 */
std::string Visualizer::generateDFADOT(const DFA* dfa,
                                       const std::string& title) {
    if (!dfa || !dfa->startState) return "";

    std::ostringstream oss;
    // DOT 图头部分
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=LR;\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n";
    oss << "    node [shape=circle, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n\n";

    // 映射 DFAState* 到显示 ID
    std::map<const DFAState*, int> stateIds;
    for (size_t i = 0; i < dfa->states.size(); ++i) {
        stateIds[dfa->states[i]] = static_cast<int>(i);
    }

    // 不可见的起始节点
    oss << "    start [shape=point, width=0.2];\n";
    oss << "    start -> " << stateIds[dfa->startState] << ";\n\n";

    // 声明状态节点
    for (const DFAState* s : dfa->states) {
        oss << "    " << stateIds[s];
        if (s->isAccept) {
            oss << " [peripheries=2]";  // 接受状态双圆圈
        }
        oss << ";\n";
    }

    // 转换边 - 按 (起点, 终点) 分组
    // DFA 是确定的：每个符号只有一个后继状态
    // 但可能有多个符号共享同一后继状态，合并为 letter/digit 类别
    std::map<std::pair<int, int>, std::vector<char>> edgeGroups;
    for (const DFAState* s : dfa->states) {
        for (const auto& trans : s->transitions) {
            char sym = trans.first;
            const DFAState* target = trans.second;
            edgeGroups[{stateIds[s], stateIds[target]}].push_back(sym);
        }
    }

    oss << "\n";
    // 生成边描述
    for (const auto& edge : edgeGroups) {
        int from = edge.first.first;
        int to   = edge.first.second;
        std::string label = compactLabel(edge.second);
        oss << "    " << from << " -> " << to
            << " [label=\"" << label << "\"];\n";
    }

    oss << "}\n";
    return oss.str();
}

//============================================================================
// Token 分类 DOT 图生成 (Token Classification DOT)
//============================================================================

/**
 * @brief 生成 Token 分类统计图
 * @details 创建饼图或树状图，展示词法分析结果的 Token 分布
 * @param tokens Token 序列
 * @param title 图的标题
 * @return DOT 格式的分类统计图
 */
std::string Visualizer::generateClassificationDOT(
    const std::vector<Token>& tokens,
    const std::string& title) {

    // 计算统计信息
    LexerStats stats = computeStats(tokens);

    std::ostringstream oss;
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=TB;\n";                    // 从上到下布局
    oss << "    node [shape=box, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n\n";

    // 根节点 - Token 分类
    oss << "    \"Token分类\" [shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // 分类节点定义
    auto addCat = [&](const char* name, int count) {
        oss << "    \"" << name << "\" [label=\"" << name << "\\n("
            << count << ")\"];\n";
    };

    // 添加各分类节点
    addCat("关键字", stats.keywordCount);
    addCat("标识符", stats.identifierCount);
    addCat("数字",   stats.numberCount);
    addCat("运算符", stats.operatorCount);
    addCat("分隔符", stats.delimiterCount);

    // 如果有错误，添加错误节点（红色高亮）
    if (stats.errorCount > 0) {
        addCat("错误", stats.errorCount);
    }

    oss << "\n";
    // 生成边连接
    oss << "    \"Token分类\" -> \"关键字\" [label=\"" << stats.keywordCount << "\"];\n";
    oss << "    \"Token分类\" -> \"标识符\" [label=\"" << stats.identifierCount << "\"];\n";
    oss << "    \"Token分类\" -> \"数字\"   [label=\"" << stats.numberCount << "\"];\n";
    oss << "    \"Token分类\" -> \"运算符\" [label=\"" << stats.operatorCount << "\"];\n";
    oss << "    \"Token分类\" -> \"分隔符\" [label=\"" << stats.delimiterCount << "\"];\n";
    if (stats.errorCount > 0) {
        oss << "    \"Token分类\" -> \"错误\"   [label=\"" << stats.errorCount
            << "\", color=red, fontcolor=red];\n";
    }

    oss << "}\n";
    return oss.str();
}

//============================================================================
// 词法识别流程图 DOT 生成 (Lexical Recognition Flowchart DOT)
//============================================================================

/**
 * @brief 生成词法识别流程图
 * @details 以流程图形式展示词法分析器的工作流程，包括：
 *          - 开始/结束
 *          - 字符读取
 *          - 空白跳过
 *          - EOF 检测
 *          - 字符类型判断（字母、数字、运算符、分隔符）
 *          - Token 输出
 * @param title 图的标题
 * @return DOT 格式的流程图
 */
std::string Visualizer::generateRecognitionFlowchartDOT(
    const std::string& title) {

    std::ostringstream oss;
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=TB;\n";
    oss << "    node [shape=box, fontsize=11, style=rounded];\n";
    oss << "    edge [fontsize=10];\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n\n";

    // 节点样式定义
    oss << "    // 决策菱形\n";
    oss << "    node_d [shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    // 处理矩形\n";
    oss << "    node_p [shape=box, style=rounded, fillcolor=lightblue, style=filled];\n";
    oss << "    // 终端椭圆\n";
    oss << "    node_t [shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // 开始节点
    oss << "    S_Start [label=\"开始\", shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // 读取字符
    oss << "    S_Read [label=\"读取下一个字符\"];\n\n";

    // 空白字符判断
    oss << "    D_WS [label=\"空白字符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    S_SkipWS [label=\"跳过空白\"];\n\n";

    // EOF 判断
    oss << "    D_EOF [label=\"文件结束?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    S_EOF [label=\"返回 EOF Token\"];\n\n";

    // 字符类型判断
    oss << "    D_Letter [label=\"字母?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Digit [label=\"数字?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Op [label=\"运算符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Delim [label=\"分隔符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n\n";

    // 处理动作
    oss << "    S_Ident [label=\"识别标识符/关键字\"];\n";
    oss << "    S_Number [label=\"识别数字\"];\n";
    oss << "    S_Operator [label=\"识别运算符\"];\n";
    oss << "    S_Delimiter [label=\"识别分隔符\"];\n";
    oss << "    S_Error [label=\"错误恢复\\n(记录并跳过)\", fillcolor=lightcoral];\n\n";

    // 输出
    oss << "    S_Output [label=\"输出 Token\"];\n\n";

    // 边连接
    oss << "    // 流程连接\n";
    oss << "    S_Start -> S_Read;\n";
    oss << "    S_Read -> D_WS;\n\n";

    oss << "    D_WS -> S_SkipWS [label=\"是\"];\n";
    oss << "    S_SkipWS -> S_Read;\n";
    oss << "    D_WS -> D_EOF [label=\"否\"];\n\n";

    oss << "    D_EOF -> S_EOF [label=\"是\"];\n";
    oss << "    D_EOF -> D_Letter [label=\"否\"];\n\n";

    oss << "    D_Letter -> S_Ident [label=\"是\"];\n";
    oss << "    D_Letter -> D_Digit [label=\"否\"];\n\n";

    oss << "    D_Digit -> S_Number [label=\"是\"];\n";
    oss << "    D_Digit -> D_Op [label=\"否\"];\n\n";

    oss << "    D_Op -> S_Operator [label=\"是\"];\n";
    oss << "    D_Op -> D_Delim [label=\"否\"];\n\n";

    oss << "    D_Delim -> S_Delimiter [label=\"是\"];\n";
    oss << "    D_Delim -> S_Error [label=\"否\"];\n\n";

    // 所有处理动作都指向输出
    oss << "    S_Ident -> S_Output;\n";
    oss << "    S_Number -> S_Output;\n";
    oss << "    S_Operator -> S_Output;\n";
    oss << "    S_Delimiter -> S_Output;\n";
    oss << "    S_Error -> S_Output;\n";
    oss << "    S_EOF -> S_Output;\n\n";

    // 循环回到读取
    oss << "    S_Output -> S_Read [label=\"重复\"];\n";

    oss << "}\n";
    return oss.str();
}

//============================================================================
// 分析报告生成 (Analysis Report)
//============================================================================

/**
 * @brief 生成词法分析报告
 * @details 生成文本格式的分析报告，包含：
 *          - Token 统计摘要
 *          - Token 详细信息表格
 *          - 词法错误汇总
 * @param tokens Token 序列
 * @param stats 统计信息
 * @param sourceFile 源代码文件名
 * @return 格式化的文本报告
 */
std::string Visualizer::generateAnalysisReport(
    const std::vector<Token>& tokens,
    const LexerStats& stats,
    const std::string& sourceFile) {

    std::ostringstream oss;
    // 报告头部
    oss << "========================================\n";
    oss << "   PL/0 词法分析报告\n";
    oss << "========================================\n\n";
    oss << "源文件:      " << sourceFile << "\n";
    oss << "生成时间:    " << __DATE__ << " " << __TIME__ << "\n";
    oss << "----------------------------------------\n\n";

    // Token 数量统计
    oss << "--- Token 统计 ---\n\n";
    oss << "  关键字:    " << std::setw(5) << stats.keywordCount    << "\n";
    oss << "  标识符:    " << std::setw(5) << stats.identifierCount << "\n";
    oss << "  数字:      " << std::setw(5) << stats.numberCount     << "\n";
    oss << "  运算符:    " << std::setw(5) << stats.operatorCount   << "\n";
    oss << "  分隔符:    " << std::setw(5) << stats.delimiterCount  << "\n";
    oss << "  错误:      " << std::setw(5) << stats.errorCount      << "\n";
    oss << "  -------------------------\n";
    oss << "  总计:      " << std::setw(5) << stats.totalTokens      << "\n\n";

    // Token 详细信息表格
    oss << "--- Token 详情 ---\n\n";
    oss << std::left
        << std::setw(5)  << "序号"
        << std::setw(13) << "类型"
        << std::setw(16) << "值"
        << std::setw(7)  << "行号"
        << "\n";
    oss << std::string(5, '-') << " "
        << std::string(13, '-') << " "
        << std::string(16, '-') << " "
        << std::string(7, '-') << "\n";

    int index = 0;
    for (const auto& t : tokens) {
        // 跳过 EOF
        if (t.type == TokenType::END_OF_FILE) continue;

        // 类型映射
        std::string typeStr;
        switch (t.type) {
            case TokenType::KEYWORD:    typeStr = "KEYWORD";     break;
            case TokenType::IDENTIFIER: typeStr = "IDENTIFIER";  break;
            case TokenType::NUMBER:     typeStr = "NUMBER";      break;
            case TokenType::OPERATOR:   typeStr = "OPERATOR";    break;
            case TokenType::DELIMITER:  typeStr = "DELIMITER";   break;
            case TokenType::ERROR:      typeStr = "ERROR";       break;
            default:                    typeStr = "UNKNOWN";     break;
        }

        oss << std::left
            << std::setw(5)  << index++
            << std::setw(13) << typeStr
            << std::setw(16) << t.value
            << std::setw(7)  << t.line
            << "\n";
    }

    // 错误汇总
    if (stats.errorCount > 0) {
        oss << "\n--- 词法错误汇总 ---\n\n";
        int errIdx = 0;
        for (const auto& t : tokens) {
            if (t.type == TokenType::ERROR) {
                oss << "  错误 " << ++errIdx << ": 第 " << t.line << " 行"
                    << " - " << t.value << "\n";
            }
        }
        oss << "\n";
    }

    // 报告尾部
    oss << "========================================\n";
    oss << "分析完成。\n";
    oss << "========================================\n";

    return oss.str();
}

//============================================================================
// 统计计算 (Stats Computation)
//============================================================================

/**
 * @brief 计算 Token 统计信息
 * @details 遍历 Token 序列，统计各类型 Token 的数量
 * @param tokens Token 序列
 * @return 包含各类别计数和总数的统计结构体
 */
LexerStats Visualizer::computeStats(const std::vector<Token>& tokens) {
    LexerStats s;
    for (const auto& t : tokens) {
        // 跳过 EOF
        if (t.type == TokenType::END_OF_FILE) continue;
        // 根据类型递增计数
        switch (t.type) {
            case TokenType::KEYWORD:    s.keywordCount++;    break;
            case TokenType::IDENTIFIER: s.identifierCount++; break;
            case TokenType::NUMBER:     s.numberCount++;     break;
            case TokenType::OPERATOR:   s.operatorCount++;   break;
            case TokenType::DELIMITER:  s.delimiterCount++;  break;
            case TokenType::ERROR:      s.errorCount++;     break;
            default: break;
        }
        // 总数递增
        s.totalTokens++;
    }
    return s;
}

} // namespace PL0
