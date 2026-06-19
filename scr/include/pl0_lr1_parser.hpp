/**
 * @file pl0_lr1_parser.hpp
 * @brief PL/0 LR(1)解析器类
 * @details 本类实现PL/0语法的规范LR(1)移进-归约解析器。
 *          构建LR(1)项目集族、ACTION表和GOTO表，执行自底向上的语法分析。
 *          与LL(1)递归下降解析器配合使用，实现PL/0程序的双解析器验证。
 * @author PL/0 Compiler Project
 * @date 2026-06-09
 */

#ifndef PL0_LR1_PARSER_HPP
#define PL0_LR1_PARSER_HPP

#include "pl0_core.hpp"
#include "pl0_lexer.hpp"
#include "pl0_symtable.hpp"
#include "pl0_codegen.hpp"

#include <map>
#include <set>
#include <deque>

namespace PL0 {

//============================================================================
// LR(1)解析器终结符
//============================================================================

enum class LR1Terminal {
    tIDENT,          // 标识符
    tNUMBER,         // 数字常量
    tPLUS,           // +
    tMINUS,          // -
    tSTAR,           // *
    tSLASH,          // /
    tEQ,             // = (关系运算符)
    tNEQ,            // #
    tLT,             // <
    tLTE,            // <=
    tGT,             // >
    tGTE,            // >=
    tASSIGN,         // :=
    tLPAREN,         // (
    tRPAREN,         // )
    tDOT,            // .
    tCOMMA,          // ,
    tSEMICOLON,      // ;
    tCONST,          // const关键字
    tVAR,            // var关键字
    tPROCEDURE,      // procedure关键字
    tBEGIN,          // begin关键字
    tEND,            // end关键字
    tIF,             // if关键字
    tTHEN,           // then关键字
    tWHILE,          // while关键字
    tDO,             // do关键字
    tCALL,           // call关键字
    tREAD,           // read关键字
    tWRITE,          // write关键字
    tODD,            // odd关键字
    tEOF,            // 文件结束
    tEPSILON         // ε（空串）
};

//============================================================================
// LR(1)解析器非终结符
//============================================================================

enum class LR1NonTerminal {
    P,               // 程序
    B,               // 分程序
    DL,              // 声明列表
    C,               // 常量声明
    CL,              // 常量列表尾部
    V,               // 变量声明
    VL,              // 变量列表尾部
    PR,              // 过程声明
    S,               // 语句
    L,               // 语句列表（begin-end内部）
    CO,              // 条件
    E,               // 表达式
    ET,              // 表达式尾部（右递归）
    T,               // 项
    TT,              // 项尾部（右递归）
    F,               // 因子
    A                // 表达式参数列表（write）
};

//============================================================================
// 文法符号（终结符或非终结符）
//============================================================================

struct LR1Symbol {
    bool isTerminal;  // 是否为终结符
    union {
        LR1Terminal terminal;      // 终结符值
        LR1NonTerminal nonTerminal; // 非终结符值
    };

    LR1Symbol() : isTerminal(true), terminal(LR1Terminal::tEPSILON) {}
    LR1Symbol(LR1Terminal t) : isTerminal(true), terminal(t) {}
    LR1Symbol(LR1NonTerminal nt) : isTerminal(false), nonTerminal(nt) {}

    // 等价比较运算符
    bool operator==(const LR1Symbol& other) const {
        if (isTerminal != other.isTerminal) return false;
        if (isTerminal) return terminal == other.terminal;
        return nonTerminal == other.nonTerminal;
    }

    // 小于比较运算符（用于set/map排序）
    bool operator<(const LR1Symbol& other) const {
        if (isTerminal != other.isTerminal) return isTerminal < other.isTerminal;
        if (isTerminal) return static_cast<int>(terminal) < static_cast<int>(other.terminal);
        return static_cast<int>(nonTerminal) < static_cast<int>(other.nonTerminal);
    }
};

//============================================================================
// 文法产生式
//============================================================================

struct LR1Production {
    int id;                          // 产生式编号
    LR1NonTerminal lhs;              // 左部（非终结符）
    std::vector<LR1Symbol> rhs;      // 右部符号列表

    LR1Production() : id(-1), lhs(LR1NonTerminal::P) {}
    LR1Production(int i, LR1NonTerminal l, std::vector<LR1Symbol> r)
        : id(i), lhs(l), rhs(std::move(r)) {}
};

//============================================================================
// LR(1)项目: [A -> alpha . beta, lookahead]
//============================================================================

struct LR1Item {
    int productionId;         // 产生式编号
    int dot;                  // 点的位置（0 = 第一个符号之前）
    LR1Terminal lookahead;    // 向前看终结符

    LR1Item() : productionId(0), dot(0), lookahead(LR1Terminal::tEOF) {}
    LR1Item(int pid, int d, LR1Terminal la)
        : productionId(pid), dot(d), lookahead(la) {}

    // 等价比较运算符
    bool operator==(const LR1Item& other) const {
        return productionId == other.productionId &&
               dot == other.dot &&
               lookahead == other.lookahead;
    }

    // 小于比较运算符（用于set排序）
    bool operator<(const LR1Item& other) const {
        if (productionId != other.productionId)
            return productionId < other.productionId;
        if (dot != other.dot) return dot < other.dot;
        return static_cast<int>(lookahead) < static_cast<int>(other.lookahead);
    }
};

//============================================================================
// LR(1)状态：LR(1)项目的集合
//============================================================================

using LR1State = std::set<LR1Item>;

//============================================================================
// 动作类型
//============================================================================

enum class LRActionType {
    SHIFT,    // 移进
    REDUCE,   // 归约
    ACCEPT,   // 接受
    ERROR     // 错误
};

struct LRAction {
    LRActionType type;  // 动作类型
    int value;          // SHIFT:下一状态; REDUCE:产生式编号

