/**
 * @file visualizer.cpp
 * @brief Visualization Output Implementation
 * @details Generates Graphviz DOT diagrams for NFA, DFA, minimized DFA,
 *          token classification tables, and lexical recognition flowcharts.
 */

#include "../include/pl0_visualizer.hpp"
#include "../dfa/nfa.h"
#include "../dfa/dfa.h"
#include <algorithm>
#include <iomanip>
#include <set>

namespace PL0 {

//============================================================================
// Helpers
//============================================================================

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

std::string Visualizer::charLabel(char c) {
    if (c == '\0') return "&epsilon;";  // epsilon in HTML entity
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
 * @brief Build a compact symbolic label from a set of characters.
 * @details Detects complete character classes to avoid listing 62
 *          individual characters per edge. Uses:
 *            - &Sigma; (Greek Sigma = "letter") for [a-zA-Z]
 *            - &delta; (Greek delta = "digit")  for [0-9]
 *          Falls back to listing individual characters for operators,
 *          delimiters, or incomplete subsets.
 */
static std::string compactLabel(const std::vector<char>& chars) {
    if (chars.empty()) return "";

    // Separate epsilon from visible characters
    bool hasEps = false;
    std::set<char> visible;
    for (char c : chars) {
        if (c == '\0') hasEps = true;
        else visible.insert(c);
    }

    // Count character categories
    int lowers = 0, uppers = 0, digits = 0;
    std::set<char> others;
    for (char c : visible) {
        if      (c >= 'a' && c <= 'z') lowers++;
        else if (c >= 'A' && c <= 'Z') uppers++;
        else if (c >= '0' && c <= '9') digits++;
        else                           others.insert(c);
    }

    // Build compact label parts
    std::vector<std::string> parts;

    if (lowers == 26) {
        parts.push_back("letter");       // complete [a-z]
    } else if (lowers > 0) {
        // Partial lower — list individually (rare for PL/0)
        for (char c : visible)
            if (c >= 'a' && c <= 'z')
                parts.push_back(std::string(1, c));
    }

    if (uppers == 26) {
        // If we already said "letter" for lowers, fold uppers into it
        // by replacing "letter" with combined label
        if (lowers == 26) {
            // Already full; nothing to add (letter covers a-zA-Z)
        } else {
            parts.push_back("letter");   // [A-Z] only
        }
    } else if (uppers > 0 && lowers < 26) {
        for (char c : visible)
            if (c >= 'A' && c <= 'Z')
                parts.push_back(std::string(1, c));
    }
    // If lowers==26 && uppers==26, "letter" already covers both — no change needed

    if (digits == 10) {
        parts.push_back("digit");        // complete [0-9]
    } else if (digits > 0) {
        for (char c : visible)
            if (c >= '0' && c <= '9')
                parts.push_back(std::string(1, c));
    }

    // Remaining individual characters (operators, delimiters, etc.)
    for (char c : others) {
        // Escape special DOT characters
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

    // Join with comma
    std::string label;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) label += ", ";
        label += parts[i];
    }

    // If we have epsilon + visible, note epsilon separately
    if (hasEps) {
        if (!label.empty()) label = "&epsilon;, " + label;
        else label = "&epsilon;";
    }

    return label;
}

bool Visualizer::writeToFile(const std::string& filename,
                             const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Visualizer: Cannot write to " << filename << "\n";
        return false;
    }
    file << content;
    file.close();
    std::cout << "Output written: " << filename << "\n";
    return true;
}

//============================================================================
// NFA DOT Generation
//============================================================================

std::string Visualizer::generateNFADOT(const NFA* nfa,
                                       const std::string& title) {
    if (!nfa || !nfa->start) return "";

    std::ostringstream oss;
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=LR;\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n";
    oss << "    node [shape=circle, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n\n";

    // Collect all states and assign display IDs
    std::map<const NFAState*, int> stateIds;
    int id = 0;
    for (const NFAState* s : nfa->states) {
        stateIds[s] = id++;
    }

    // Invisible start node pointing to start state
    oss << "    start [shape=point, width=0.2];\n";
    oss << "    start -> " << stateIds[nfa->start] << ";\n\n";

    // Declare nodes: accept states get double circle
    for (const NFAState* s : nfa->states) {
        oss << "    " << stateIds[s];
        if (s == nfa->accept) {
            oss << " [peripheries=2]";
        }
        oss << ";\n";
    }

    // Transitions — group by (from, to) with all symbols
    // NFA can have multiple symbols between same pair; merge into compact label
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
    for (const auto& edge : edgeLabels) {
        int from = edge.first.first;
        int to   = edge.first.second;
        const auto& syms = edge.second;

        // Build compact symbolic label (letter/digit/individual)
        std::string label = compactLabel(syms);
        bool hasEpsilon = (std::find(syms.begin(), syms.end(), '\0') != syms.end());

        oss << "    " << from << " -> " << to
            << " [label=\"" << label << "\"";
        if (hasEpsilon) {
            oss << ", style=dashed";
        }
        oss << "];\n";
    }

    oss << "}\n";
    return oss.str();
}

