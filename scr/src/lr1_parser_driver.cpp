/**
 * @file lr1_parser_driver.cpp
 * @brief LR(1)解析器驱动器 — Token映射、移进-归约引擎、语义动作
 * @details 包含LR(1)解析器构造函数（协调分析表构建）、移进-归约解析循环、
 *          Token到终结符的映射，以及所有生成四元式的语义动作。
 * @author PL/0 Compiler Project
 * @date 2026-06-09
 */

#include "../include/pl0_lr1_parser.hpp"
#include <stack>
#include <string>
#include <sstream>

namespace PL0 {

//============================================================================
// 构造函数 / 析构函数
//============================================================================
// 构造函数按顺序执行：
// 1. 初始化文法（定义46条产生式）
// 2. 计算FIRST集
// 3. 计算FOLLOW集
// 4. 构建规范LR(1)项目集族
// 5. 构建分析表（ACTION和GOTO）

LR1Parser::LR1Parser()
    : hasError_(false), tokens_(nullptr), tokenPos_(0) {
    initGrammar();
    computeFirstSets();
    computeFollowSets();
    buildCanonicalCollection();
    buildParseTables();
}

LR1Parser::~LR1Parser() {}

//============================================================================
// Token到终结符映射
//============================================================================
// 将词法分析器生成的Token转换为LR(1)文法的终结符
// 支持：关键字、标识符、数字、运算符、分隔符、EOF

LR1Terminal LR1Parser::tokenToTerminal(const Token& token) const {
    using T = LR1Terminal;

    if (token.type == TokenType::END_OF_FILE) return T::tEOF;

    if (token.type == TokenType::KEYWORD) {
        if (token.value == "const")      return T::tCONST;
        if (token.value == "var")        return T::tVAR;
        if (token.value == "procedure")  return T::tPROCEDURE;
        if (token.value == "begin")      return T::tBEGIN;
        if (token.value == "end")        return T::tEND;
        if (token.value == "if")         return T::tIF;
        if (token.value == "then")       return T::tTHEN;
        if (token.value == "while")      return T::tWHILE;
        if (token.value == "do")         return T::tDO;
        if (token.value == "call")       return T::tCALL;
        if (token.value == "read")       return T::tREAD;
        if (token.value == "write")      return T::tWRITE;
        if (token.value == "odd")        return T::tODD;
    }

    if (token.type == TokenType::IDENTIFIER) return T::tIDENT;
    if (token.type == TokenType::NUMBER)     return T::tNUMBER;

    if (token.type == TokenType::OPERATOR) {
        if (token.value == "+")  return T::tPLUS;
        if (token.value == "-")  return T::tMINUS;
        if (token.value == "*")  return T::tSTAR;
        if (token.value == "/")  return T::tSLASH;
        if (token.value == "=")  return T::tEQ;
        if (token.value == "#")  return T::tNEQ;
        if (token.value == "<")  return T::tLT;
        if (token.value == "<=") return T::tLTE;
        if (token.value == ">")  return T::tGT;
        if (token.value == ">=") return T::tGTE;
        if (token.value == ":=") return T::tASSIGN;
    }

    if (token.type == TokenType::DELIMITER) {
        if (token.value == "(") return T::tLPAREN;
        if (token.value == ")") return T::tRPAREN;
        if (token.value == ".") return T::tDOT;
        if (token.value == ",") return T::tCOMMA;
        if (token.value == ";") return T::tSEMICOLON;
    }

    return T::tEOF; // fallback
}

//============================================================================
// 语义动作占位符
//============================================================================
// 实际的语义动作内联在parse()方法的归约处理器中。
// 此方法仅用于文档说明和潜在的外部调用。

void LR1Parser::executeSemanticAction(int productionId) {
    (void)productionId; // See parse() for actual implementation
}

//============================================================================
// 解析驱动器 — 移进-归约算法
//============================================================================
// LR(1)移进-归约解析的主循环：
// 1. 根据当前状态和向前看符号查找ACTION表
// 2. SHIFT：将终结符压栈，进入下一状态
// 3. REDUCE：根据产生式归约，执行语义动作，通过GOTO表转移状态
// 4. ACCEPT：解析成功完成
// 5. ERROR：语法错误

bool LR1Parser::parse(const std::vector<Token>& tokens) {
    using T = LR1Terminal;
    using N = LR1NonTerminal;

    tokens_ = &tokens;
    tokenPos_ = 0;
    hasError_ = false;
    errorMessage_ = "";
    quadruples_.clear();

    // Emit program start marker (matching LL(1) behavior)
    symTable_.enterScope("main");
    quadruples_.push_back(Quadruple(
        OpCode::SYSS, "", "", "",
        static_cast<int>(quadruples_.size()) + 100));

    // ---- Value stack: tracks semantic values alongside grammar symbols ----
    struct StackEntry {
        LR1Symbol symbol;
        std::string value;       // For identifiers, numbers, operators
        std::string tempName;    // For expression temporaries (T1, T2, ...)
        int quadIndex = 0;       // Quad index at time of push (0-based, for jump labels)
    };

    std::vector<StackEntry> valueStack;
    std::vector<int> stateStack;
    stateStack.push_back(0); // Start state

    // ---- Lambda helpers ----
    auto currentTerminal = [&]() -> T {
        if (tokenPos_ < tokens.size())
            return tokenToTerminal(tokens[tokenPos_]);
        return T::tEOF;
    };

    auto currentValue = [&]() -> std::string {
        if (tokenPos_ < tokens.size())
            return tokens[tokenPos_].value;
        return "";
    };

    auto advanceToken = [&]() {
        if (tokenPos_ < tokens.size()) tokenPos_++;
    };

    int tempCounter = 0;
    auto newTemp = [&]() -> std::string {
        return "T" + std::to_string(++tempCounter);
    };

    bool accepted = false;

    // ---- Main parsing loop ----
    while (!accepted) {
        int state = stateStack.back();
        T lookahead = currentTerminal();

        auto actionKey = std::make_pair(state, lookahead);
        auto actionIt = actionTable_.find(actionKey);

        if (actionIt == actionTable_.end()) {
            int lineNum = (tokenPos_ < tokens.size())
                              ? tokens[tokenPos_].line : 0;
            hasError_ = true;
            errorMessage_ = "(LR1 Syntax Error, Line: " +
                            std::to_string(lineNum) +
                            ") Unexpected token '" + currentValue() +
                            "', state " + std::to_string(state);
            return false;
        }

        const LRAction& action = actionIt->second;

        switch (action.type) {

        // ================================================================
        // SHIFT
        // ================================================================
        case LRActionType::SHIFT: {
            StackEntry entry;
            entry.symbol = LR1Symbol(lookahead);
            entry.value = currentValue();
            entry.quadIndex = static_cast<int>(quadruples_.size());
            valueStack.push_back(entry);
            stateStack.push_back(action.value);
            advanceToken();
            break;
        }

        // ================================================================
        // REDUCE
        // ================================================================
        case LRActionType::REDUCE: {
            int prodId = action.value;
            const auto& prod = productions_[prodId];

            // Epsilon productions: nothing to pop, no semantic action needed
            if (prodId == 6  || // DL -> epsilon
                prodId == 9  || // CL -> epsilon
                prodId == 12 || // VL -> epsilon
                prodId == 17 || // L  -> epsilon
                prodId == 24 || // A  -> epsilon
                prodId == 25 || // S  -> epsilon
                prodId == 38 || // ET -> epsilon
                prodId == 42) { // TT -> epsilon
                // No popping, just push the result non-terminal
                StackEntry result;
                result.symbol = LR1Symbol(prod.lhs);
                result.quadIndex = static_cast<int>(quadruples_.size());
                valueStack.push_back(result);
            } else {
                // Pop RHS symbols from both stacks
                size_t popCount = prod.rhs.size();
                std::vector<StackEntry> rhsValues;
                for (size_t i = 0; i < popCount; i++) {
                    rhsValues.insert(rhsValues.begin(), valueStack.back());
                    valueStack.pop_back();
                    stateStack.pop_back();
                }

                // Execute semantic action
                StackEntry result;
                result.symbol = LR1Symbol(prod.lhs);
                result.quadIndex = rhsValues[0].quadIndex;

                switch (prodId) {

                // ---- Declarations ----
                case 7:   // C -> const I = N CL ;
                case 8: { // CL -> , I = N CL
                    std::string name = rhsValues[1].value;
                    int val = std::stoi(rhsValues[3].value);
                    Symbol sym;
                    sym.name = name;
                    sym.kind = SymbolKind::CONSTANT;
                    sym.value = val;
                    sym.level = symTable_.getCurrentLevel();
                    symTable_.addSymbol(sym);
                    break;
                }

                case 10:  // V -> var I VL ;
                case 11: { // VL -> , I VL
                    std::string name = rhsValues[1].value;
                    Symbol sym;
                    sym.name = name;
                    sym.kind = SymbolKind::VARIABLE;
                    sym.level = symTable_.getCurrentLevel();
                    sym.address = symTable_.getCurrentLevel();
                    symTable_.addSymbol(sym);
                    break;
                }

                case 13: { // PR -> procedure I ; B ;
                    std::string name = rhsValues[1].value;
                    Symbol sym;
                    sym.name = name;
                    sym.kind = SymbolKind::PROCEDURE;
                    sym.level = symTable_.getCurrentLevel();
                    sym.address = static_cast<int>(quadruples_.size()) + 100;
                    symTable_.addSymbol(sym);
                    break;
                }

                // ---- Statements ----
                case 14: { // S -> I := E
                    std::string name = rhsValues[0].value;
                    std::string temp = rhsValues[2].tempName.empty()
                                           ? rhsValues[2].value
                                           : rhsValues[2].tempName;
                    quadruples_.push_back(Quadruple(
                        OpCode::ASSIGN, temp, "", name,
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                case 20: { // S -> call I
                    quadruples_.push_back(Quadruple(
                        OpCode::CALL, rhsValues[1].value, "", "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                case 21: { // S -> read ( I )
                    quadruples_.push_back(Quadruple(
                        OpCode::READ, "", "", rhsValues[2].value,
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                case 22: // S -> write ( E A ) — first argument
                    quadruples_.push_back(Quadruple(
                        OpCode::WRITE, "", "", "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;

                case 23: // A -> , E A  (additional write argument)
                    quadruples_.push_back(Quadruple(
                        OpCode::WRITE, "", "", "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;

                // ---- Conditions ----
                case 26: { // CO -> odd E
                    std::string operand = rhsValues[1].tempName.empty()
                                          ? rhsValues[1].value
                                          : rhsValues[1].tempName;
                    quadruples_.push_back(Quadruple(
                        OpCode::ODD, operand, "", "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }
                case 27: // CO -> E = E
                case 28: // CO -> E # E
                case 29: // CO -> E < E
                case 30: // CO -> E <= E
                case 31: // CO -> E > E
                case 32: { // CO -> E >= E
                    // 左操作数来自rhsValues[0]（左E），右操作数来自rhsValues[2]（右E）
                    std::string leftOp = rhsValues[0].tempName.empty()
                                        ? rhsValues[0].value
                                        : rhsValues[0].tempName;
                    std::string rightOp = rhsValues[2].tempName.empty()
                                          ? rhsValues[2].value
                                          : rhsValues[2].tempName;
                    OpCode op;
                    switch (prodId) {
                        case 27: op = OpCode::EQ;  break;
                        case 28: op = OpCode::NEQ; break;
                        case 29: op = OpCode::LT;  break;
                        case 30: op = OpCode::LTE; break;
                        case 31: op = OpCode::GT;  break;
                        default: op = OpCode::GTE; break;
                    }
                    quadruples_.push_back(Quadruple(
                        op, leftOp, rightOp, "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                // ---- Expressions (right-recursive: need left operand from stack) ----
                case 33: // E -> + T ET (unary plus — pass through)
                    result.tempName = rhsValues[1].tempName.empty()
                                          ? rhsValues[1].value
                                          : rhsValues[1].tempName;
                    break;

                case 34: { // E -> - T ET (unary minus)
                    std::string tVal = rhsValues[1].tempName.empty()
                                           ? rhsValues[1].value
                                           : rhsValues[1].tempName;
                    result.tempName = newTemp();
                    quadruples_.push_back(Quadruple(
                        OpCode::SUB, "0", tVal, result.tempName,
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                case 35: // E -> T ET
                case 39: // T -> F TT
                    if (rhsValues[1].tempName.empty())
                        result.tempName = rhsValues[0].tempName.empty()
                                              ? rhsValues[0].value
                                              : rhsValues[0].tempName;
                    else
                        result.tempName = rhsValues[1].tempName;
                    break;

                case 36: // ET -> + T ET (binary +)
                case 37: // ET -> - T ET (binary -)
                case 40: // TT -> * F TT
                case 41: { // TT -> / F TT
                    // Left operand is on value stack just below popped RHS
                    std::string leftVal = valueStack.back().tempName.empty()
                                          ? valueStack.back().value
                                          : valueStack.back().tempName;
                    std::string rightVal = rhsValues[1].tempName.empty()
                                           ? rhsValues[1].value
                                           : rhsValues[1].tempName;
                    result.tempName = newTemp();
                    OpCode op = OpCode::ADD;
                    if (prodId == 37) op = OpCode::SUB;
                    else if (prodId == 40) op = OpCode::MUL;
                    else if (prodId == 41) op = OpCode::DIV;
                    quadruples_.push_back(Quadruple(
                        op, leftVal, rightVal, result.tempName,
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                case 43: { // F -> I
                    std::string name = rhsValues[0].value;
                    Symbol* sym = symTable_.lookup(name);
                    if (sym && sym->kind == SymbolKind::CONSTANT)
                        result.value = std::to_string(sym->value);
                    else
                        result.tempName = name;
                    break;
                }

                case 44: // F -> N
                    result.value = rhsValues[0].value;
                    break;

                case 45: // F -> ( E )
                    result.tempName = rhsValues[1].tempName.empty()
                                          ? rhsValues[1].value
                                          : rhsValues[1].tempName;
                    break;

                // Program/Block (no semantic action needed)
                case 1:  // P -> B .
                case 2:  // B -> DL S
                case 3:  // DL -> DL C
                case 4:  // DL -> DL V
                case 5:  // DL -> DL PR
                case 15: // S -> begin S L end
                case 16: // L -> ; S L
                    break;

                case 18: // S -> if CO then S
                    // LL(1) also emits no jump for if-then;
                    // the PL/0 VM uses the condition result implicitly.
                    break;

                case 19: { // S -> while CO do S
                    // rhsValues[1] = CO (condition), rhsValues[3] = S (body)
                    // Emit JUMP back to the start of the condition code
                    int condStart = rhsValues[1].quadIndex;
                    std::string loopLabel = "L" + std::to_string(condStart);
                    quadruples_.push_back(Quadruple(
                        OpCode::JUMP, loopLabel, "", "",
                        static_cast<int>(quadruples_.size()) + 100));
                    break;
                }

                default:
                    break;
                }

                valueStack.push_back(result);
            }

            // After reduction, look up GOTO
            int curState = stateStack.back();
            N lhs = prod.lhs;
            auto gotoKey = std::make_pair(curState, lhs);
            auto gotoIt = gotoTable_.find(gotoKey);
            if (gotoIt != gotoTable_.end()) {
                stateStack.push_back(gotoIt->second);
            } else {
                hasError_ = true;
                std::ostringstream oss;
                oss << "(LR1 Internal Error) No GOTO for state "
                    << curState << ", non-terminal "
                    << nonTerminalToString(lhs);
                errorMessage_ = oss.str();
                return false;
            }
            break;
        }

        // ================================================================
        // ACCEPT
        // ================================================================
        case LRActionType::ACCEPT:
            quadruples_.push_back(Quadruple(
                OpCode::SYSC, "", "", "",
                static_cast<int>(quadruples_.size()) + 100));
            symTable_.exitScope();
            accepted = true;
            break;

        // ================================================================
        // ERROR
        // ================================================================
        case LRActionType::ERROR:
        default: {
            int lineNum = (tokenPos_ < tokens.size())
                              ? tokens[tokenPos_].line : 0;
            hasError_ = true;
            errorMessage_ = "(LR1 Syntax Error, Line: " +
                            std::to_string(lineNum) +
                            ") at token '" + currentValue() + "'";
            return false;
        }
        }
    }

    return !hasError_;
}

} // namespace PL0
