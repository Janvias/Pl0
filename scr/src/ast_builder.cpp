/**
 * @file ast_builder.cpp
 * @brief AST构建器实现 — 语法树构造与DOT生成
 * @details 使用父节点栈来跟踪递归下降解析过程中的嵌套关系。
 *          LL(1)解析器在适当的语法位置调用beginNode/endNode/addLeaf来增量构建AST。
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#include "../include/pl0_ast.hpp"
#include <iomanip>

namespace PL0 {

//============================================================================
// 构造函数 / 析构函数
//============================================================================

ASTBuilder::ASTBuilder() {
    reset();
}

ASTBuilder::~ASTBuilder() {}

void ASTBuilder::reset() {
    root_.reset();
    stack_.clear();
}

//============================================================================
// 节点构造
//============================================================================

void ASTBuilder::beginNode(ASTNodeType type, const std::string& value) {
    auto node = std::make_unique<ASTNode>(type, value);
    ASTNode* ptr = node.get();

    if (stack_.empty()) {
        // This is the root
        root_ = std::move(node);
    } else {
        // Add as child of current top
        stack_.back()->children.push_back(std::move(node));
    }
    stack_.push_back(ptr);
}

void ASTBuilder::endNode() {
    if (!stack_.empty()) {
        stack_.pop_back();
    }
}

void ASTBuilder::addLeaf(ASTNodeType type, const std::string& value) {
    if (stack_.empty()) return;
    stack_.back()->addChild(type, value);
}

//============================================================================
// 节点类型转字符串
//============================================================================

std::string ASTBuilder::nodeTypeToString(ASTNodeType t) {
    switch (t) {
        case ASTNodeType::PROGRAM:        return "PROGRAM";
        case ASTNodeType::BLOCK:          return "BLOCK";
        case ASTNodeType::CONST_DECL:     return "CONST_DECL";
        case ASTNodeType::VAR_DECL:       return "VAR_DECL";
        case ASTNodeType::PROC_DECL:      return "PROC_DECL";
        case ASTNodeType::ASSIGN_STMT:    return "ASSIGN";
        case ASTNodeType::IF_STMT:        return "IF";
        case ASTNodeType::WHILE_STMT:     return "WHILE";
        case ASTNodeType::CALL_STMT:      return "CALL";
        case ASTNodeType::READ_STMT:      return "READ";
        case ASTNodeType::WRITE_STMT:     return "WRITE";
        case ASTNodeType::BEGIN_END:      return "BEGIN_END";
        case ASTNodeType::CONDITION:      return "CONDITION";
        case ASTNodeType::BINOP:          return "BINOP";
        case ASTNodeType::UNOP:           return "UNOP";
        case ASTNodeType::IDENTIFIER:     return "ID";
        case ASTNodeType::NUMBER:         return "NUM";
        case ASTNodeType::STATEMENT_LIST: return "STMT_LIST";
        default: return "?";
    }
}

//============================================================================
// DOT生成（用于Graphviz可视化）
//============================================================================

std::string ASTBuilder::generateDOT(const std::string& graphName) const {
    if (!root_) return "";

    std::ostringstream ss;
    ss << "digraph " << graphName << " {\n";
    ss << "    rankdir=TB;\n";
    ss << "    node [shape=box, style=filled, fillcolor=lightblue, "
          "fontname=\"Consolas\"];\n";
    ss << "    edge [fontname=\"Consolas\"];\n\n";

    int id = 0;
    generateDOTNode(root_.get(), id, ss);

    ss << "}\n";
    return ss.str();
}

void ASTBuilder::generateDOTNode(const ASTNode* node, int& id,
                                  std::ostringstream& ss) const {
    int myId = id++;

    // Node label: TYPE [value]
    std::string label = nodeTypeToString(node->type);
    if (!node->value.empty()) {
        label += "\\n" + node->value;
    }

    // Color by node type
    const char* color = "lightblue";
    switch (node->type) {
        case ASTNodeType::PROGRAM:     color = "lightyellow"; break;
        case ASTNodeType::BLOCK:       color = "lightcyan"; break;
        case ASTNodeType::IF_STMT:
        case ASTNodeType::WHILE_STMT:  color = "lightcoral"; break;
        case ASTNodeType::ASSIGN_STMT: color = "lightgreen"; break;
        case ASTNodeType::BINOP:
        case ASTNodeType::UNOP:        color = "plum"; break;
        case ASTNodeType::IDENTIFIER:  color = "palegreen"; break;
        case ASTNodeType::NUMBER:      color = "wheat"; break;
        default: break;
    }

    ss << "    node" << myId << " [label=\"" << label
       << "\", fillcolor=" << color << "];\n";

    // Children
    for (const auto& child : node->children) {
        int childId = id;
        generateDOTNode(child.get(), id, ss);
        ss << "    node" << myId << " -> node" << childId << ";\n";
    }
}

//============================================================================
// 文本树转储（用于调试输出）
//============================================================================

void ASTBuilder::printTree(std::ostream& os) const {
    if (!root_) {
        os << "(empty AST)\n";
        return;
    }
    printTree(os, root_.get(), 0);
}

void ASTBuilder::printTree(std::ostream& os,
                            const ASTNode* node, int indent) const {
    os << std::string(indent * 2, ' ')
       << nodeTypeToString(node->type);
    if (!node->value.empty()) {
        os << " [" << node->value << "]";
    }
    os << "\n";

    for (const auto& child : node->children) {
        printTree(os, child.get(), indent + 1);
    }
}

} // namespace PL0
