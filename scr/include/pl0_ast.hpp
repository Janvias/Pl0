/**
 * @file pl0_ast.hpp
 * @brief PL/0 Abstract Syntax Tree — Node types, ASTBuilder, DOT generator
 * @details Builds an AST during LL(1) recursive-descent parsing and
 *          generates Graphviz DOT format for visualization.
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_AST_HPP
#define PL0_AST_HPP

#include "pl0_core.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <memory>

namespace PL0 {

//============================================================================
// AST Node Types
//============================================================================

enum class ASTNodeType {
    PROGRAM,        // 程序根节点
    BLOCK,          // 分程序
    CONST_DECL,     // 常量声明
    VAR_DECL,       // 变量声明
    PROC_DECL,      // 过程声明
    ASSIGN_STMT,    // 赋值语句
    IF_STMT,        // if语句
    WHILE_STMT,     // while语句
    CALL_STMT,      // call语句
    READ_STMT,      // read语句
    WRITE_STMT,     // write语句
    BEGIN_END,      // begin-end复合语句
    CONDITION,      // 条件表达式
    BINOP,          // 二元运算
    UNOP,           // 一元运算
    IDENTIFIER,     // 标识符
    NUMBER,         // 数字
    STATEMENT_LIST  // 语句列表
};

//============================================================================
// AST Node
//============================================================================

struct ASTNode {
    ASTNodeType type;
    std::string value;                    // 标识符名 / 数字值 / 操作符
    std::vector<std::unique_ptr<ASTNode>> children;

    ASTNode(ASTNodeType t) : type(t) {}
    ASTNode(ASTNodeType t, const std::string& v) : type(t), value(v) {}

    ASTNode* addChild(ASTNodeType t) {
        auto node = std::make_unique<ASTNode>(t);
        ASTNode* ptr = node.get();
        children.push_back(std::move(node));
        return ptr;
    }

    ASTNode* addChild(ASTNodeType t, const std::string& v) {
        auto node = std::make_unique<ASTNode>(t, v);
        ASTNode* ptr = node.get();
        children.push_back(std::move(node));
        return ptr;
    }
};

//============================================================================
// AST Builder
//============================================================================

class ASTBuilder {
public:
    ASTBuilder();
    ~ASTBuilder();

    // Reset for a new parse
    void reset();

    // Root access
    ASTNode* root() { return root_.get(); }

    // Stack-based node construction (matches recursive-descent nesting)
    void beginNode(ASTNodeType type, const std::string& value = "");
    void endNode();
    void addLeaf(ASTNodeType type, const std::string& value = "");

    // DOT generation for Graphviz
    std::string generateDOT(const std::string& graphName = "AST") const;

    // Textual tree dump
    void printTree(std::ostream& os) const;
    void printTree(std::ostream& os, const ASTNode* node, int indent) const;

private:
    std::unique_ptr<ASTNode> root_;
    std::vector<ASTNode*> stack_;  // current nesting stack

    static std::string nodeTypeToString(ASTNodeType t);
    void generateDOTNode(const ASTNode* node, int& id,
                         std::ostringstream& ss) const;
};

} // namespace PL0

#endif // PL0_AST_HPP
