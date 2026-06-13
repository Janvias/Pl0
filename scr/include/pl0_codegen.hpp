/**
 * @file pl0_codegen.hpp
 * @brief PL/0编译器代码生成器类
 * @details 本类为PL/0编译器生成四元式中间代码，
 *          支持发射操作、跳转指令和条件分支的回填
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_CODEGEN_HPP
#define PL0_CODEGEN_HPP

#include "pl0_core.hpp"

namespace PL0 {

/**
 * @class CodeGenerator
 * @brief PL/0编译器中间代码生成器
 * @details CodeGenerator类产生四元式(op, arg1, arg2, result)
 *          表示PL/0源代码的中间表示
 */
class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();

    // 四元式生成
    void emit(OpCode op, const std::string& arg1 = "",
              const std::string& arg2 = "", const std::string& result = "");

    // 跳转指令
    int emitJump(OpCode op, const std::string& target = "");  // 发射跳转指令
    void backpatch(int index, const std::string& target);     // 回填跳转目标

    // 获取四元式
    const std::vector<Quadruple>& getQuadruples() const { return quadruples_; }
    Quadruple& getQuadruple(int index);

    // 辅助函数
    std::string opCodeToString(OpCode op) const;  // 操作码转字符串
    void print(std::ostream& os) const;           // 输出四元式
    void clear();                                 // 清空四元式列表

private:
    std::vector<Quadruple> quadruples_;  // 四元式列表
    int nextIndex_;                      // 下一个四元式序号
    int startAddress_;                   // 起始地址
};

} // namespace PL0

#endif // PL0_CODEGEN_HPP
