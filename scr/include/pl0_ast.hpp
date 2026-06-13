/**
 * @file pl0_ast.hpp
 * @brief PL/0抽象语法树 — 节点类型、ASTBuilder、DOT生成器
 * @details 在LL(1)递归下降解析过程中构建AST，
 *          并生成Graphviz DOT格式用于可视化
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
// AST节点类型
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
// AST节点结构
//============================================================================

struct ASTNode {
    ASTNodeType type;                              // 节点类型
    std::string value;                             // 标识符名/数字值/操作符
    std::vector<std::unique_ptr<ASTNode>> children;  // 子节点列表

    ASTNode(ASTNodeType t) : type(t) {}
    ASTNode(ASTNodeType t, const std::string& v) : type(t), value(v) {}

    // 添加子节点（无值）
    ASTNode* addChild(ASTNodeType t) {
        auto node = std::make_unique<ASTNode>(t);
        ASTNode* ptr = node.get();
        children.push_back(std::move(node));
        return ptr;
    }

    // 添加子节点（带值）
    ASTNode* addChild(ASTNodeType t, const std::string& v) {
        auto node = std::make_unique<ASTNode>(t, v);
        ASTNode* ptr = node.get();
        children.push_back(std::move(node));
        return ptr;
    }
};

//============================================================================
// AST构建器
//============================================================================

class ASTBuilder {
public:
    ASTBuilder();
    ~ASTBuilder();

    // 重置（用于新的解析）
    void reset();

    // 根节点访问
    ASTNode* root() { return root_.get(); }

    // 基于栈的节点构造（匹配递归下降嵌套）
    void beginNode(ASTNodeType type, const std::string& value = "");  // 开始节点
    void endNode();                                                   // 结束节点
    void addLeaf(ASTNodeType type, const std::string& value = "");    // 添加叶子节点

    // DOT生成（用于Graphviz）
    std::string generateDOT(const std::string& graphName = "AST") const;

    // 文本树输出
    void printTree(std::ostream& os) const;
    void printTree(std::ostream& os, const ASTNode* node, int indent) const;

private:
    std::unique_ptr<ASTNode> root_;       // 根节点
    std::vector<ASTNode*> stack_;         // 当前嵌套栈

    static std::string nodeTypeToString(ASTNodeType t);  // 节点类型转字符串
    void generateDOTNode(const ASTNode* node, int& id,
                         std::ostringstream& ss) const;  // 生成DOT节点
};

} // namespace PL0

#endif // PL0_AST_HPP
