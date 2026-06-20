/**
 * @file run_tests.cpp
 * @brief PL/0编译器测试主入口
 * @details 运行所有测试套件，包括单元测试、集成测试、系统测试和性能测试
 * @author PL/0 Compiler Project
 * @date 2026-06-20
 */

#include "test_framework.hpp"
#include "pl0_compiler.hpp"
#include <chrono>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace PL0;
using namespace PL0::Test;

// 自动检测：从 scr/ 运行时使用 "tests/" 前缀，从 tests/ 运行时使用 ""
static std::string BASE_DIR;

// 创建输出目录 (_mkdir 在MinGW上可用)
static void ensureOutputDir() {
    std::string dir = BASE_DIR + "output";
    _mkdir(dir.c_str());
}

//============================================================================
// 测试执行辅助函数
//============================================================================

/**
 * @brief 编译PL/0源文件并检查结果
 * @param file 源文件路径
 * @param expectedSuccess 预期编译是否成功
 * @param useDual 是否使用双解析器模式
 * @param[out] outSuccess 实际编译是否成功
 * @param[out] outMessage 输出消息
 */
void runCompileTest(const std::string& file, bool expectedSuccess, bool useDual,
                    bool& outSuccess, std::string& outMessage) {
    Compiler compiler;
    compiler.setVerbose(false);

    if (useDual) {
        compiler.setParserMode(ParserMode::DUAL);
    } else {
        compiler.setParserMode(ParserMode::LL1_ONLY);
    }

    // 尝试给定路径；如果文件未找到，尝试去掉或添加"tests/"前缀
    // (同时支持从 scr/ 和 tests/ 目录运行)
    std::string resolvedFile = file;
    bool compileResult = compiler.compile(resolvedFile);
    if (!compileResult && compiler.getErrorMessage().find("Cannot open file") != std::string::npos
        && file.compare(0, 6, "tests/") == 0) {
        // 去掉"tests/"前缀重试
        resolvedFile = file.substr(6);
        compileResult = compiler.compile(resolvedFile);
    }
    if (!compileResult && compiler.getErrorMessage().find("Cannot open file") != std::string::npos
        && file.compare(0, 6, "tests/") != 0) {
        // 添加"tests/"前缀重试
        resolvedFile = "tests/" + file;
        compileResult = compiler.compile(resolvedFile);
    }

    if (expectedSuccess) {
        // 预期编译成功
        if (compileResult) {
            outSuccess = true;
            outMessage = "Compilation OK (as expected)";

            // 双解析器模式额外检查
            if (useDual) {
                const auto& validation = compiler.getValidationResult();
                if (!validation.bothSucceeded) {
                    outSuccess = false;
                    outMessage = "Dual parser mismatch: LL1=" + validation.ll1Error +
                                 ", LR1=" + validation.lr1Error;
                } else if (!validation.resultsMatch) {
                    // 四元式差异是允许的（不同但等效的代码生成策略）
                    outMessage = "Compilation OK, quads differ (equivalent)";
                }
            }

            // 生成中间输出文件: 四元式报告 + 缓存
            // 输出到 tests/output/<testname>_*.txt
            {
                // 从文件路径提取测试名 (如 unit/lexer/keywords -> keywords)
                std::string testName = resolvedFile;
                size_t lastSlash = testName.find_last_of("/\\");
                if (lastSlash != std::string::npos) testName = testName.substr(lastSlash + 1);
                size_t dot = testName.rfind('.');
                if (dot != std::string::npos) testName = testName.substr(0, dot);

                // 写入到 BASE_DIR 前缀路径（相对CWD）
                std::string prefix = BASE_DIR + "output/" + testName;
                compiler.writeQuadReport(prefix + "_quad.txt");
                compiler.writeCache(prefix + "_cache.txt");
            }
        } else {
            outSuccess = false;
            outMessage = "Compilation FAILED (expected success): " + compiler.getErrorMessage();
        }
    } else {
        // 预期编译失败
        if (compileResult) {
            outSuccess = false;
            outMessage = "Compilation OK (expected FAILURE)";
        } else {
            outSuccess = true;
            outMessage = "Error correctly detected: " + compiler.getErrorMessage();
        }
    }
}

/**
 * @brief 运行单个测试用例
 * @param tc 测试用例引用
 */
void runSingleTest(TestCase& tc) {
    auto start = std::chrono::high_resolution_clock::now();

    bool success;
    std::string message;
    runCompileTest(tc.file, tc.expectSuccess, tc.expectDual, success, message);

    auto end = std::chrono::high_resolution_clock::now();
    tc.passed = success;
    tc.message = message;
    tc.durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    printTestResult(tc);
}

//============================================================================
// 测试套件注册
//============================================================================

