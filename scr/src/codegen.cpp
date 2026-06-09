/**
 * @file codegen.cpp
 * @brief Code Generator Implementation
 */

#include "../include/pl0_codegen.hpp"

namespace PL0 {

//============================================================================
// CodeGenerator 实现
//============================================================================

CodeGenerator::CodeGenerator() : nextIndex_(START_ADDRESS), startAddress_(START_ADDRESS) {}

CodeGenerator::~CodeGenerator() {}

void CodeGenerator::emit(OpCode op, const std::string& arg1,
                         const std::string& arg2, const std::string& result) {
    Quadruple q(op, arg1, arg2, result, nextIndex_++);
    quadruples_.push_back(q);
}

int CodeGenerator::emitJump(OpCode op, const std::string& target) {
    Quadruple q(op, target, "", "", nextIndex_++);
    quadruples_.push_back(q);
    return nextIndex_ - 1;
}

void CodeGenerator::backpatch(int index, const std::string& target) {
    int internalIndex = index - startAddress_;
    if (internalIndex >= 0 && internalIndex < static_cast<int>(quadruples_.size())) {
        quadruples_[internalIndex].arg1 = target;
    }
}

Quadruple& CodeGenerator::getQuadruple(int index) {
    return quadruples_[index - startAddress_];
}

std::string CodeGenerator::opCodeToString(OpCode op) const {
    switch (op) {
        case OpCode::ADD: return "+";
        case OpCode::SUB: return "-";
        case OpCode::MUL: return "*";
        case OpCode::DIV: return "/";
        case OpCode::ODD: return "odd";
        case OpCode::EQ: return "=";
        case OpCode::NEQ: return "#";
        case OpCode::LT: return "<";
        case OpCode::LTE: return "<=";
        case OpCode::GT: return ">";
        case OpCode::GTE: return ">=";
        case OpCode::ASSIGN: return ":=";
        case OpCode::JUMP: return "jump";
        case OpCode::JZ: return "jz";
        case OpCode::JNZ: return "jnz";
        case OpCode::CALL: return "call";
        case OpCode::RET: return "ret";
        case OpCode::SYSS: return "syss";
        case OpCode::SYSC: return "sysc";
        case OpCode::READ: return "read";
        case OpCode::WRITE: return "write";
        default: return "unknown";
    }
}

void CodeGenerator::print(std::ostream& os) const {
    os << "\n===== QUADRUPLES =====\n";
    for (const auto& q : quadruples_) {
        os << "(" << q.index << ")(" << opCodeToString(q.op)
           << "," << q.arg1 << "," << q.arg2 << "," << q.result << ")\n";
    }
}

void CodeGenerator::clear() {
    quadruples_.clear();
    nextIndex_ = startAddress_;
}

} // namespace PL0