//============================================================================
// DFA DOT Generation
//============================================================================

std::string Visualizer::generateDFADOT(const DFA* dfa,
                                       const std::string& title) {
    if (!dfa || !dfa->startState) return "";

    std::ostringstream oss;
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=LR;\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n";
    oss << "    node [shape=circle, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n\n";

    // Map DFAState* to display IDs
    std::map<const DFAState*, int> stateIds;
    for (size_t i = 0; i < dfa->states.size(); ++i) {
        stateIds[dfa->states[i]] = static_cast<int>(i);
    }

    // Invisible start
    oss << "    start [shape=point, width=0.2];\n";
    oss << "    start -> " << stateIds[dfa->startState] << ";\n\n";

    // Declare nodes
    for (const DFAState* s : dfa->states) {
        oss << "    " << stateIds[s];
        if (s->isAccept) {
            oss << " [peripheries=2]";
        }
        oss << ";\n";
    }

    // Transitions — group by (from, to) with compact symbolic labels
    // DFA is deterministic: one next state per symbol, but many symbols
    // may share the same target. Group them into letter/digit classes.
    std::map<std::pair<int, int>, std::vector<char>> edgeGroups;
    for (const DFAState* s : dfa->states) {
        for (const auto& trans : s->transitions) {
            char sym = trans.first;
            const DFAState* target = trans.second;
            edgeGroups[{stateIds[s], stateIds[target]}].push_back(sym);
        }
    }

    oss << "\n";
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
// Token Classification DOT
//============================================================================

std::string Visualizer::generateClassificationDOT(
    const std::vector<Token>& tokens,
    const std::string& title) {

    LexerStats stats = computeStats(tokens);

    std::ostringstream oss;
    oss << "digraph " << dotEscape(title) << " {\n";
    oss << "    rankdir=TB;\n";
    oss << "    node [shape=box, fontsize=12];\n";
    oss << "    edge [fontsize=11];\n";
    oss << "    label=\"" << dotEscape(title) << "\";\n";
    oss << "    labelloc=t;\n";
    oss << "    fontsize=16;\n\n";

    oss << "    \"Token分类\" [shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // Category nodes
    auto addCat = [&](const char* name, int count) {
        oss << "    \"" << name << "\" [label=\"" << name << "\\n("
            << count << ")\"];\n";
    };

    addCat("关键字", stats.keywordCount);
    addCat("标识符", stats.identifierCount);
    addCat("数字",   stats.numberCount);
    addCat("运算符", stats.operatorCount);
    addCat("分隔符", stats.delimiterCount);

    if (stats.errorCount > 0) {
        addCat("错误", stats.errorCount);
    }

    oss << "\n";
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
// Lexical Recognition Flowchart DOT
//============================================================================

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

    // Node styles
    oss << "    // Decision diamonds\n";
    oss << "    node_d [shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    // Process boxes\n";
    oss << "    node_p [shape=box, style=rounded, fillcolor=lightblue, style=filled];\n";
    oss << "    // Terminal\n";
    oss << "    node_t [shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // Start
    oss << "    S_Start [label=\"开始\", shape=ellipse, style=filled, fillcolor=lightgray];\n\n";

    // Read char
    oss << "    S_Read [label=\"读取下一个字符\"];\n\n";

    // Whitespace decision
    oss << "    D_WS [label=\"空白字符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    S_SkipWS [label=\"跳过空白\"];\n\n";

    // EOF decision
    oss << "    D_EOF [label=\"文件结束?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    S_EOF [label=\"返回 EOF Token\"];\n\n";

    // Character type decision
    oss << "    D_Letter [label=\"字母?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Digit [label=\"数字?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Op [label=\"运算符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n";
    oss << "    D_Delim [label=\"分隔符?\", shape=diamond, style=filled, fillcolor=lightyellow];\n\n";

    // Actions
    oss << "    S_Ident [label=\"识别标识符/关键字\"];\n";
    oss << "    S_Number [label=\"识别数字\"];\n";
    oss << "    S_Operator [label=\"识别运算符\"];\n";
    oss << "    S_Delimiter [label=\"识别分隔符\"];\n";
    oss << "    S_Error [label=\"错误恢复\\n(记录并跳过)\", fillcolor=lightcoral];\n\n";

    // Output
    oss << "    S_Output [label=\"输出 Token\"];\n\n";

    // Edges
    oss << "    // Flow\n";
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

    // All actions lead to output
    oss << "    S_Ident -> S_Output;\n";
    oss << "    S_Number -> S_Output;\n";
    oss << "    S_Operator -> S_Output;\n";
    oss << "    S_Delimiter -> S_Output;\n";
    oss << "    S_Error -> S_Output;\n";
    oss << "    S_EOF -> S_Output;\n\n";

    // Loop back
    oss << "    S_Output -> S_Read [label=\"重复\"];\n";

    oss << "}\n";
    return oss.str();
}

//============================================================================
// Analysis Report
//============================================================================

std::string Visualizer::generateAnalysisReport(
    const std::vector<Token>& tokens,
    const LexerStats& stats,
    const std::string& sourceFile) {

    std::ostringstream oss;
    oss << "========================================\n";
    oss << "   PL/0 Lexical Analysis Report\n";
    oss << "========================================\n\n";
    oss << "Source file:  " << sourceFile << "\n";
    oss << "Generated:    " << __DATE__ << " " << __TIME__ << "\n";
    oss << "----------------------------------------\n\n";

    // Token count summary
    oss << "--- Token Statistics ---\n\n";
    oss << "  Keyword:    " << std::setw(5) << stats.keywordCount    << "\n";
    oss << "  Identifier: " << std::setw(5) << stats.identifierCount << "\n";
    oss << "  Number:     " << std::setw(5) << stats.numberCount     << "\n";
    oss << "  Operator:   " << std::setw(5) << stats.operatorCount   << "\n";
    oss << "  Delimiter:  " << std::setw(5) << stats.delimiterCount  << "\n";
    oss << "  Error:      " << std::setw(5) << stats.errorCount      << "\n";
    oss << "  -------------------------\n";
    oss << "  Total:      " << std::setw(5) << stats.totalTokens      << "\n\n";

    // Token detail table
    oss << "--- Token Detail ---\n\n";
    oss << std::left
        << std::setw(5)  << "No."
        << std::setw(13) << "Type"
        << std::setw(16) << "Value"
        << std::setw(7)  << "Line"
        << "\n";
    oss << std::string(5, '-') << " "
        << std::string(13, '-') << " "
        << std::string(16, '-') << " "
        << std::string(7, '-') << "\n";

    int index = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::END_OF_FILE) continue;

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

    // Error summary
    if (stats.errorCount > 0) {
        oss << "\n--- Lexical Errors ---\n\n";
        int errIdx = 0;
        for (const auto& t : tokens) {
            if (t.type == TokenType::ERROR) {
                oss << "  Error " << ++errIdx << ": Line " << t.line
                    << " - " << t.value << "\n";
            }
        }
        oss << "\n";
    }

    oss << "========================================\n";
    oss << "Analysis completed.\n";
    oss << "========================================\n";

    return oss.str();
}

//============================================================================
// Stats Computation
//============================================================================

LexerStats Visualizer::computeStats(const std::vector<Token>& tokens) {
    LexerStats s;
    for (const auto& t : tokens) {
        if (t.type == TokenType::END_OF_FILE) continue;
        switch (t.type) {
            case TokenType::KEYWORD:    s.keywordCount++;    break;
            case TokenType::IDENTIFIER: s.identifierCount++; break;
            case TokenType::NUMBER:     s.numberCount++;     break;
            case TokenType::OPERATOR:   s.operatorCount++;   break;
            case TokenType::DELIMITER:  s.delimiterCount++;  break;
            case TokenType::ERROR:      s.errorCount++;      break;
            default: break;
        }
        s.totalTokens++;
    }
    return s;
}

} // namespace PL0