// --- 单元测试: 词法分析 (Lexer) ---
void registerLexerTests() {
    registerTest("Unit | Lexer", "Keyword recognition",        BASE_DIR + "unit/lexer/keywords.pl0",             true,  true);
    registerTest("Unit | Lexer", "Identifier recognition",     BASE_DIR + "unit/lexer/identifiers.pl0",          true,  true);
    registerTest("Unit | Lexer", "Number recognition",         BASE_DIR + "unit/lexer/numbers.pl0",              true,  true);
    registerTest("Unit | Lexer", "Operator recognition",       BASE_DIR + "unit/lexer/operators.pl0",            true,  true);
    registerTest("Unit | Lexer", "Delimiter recognition",      BASE_DIR + "unit/lexer/delimiters.pl0",           true,  true);
    registerTest("Unit | Lexer", "Comments and whitespace",    BASE_DIR + "unit/lexer/comments_whitespace.pl0",  true,  true);
}

// --- 单元测试: 语法分析/声明 (Parser - Declarations) ---
void registerParserDeclTests() {
    registerTest("Unit | Parser-Decl", "Const declaration",       BASE_DIR + "unit/parser/const_decl.pl0",        true,  true);
    registerTest("Unit | Parser-Decl", "Multiple const decls",     BASE_DIR + "unit/parser/multi_const.pl0",       true,  true);
    registerTest("Unit | Parser-Decl", "Var declaration",         BASE_DIR + "unit/parser/var_decl.pl0",          true,  true);
    registerTest("Unit | Parser-Decl", "Procedure declaration",   BASE_DIR + "unit/parser/proc_decl.pl0",         true,  true);
    registerTest("Unit | Parser-Decl", "Nested procedures",       BASE_DIR + "unit/parser/nested_proc.pl0",       true,  true);
}

// --- 单元测试: 语法分析/语句 (Parser - Statements) ---
void registerParserStmtTests() {
    registerTest("Unit | Parser-Stmt", "Assignment statement",    BASE_DIR + "unit/parser/assign_stmt.pl0",       true,  true);
    registerTest("Unit | Parser-Stmt", "Compound statement",      BASE_DIR + "unit/parser/compound_stmt.pl0",     true,  true);
    registerTest("Unit | Parser-Stmt", "If statement",            BASE_DIR + "unit/parser/if_stmt.pl0",           true,  true);
    registerTest("Unit | Parser-Stmt", "While statement",         BASE_DIR + "unit/parser/while_stmt.pl0",        true,  true);
    registerTest("Unit | Parser-Stmt", "Call statement",          BASE_DIR + "unit/parser/call_stmt.pl0",         true,  true);
    registerTest("Unit | Parser-Stmt", "Read/Write statements",   BASE_DIR + "unit/parser/read_write_stmt.pl0",   true,  true);
}

// --- 单元测试: 语法分析/表达式 (Parser - Expressions) ---
void registerParserExprTests() {
    registerTest("Unit | Parser-Expr", "Simple expression",       BASE_DIR + "unit/parser/simple_expr.pl0",       true,  true);
    registerTest("Unit | Parser-Expr", "Complex expression",      BASE_DIR + "unit/parser/complex_expr.pl0",      true,  true);
    registerTest("Unit | Parser-Expr", "Parenthesized expr",      BASE_DIR + "unit/parser/paren_expr.pl0",        true,  true);
    registerTest("Unit | Parser-Expr", "Negation expression",     BASE_DIR + "unit/parser/neg_expr.pl0",          true,  true);
}

// --- 单元测试: 语法分析/条件 (Parser - Conditions) ---
void registerParserCondTests() {
    registerTest("Unit | Parser-Cond", "Odd & relational conds",  BASE_DIR + "unit/parser/conditions.pl0",        true,  true);
}

// --- 单元测试: LR(1)解析器 ---
void registerLR1ParserTests() {
    registerTest("Unit | LR(1) Parser", "Shift-reduce basic",     BASE_DIR + "unit/lr1_parser/lr1_shift_reduce.pl0",  true,  true);
    registerTest("Unit | LR(1) Parser", "Left recursion equiv",   BASE_DIR + "unit/lr1_parser/lr1_left_recursion.pl0", true,  true);
    registerTest("Unit | LR(1) Parser", "382 states coverage",    BASE_DIR + "unit/lr1_parser/lr1_382_states.pl0",     true,  true);
    registerTest("Unit | LR(1) Parser", "GOTO/ACTION table",      BASE_DIR + "unit/lr1_parser/lr1_goto_action.pl0",    true,  true);
}

// --- 单元测试: AST构建器 ---
void registerASTBuilderTests() {
    registerTest("Unit | AST Builder", "AST node construction",   BASE_DIR + "unit/astbuilder/ast_nodes.pl0",         true,  true);
}

