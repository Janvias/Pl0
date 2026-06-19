/**
 * @file pl0_compiler.hpp
 * @brief PL/0编译器主类
 * @details 协调词法分析、双模式语法分析(LL(1) + LR(1))、
 *          语义分析和代码生成。支持AST可视化、错误建议和
 *          语法规范化等扩展功能。
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef PL0_COMPILER_HPP
#define PL0_COMPILER_HPP

#include "pl0_core.hpp"
#include "pl0_lexer.hpp"
#include "pl0_symtable.hpp"
#include "pl0_codegen.hpp"
#include "pl0_parser.hpp"
#include "pl0_lr1_parser.hpp"
#include "pl0_ast.hpp"
#include "pl0_error_suggestor.hpp"
#include "pl0_grammar_normalizer.hpp"
#include "pl0_visualizer.hpp"

namespace PL0 {

/**
 * @class Compiler
 * @brief PL/0编译器主控制器
 * @details Compiler类整合所有编译器组件，提供统一的编译接口，
 *          支持LL(1)和LR(1)双解析器验证、AST构建和可视化输出
 */
class Compiler {
public:
    Compiler();
    ~Compiler();

    //============================================================================
    // 编译操作
    //============================================================================

    /** 编译源文件 */
    bool compile(const std::string& inputFile);

    //============================================================================
    // 解析器模式设置
    //============================================================================

    /** 设置解析器模式 */
    void setParserMode(ParserMode mode) { parserMode_ = mode; }
    ParserMode getParserMode() const { return parserMode_; }
    const ValidationResult& getValidationResult() const { return validationResult_; }

    //============================================================================
    // 扩展功能开关
    //============================================================================

    /** 启用/禁用AST构建 */
    void enableAST(bool enable)     { astEnabled_ = enable; }
    /** 启用/禁用错误建议 */
    void enableSuggest(bool enable) { suggestEnabled_ = enable; }
    /** 启用/禁用语法规范化 */
    void enableNormalize(bool enable) { normalizeEnabled_ = enable; }

    bool isASTEnabled()     const { return astEnabled_; }
    bool isSuggestEnabled() const { return suggestEnabled_; }
    bool isNormalizeEnabled() const { return normalizeEnabled_; }

    //============================================================================
    // AST输出
    //============================================================================

    /** 获取AST构建器 */
    const ASTBuilder* getAST() const { return astBuilder_.get(); }
    /** 获取AST的DOT格式字符串 */
    std::string getASTDOT() const;

    //============================================================================
    // 语法规范化
    //============================================================================

    /** 获取规范化后的语法（基于46条LR(1)产生式） */
    std::string getNormalizedGrammar() const;

    //============================================================================
    // 错误建议
    //============================================================================

    /** 获取最近错误的修复建议 */
    std::string getErrorSuggestions() const;

    //============================================================================
    // 输出控制
    //============================================================================

    /** 设置详细输出模式 */
    void setVerbose(bool verbose) { verbose_ = verbose; }
    /** 打印编译结果 */
    void printResults(std::ostream& os = std::cout);

    //============================================================================
    // 文件输出
    //============================================================================

    /** 输出词法分析报告 */
    bool writeLexReport(const std::string& filename);
    /** 输出四元式报告 */
    bool writeQuadReport(const std::string& filename);
    /** 输出缓存文件 */
    bool writeCache(const std::string& filename);
    /** 输出AST DOT文件 */
    bool writeASTDOT(const std::string& filename);

    //============================================================================
    // DFA可视化输出（按需构建真实DFA）
    //============================================================================

    /** 输出NFA状态图 */
    bool dumpNFA(const std::string& filename);
    /** 输出DFA状态图 */
    bool dumpDFA(const std::string& filename);
    /** 输出最小化DFA状态图 */
    bool dumpMinDFA(const std::string& filename);
    /** 输出Token分类图 */
    bool dumpClassification(const std::string& filename);
    /** 输出词法分析报告 */
    bool dumpReport(const std::string& filename);
    /** 输出所有可视化文件（使用给定前缀） */
    bool dumpAll(const std::string& prefix);

    //============================================================================
    // 状态查询
    //============================================================================

    /** 检查编译是否成功 */
    bool isSuccess() const { return success_; }
    /** 获取错误消息 */
    const std::string& getErrorMessage() const { return errorMessage_; }

private:
    std::string inputFile_;
    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<SymbolTable> symTable_;
    std::unique_ptr<CodeGenerator> codeGen_;
    std::unique_ptr<Parser> ll1Parser_;
    std::unique_ptr<LR1Parser> lr1Parser_;

    // 扩展功能组件
    std::unique_ptr<ASTBuilder> astBuilder_;
    ErrorSuggestor errorSuggestor_;
    GrammarNormalizer grammarNormalizer_;

    ParserMode parserMode_;
    bool verbose_;
    bool success_;
    std::string errorMessage_;
    ValidationResult validationResult_;

    // 功能开关
    bool astEnabled_;
    bool suggestEnabled_;
    bool normalizeEnabled_;

    //============================================================================
    // 私有方法
    //============================================================================

    void printHeader(std::ostream& os);
    void printStateTransitionDiagram(std::ostream& os);
    void printRecognitionFlowchart(std::ostream& os);
    void printValidationResult(std::ostream& os);
    void printErrorSuggestions(std::ostream& os);

    /** 按需构建真实DFA流水线（一次性，缓存） */
    void ensureRealDFAs() const;

    // 可变缓存（用于可视化专用DFA实例）
    mutable void* cachedIdentDFA_ = nullptr;
    mutable void* cachedNumberDFA_ = nullptr;
};

} // namespace PL0

#endif // PL0_COMPILER_HPP