    LRAction() : type(LRActionType::ERROR), value(-1) {}
    LRAction(LRActionType t, int v) : type(t), value(v) {}

    bool operator==(const LRAction& other) const {
        return type == other.type && value == other.value;
    }
};

//============================================================================
// 解析结果节点（用于构建AST/比较）
//============================================================================

struct LR1ParseNode {
    LR1NonTerminal symbol;          // 非终结符符号
    int productionId;               // 使用的产生式编号
    std::vector<LR1ParseNode> children;  // 子节点列表
    Token token;                    // 叶节点（终结符）的Token

    LR1ParseNode() : symbol(LR1NonTerminal::P), productionId(-1) {}
};

//============================================================================
// 双解析器模式
//============================================================================

enum class ParserMode {
    LL1_ONLY,       // 仅使用LL(1)递归下降解析器
    LR1_ONLY,       // 仅使用LR(1)移进-归约解析器
    DUAL            // 使用双解析器并比较结果
};

//============================================================================
// LR1Parser类
//============================================================================

class LR1Parser {
public:
    LR1Parser();
    ~LR1Parser();

    //============================================================================
    // 语法分析
    //============================================================================

    /** 执行语法分析 */
    bool parse(const std::vector<Token>& tokens);

    //============================================================================
    // 错误处理
    //============================================================================

    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

    //============================================================================
    // 生成的代码输出
    //============================================================================

    const std::vector<Quadruple>& getQuadruples() const { return quadruples_; }

    //============================================================================
    // 内部状态诊断输出
    //============================================================================

    const std::vector<LR1State>& getStates() const { return states_; }
    void printParseTable(std::ostream& os) const;
    void printStates(std::ostream& os) const;

private:
    //============================================================================
    // 文法数据
    //============================================================================

    std::vector<LR1Production> productions_;  // 产生式列表
    std::map<LR1Symbol, std::set<LR1Terminal>> firstSets_;  // FIRST集
    std::map<LR1NonTerminal, std::set<LR1Terminal>> followSets_;  // FOLLOW集

    //============================================================================
    // LR(1)规范集族
    //============================================================================

    std::vector<LR1State> states_;  // 状态集合

    //============================================================================
    // 解析表
    //============================================================================

    // ACTION[state, terminal] -> action
    std::map<std::pair<int, LR1Terminal>, LRAction> actionTable_;
    // GOTO[state, non-terminal] -> next state
    std::map<std::pair<int, LR1NonTerminal>, int> gotoTable_;

    //============================================================================
    // 解析结果
    //============================================================================

    bool hasError_;
    std::string errorMessage_;
    std::vector<Quadruple> quadruples_;

    //============================================================================
    // Token流和当前位置
    //============================================================================

    const std::vector<Token>* tokens_;
    size_t tokenPos_;

    //============================================================================
    // 符号表（LR1解析器语义分析专用）
    //============================================================================

    SymbolTable symTable_;

    //============================================================================
    // 文法初始化
    //============================================================================

    void initGrammar();  // 初始化文法
    void computeFirstSets();  // 计算FIRST集
    void computeFollowSets();  // 计算FOLLOW集
    std::set<LR1Terminal> computeFirst(const std::vector<LR1Symbol>& symbols, size_t startPos);  // 计算符号串FIRST集

    //============================================================================
    // LR(1)项目集构造
    //============================================================================

    LR1State computeClosure(const LR1State& items);  // 计算闭包
    void buildCanonicalCollection();  // 构建规范集族

    //============================================================================
    // 解析表构造
    //============================================================================

    void buildParseTables();  // 构建ACTION和GOTO表

    //============================================================================
    // Token到终结符映射
    //============================================================================

    LR1Terminal tokenToTerminal(const Token& token) const;  // Token转终结符

    //============================================================================
    // 终结符字符串转换（诊断输出）
    //============================================================================

    std::string terminalToString(LR1Terminal t) const;  // 终结符转字符串
    std::string nonTerminalToString(LR1NonTerminal nt) const;  // 非终结符转字符串
    std::string symbolToString(const LR1Symbol& sym) const;  // 符号转字符串
    std::string productionToString(int prodId) const;  // 产生式转字符串

    //============================================================================
    // 归约时执行的语义动作
    //============================================================================

    void executeSemanticAction(int productionId);  // 执行语义动作

    //============================================================================
    // 比较辅助函数
    //============================================================================

    std::string getTokenValue(const Token& token) const;  // 获取Token值
};

//============================================================================
// 双解析器验证结果
//============================================================================

struct ValidationResult {
    bool bothSucceeded;              // 两个解析器都成功
    bool resultsMatch;               // 结果匹配
    std::string ll1Error;            // LL(1)错误消息
    std::string lr1Error;            // LR(1)错误消息
    std::string diagnosticInfo;      // 诊断信息
    std::vector<Quadruple> ll1Quads; // LL(1)四元式
    std::vector<Quadruple> lr1Quads; // LR(1)四元式
};

/** 比较双解析器结果 */
ValidationResult compareParserResults(
    bool ll1Success, const std::string& ll1Error,
    const std::vector<Quadruple>& ll1Quads,
    bool lr1Success, const std::string& lr1Error,
    const std::vector<Quadruple>& lr1Quads
);

} // namespace PL0

#endif // PL0_LR1_PARSER_HPP