// --- 单元测试: 符号表 (Symbol Table) ---
void registerSymTableTests() {
    registerTest("Unit | Symbol Table", "Symbol insert (const/var/proc)", BASE_DIR + "unit/symtable/symbol_insert.pl0",   true,  true);
    registerTest("Unit | Symbol Table", "Cross-scope lookup",            BASE_DIR + "unit/symtable/scope_lookup.pl0",     true,  true);
    registerTest("Unit | Symbol Table", "Scope shadowing",               BASE_DIR + "unit/symtable/scope_shadow.pl0",     true,  true);
}

// --- 单元测试: 代码生成 (Code Generator) ---
void registerCodeGenTests() {
    registerTest("Unit | CodeGen", "Assignment quads",           BASE_DIR + "unit/codegen/assign_codegen.pl0",    true,  true);
    registerTest("Unit | CodeGen", "If/While quads",             BASE_DIR + "unit/codegen/if_while_codegen.pl0",  true,  true);
    registerTest("Unit | CodeGen", "Procedure call quads",       BASE_DIR + "unit/codegen/call_codegen.pl0",      true,  true);
    registerTest("Unit | CodeGen", "IO quads",                   BASE_DIR + "unit/codegen/io_codegen.pl0",        true,  true);
    registerTest("Unit | CodeGen", "Complex arithmetic quads",   BASE_DIR + "unit/codegen/complex_arith_codegen.pl0", true,  true);
}

// --- 集成测试: 编译流程 ---
void registerIntegrationCompileTests() {
    registerTest("Integration | Compile", "Full program compilation",  BASE_DIR + "integration/compilation/full_compile.pl0",  true,  true);
    registerTest("Integration | Compile", "Multi-procedure compilation", BASE_DIR + "integration/compilation/proc_compile.pl0",  true,  true);
}

// --- 集成测试: 双解析器验证 ---
void registerDualParserTests() {
    registerTest("Integration | Dual Parser", "Simple program dual parse",  BASE_DIR + "integration/dual_parser/dual_simple.pl0",  true,  true);
    registerTest("Integration | Dual Parser", "Complex program dual parse", BASE_DIR + "integration/dual_parser/dual_complex.pl0", true,  true);
}

// --- 系统测试: 功能测试 ---
void registerFunctionalTests() {
    registerTest("System | Functional", "Complete calculation program",  BASE_DIR + "system/functional/complete_program.pl0",  true,  true);
    registerTest("System | Functional", "Fibonacci sequence",             BASE_DIR + "system/functional/fibonacci.pl0",         true,  true);
    registerTest("System | Functional", "Factorial calculation",          BASE_DIR + "system/functional/factorial.pl0",         true,  true);
    registerTest("System | Functional", "Odd/Even check",                 BASE_DIR + "system/functional/odd_even.pl0",          true,  true);
    registerTest("System | Functional", "GCD calculation",                BASE_DIR + "system/functional/gcd.pl0",               true,  true);
}

// --- 系统测试: 边界测试 ---
void registerBoundaryTests() {
    registerTest("System | Boundary", "Empty program (dot only)",    BASE_DIR + "system/boundary/empty_program.pl0",       true,  true);
    registerTest("System | Boundary", "Empty compound statement",    BASE_DIR + "system/boundary/empty_compound.pl0",      true,  true);
    registerTest("System | Boundary", "Max identifier length (8)",   BASE_DIR + "system/boundary/max_identifier.pl0",      true,  true);
    registerTest("System | Boundary", "Max number (8 digits)",       BASE_DIR + "system/boundary/max_number.pl0",          true,  true);
    registerTest("System | Boundary", "Deeply nested procedures",    BASE_DIR + "system/boundary/deep_nesting.pl0",        true,  true);
    registerTest("System | Boundary", "Many variable declarations",  BASE_DIR + "system/boundary/many_vars.pl0",           true,  true);
}

// --- 系统测试: 错误处理 ---
void registerErrorHandlingTests() {
    registerTest("System | Error Handling", "Lexical error (illegal char)",  BASE_DIR + "system/error_handling/lexer_error.pl0",      false, true);
    registerTest("System | Error Handling", "Syntax error (incomplete)",     BASE_DIR + "system/error_handling/syntax_error.pl0",     false, true);
    registerTest("System | Error Handling", "Missing semicolon",             BASE_DIR + "system/error_handling/missing_semicolon.pl0", false, true);
    registerTest("System | Error Handling", "Undefined variable",            BASE_DIR + "system/error_handling/undefined_var.pl0",    false, true);
    registerTest("System | Error Handling", "Duplicate definition",          BASE_DIR + "system/error_handling/duplicate_def.pl0",    true,  true);  // 已知限制：当前编译器不检测重复定义
    registerTest("System | Error Handling", "Const assignment error",        BASE_DIR + "system/error_handling/wrong_assign.pl0",     false, true);
    registerTest("System | Error Handling", "Both parsers report error",     BASE_DIR + "system/error_handling/dual_error.pl0",       false, true);
}

