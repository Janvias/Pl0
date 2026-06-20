/**
 * @file test_framework.hpp
 * @brief PL/0编译器测试框架
 * @details 轻量级测试框架，提供断言宏、测试注册和结果报告功能
 * @author PL/0 Compiler Project
 * @date 2026-06-20
 */

#ifndef PL0_TEST_FRAMEWORK_HPP
#define PL0_TEST_FRAMEWORK_HPP

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdint>

namespace PL0 {
namespace Test {

//============================================================================
// 颜色输出支持
// 注意：不直接include <windows.h>，因为其"ERROR"宏会与
// PL/0的TokenType::ERROR冲突。现代Windows终端（Win10+）原生支持ANSI。
//============================================================================

// ANSI颜色代码（所有平台通用）
#define COLOR_GREEN  "\033[32m"
#define COLOR_RED    "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN   "\033[36m"
#define COLOR_RESET  "\033[0m"
#define COLOR_BOLD   "\033[1m"

inline void enableANSIColors() {
    // 现代终端（Windows 10+、Linux、macOS）默认支持ANSI转义序列
    // 不做额外配置，如果颜色不显示请升级终端
}

//============================================================================
// 测试结果结构
//============================================================================

struct TestCase {
    std::string suite;      // 测试套件名称
    std::string name;       // 测试用例名称
    std::string file;       // 测试文件路径
    bool expectSuccess;     // 预期是否成功编译
    bool expectDual;        // 是否预期双解析器一致
    bool passed;            // 测试是否通过
    std::string message;    // 测试结果消息
    int64_t durationUs;     // 测试耗时（微秒）

    TestCase() : expectSuccess(true), expectDual(true), passed(false), durationUs(0) {}
};

//============================================================================
// 测试结果收集器
//============================================================================

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void addTest(const TestCase& tc) {
        tests_.push_back(tc);
    }

    void addTest(TestCase&& tc) {
        tests_.push_back(std::move(tc));
    }

    TestCase& currentTest() {
        return tests_.back();
    }

    const std::vector<TestCase>& getTests() const { return tests_; }

    // 统计信息
    int getPassedCount() const {
        int count = 0;
        for (const auto& t : tests_) if (t.passed) count++;
        return count;
    }

    int getFailedCount() const {
        int count = 0;
        for (const auto& t : tests_) if (!t.passed) count++;
        return count;
    }

    int getTotalCount() const {
        return static_cast<int>(tests_.size());
    }

    void clear() { tests_.clear(); }

private:
    TestRunner() = default;
    std::vector<TestCase> tests_;
};

//============================================================================
// 断言宏
//============================================================================

#define TEST_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            PL0::Test::TestRunner::instance().currentTest().passed = false; \
            PL0::Test::TestRunner::instance().currentTest().message = msg; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(a, b, msg) \
    TEST_ASSERT((a) == (b), std::string(msg) + " (expected: " + std::to_string(b) + ", got: " + std::to_string(a) + ")")

#define TEST_ASSERT_TRUE(condition, msg) \
    TEST_ASSERT(condition, msg)

#define TEST_ASSERT_FALSE(condition, msg) \
    TEST_ASSERT(!(condition), msg)

//============================================================================
// 测试注册辅助
//============================================================================

inline void registerTest(const std::string& suite,
                         const std::string& name,
                         const std::string& file,
                         bool expectSuccess,
                         bool expectDual) {
    TestCase tc;
    tc.suite = suite;
    tc.name = name;
    tc.file = file;
    tc.expectSuccess = expectSuccess;
    tc.expectDual = expectDual;
    tc.passed = false;
    tc.message = "";
    tc.durationUs = 0;
    TestRunner::instance().addTest(tc);
}

//============================================================================
// 输出函数
//============================================================================

inline void printTestHeader() {
    std::cout << COLOR_BOLD << COLOR_CYAN;
    std::cout << "==============================================================\n";
    std::cout << "           PL/0 Compiler Test Suite v1.0\n";
    std::cout << "==============================================================";
    std::cout << COLOR_RESET << std::endl << std::endl;
}

inline void printTestSummary() {
    auto& runner = PL0::Test::TestRunner::instance();
    int passed = runner.getPassedCount();
    int failed = runner.getFailedCount();
    int total = runner.getTotalCount();

    std::cout << COLOR_BOLD;
    std::cout << "\n--------------------------------------------------------------\n";
    std::cout << "                    Test Result Summary\n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << "  Total: " << std::setw(6) << total << "  |  ";

    if (passed > 0) std::cout << COLOR_GREEN;
    std::cout << "Passed: " << std::setw(6) << passed << COLOR_RESET << COLOR_BOLD << "  |  ";

    if (failed > 0) std::cout << COLOR_RED;
    std::cout << "Failed: " << std::setw(6) << failed << COLOR_RESET << COLOR_BOLD;

    double rate = (total > 0) ? (100.0 * passed / total) : 0.0;
    std::cout << "  |  Rate: " << std::fixed << std::setprecision(1)
              << rate << "%\n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << COLOR_RESET << std::endl;
}

inline void printSuiteHeader(const std::string& suite) {
    std::cout << COLOR_BOLD << COLOR_YELLOW;
    std::cout << "\n-- " << suite << "\n";
    std::cout << COLOR_RESET;
}

inline void printSuiteFooter() {
    // no-op for now
}

inline void printTestResult(const TestCase& tc) {
    std::cout << "  ";
    if (tc.passed) {
        std::cout << COLOR_GREEN << "[PASS]" << COLOR_RESET;
    } else {
        std::cout << COLOR_RED << "[FAIL]" << COLOR_RESET;
    }
    std::cout << " " << std::left << std::setw(45) << tc.name;
    if (!tc.message.empty()) {
        std::cout << " [" << COLOR_YELLOW << tc.message << COLOR_RESET << "]";
    }
    if (tc.durationUs > 0) {
        std::cout << " (" << std::fixed << std::setprecision(2)
                  << (tc.durationUs / 1000.0) << "ms)";
    }
    std::cout << std::endl;
}

} // namespace Test
} // namespace PL0

#endif // PL0_TEST_FRAMEWORK_HPP
