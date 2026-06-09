/**
 * @file pl0_codegen.hpp
 * @brief PL/0 Compiler Code Generator Class
 * @details This class generates quadruple intermediate code for the PL/0 compiler.
 *          It supports emitting operations, jump instructions, and backpatching
 *          for conditional branches.
 * @author PL/0 Compiler Project
 * @date 2026-06-06
 */

#ifndef PL0_CODEGEN_HPP
#define PL0_CODEGEN_HPP

#include "pl0_core.hpp"

namespace PL0 {

/**
 * @class CodeGenerator
 * @brief Intermediate code generator for PL/0 compiler
 * @details The CodeGenerator class produces quadruples (op, arg1, arg2, result)
 *          representing the intermediate representation of PL/0 source code.
 */
class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();

    // 四元式生成
    void emit(OpCode op, const std::string& arg1 = "",
              const std::string& arg2 = "", const std::string& result = "");

    // 跳转指令
    int emitJump(OpCode op, const std::string& target = "");
    void backpatch(int index, const std::string& target);

    // 获取四元式
    const std::vector<Quadruple>& getQuadruples() const { return quadruples_; }
    Quadruple& getQuadruple(int index);

    // 辅助函数
    std::string opCodeToString(OpCode op) const;
    void print(std::ostream& os) const;
    void clear();

private:
    std::vector<Quadruple> quadruples_;
    int nextIndex_;
    int startAddress_;
};

} // namespace PL0

#endif // PL0_CODEGEN_HPP