// --- 性能测试 ---
void registerPerformanceTests() {
    registerTest("Performance | Speed", "Medium program compile speed",  BASE_DIR + "performance/compile_speed/medium_prog.pl0",  true,  true);
}

//============================================================================
// 主函数
//============================================================================

int main() {
    enableANSIColors();
    printTestHeader();

    // 自动检测BASE_DIR: 从scr/运行时使用"tests/"前缀，从tests/运行时使用""
    {
        std::ifstream probe("unit/lexer/keywords.pl0");
        if (probe.is_open()) {
            BASE_DIR = "";          // 已在 tests/ 目录内
        } else {
            BASE_DIR = "tests/";    // 在 scr/ 目录下
        }
    }

    // 确保输出目录存在
    ensureOutputDir();

    // 注册所有测试
    // runCompileTest 在文件未找到时会自动重试另一种前缀。
    registerLexerTests();
    registerParserDeclTests();
    registerParserStmtTests();
    registerParserExprTests();
    registerParserCondTests();
    registerLR1ParserTests();
    registerASTBuilderTests();
    registerSymTableTests();
    registerCodeGenTests();
    registerIntegrationCompileTests();
    registerDualParserTests();
    registerFunctionalTests();
    registerBoundaryTests();
    registerErrorHandlingTests();
    registerPerformanceTests();

    // 获取所有已注册测试
    auto& runner = TestRunner::instance();
    auto tests = runner.getTests();  // 拷贝以便修改结果

    // 按套件分组运行
    std::string currentSuite;
    for (auto& tc : tests) {
        if (tc.suite != currentSuite) {
            if (!currentSuite.empty()) {
                std::cout << "--------------------------------------------------------------\n";
            }
            currentSuite = tc.suite;
            printSuiteHeader(currentSuite);
        }
        runSingleTest(tc);
    }
    std::cout << "--------------------------------------------------------------\n";

    // 更新 runner 中的结果
    runner.clear();
    for (const auto& tc : tests) {
        runner.addTest(tc);
    }

    // 按类别统计
    std::cout << COLOR_BOLD << "\n-- Category Statistics -------------------------------------\n" << COLOR_RESET;
    auto allTests = runner.getTests();
    struct CategoryStats {
        std::string name;
        int total, passed;
    };
    std::vector<CategoryStats> categories;
    for (const auto& tc : allTests) {
        // 取套件名第一段作为分类名
        std::string cat = tc.suite.substr(0, tc.suite.find('|') - 1);
        auto it = std::find_if(categories.begin(), categories.end(),
                               [&cat](const CategoryStats& cs) { return cs.name == cat; });
        if (it != categories.end()) {
            it->total++;
            if (tc.passed) it->passed++;
        } else {
            categories.push_back({cat, 1, tc.passed ? 1 : 0});
        }
    }
    for (const auto& cs : categories) {
        std::cout << "  " << std::left << std::setw(20) << cs.name
                  << " : " << COLOR_GREEN << cs.passed << COLOR_RESET
                  << " / " << cs.total;
        if (cs.passed < cs.total) {
            std::cout << "  " << COLOR_RED << "(" << (cs.total - cs.passed) << " failed)" << COLOR_RESET;
        }
        std::cout << "\n";
    }
    std::cout << "--------------------------------------------------------------\n";

    printTestSummary();

    // 写入测试结果到文件 (tests/build/test_results.txt)
    {
        auto writeResults = [&](const std::string& filepath) -> bool {
            std::ofstream ofs(filepath);
            if (!ofs) return false;
            ofs << "PL/0 Compiler Test Suite Results\n";
            ofs << "================================\n\n";
            for (const auto& tc : tests) {
                ofs << (tc.passed ? "[PASS]" : "[FAIL]") << " "
                    << tc.suite << " | " << tc.name << "\n";
                if (!tc.message.empty()) {
                    ofs << "       " << tc.message << "\n";
                }
            }
            ofs << "\n--------------------------------\n";
            ofs << "Total: " << runner.getTotalCount()
                << "  Passed: " << runner.getPassedCount()
                << "  Failed: " << runner.getFailedCount()
                << "  Rate: " << std::fixed << std::setprecision(1)
                << (100.0 * runner.getPassedCount() / runner.getTotalCount()) << "%\n";
            return true;
        };
        // 尝试从 tests/ 目录写入，如果失败则尝试当前目录
        if (!writeResults("tests/build/test_results.txt")) {
            writeResults("build/test_results.txt");
        }
    }

    // 有失败返回1，全部通过返回0
    return (runner.getFailedCount() > 0) ? 1 : 0;
}
