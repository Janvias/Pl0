/**
 * @file pl0_lr1_parser.hpp
 * @brief PL/0 LR(1) Parser Class
 * @details This class implements a canonical LR(1) shift-reduce parser for
 *          the PL/0 grammar. It constructs LR(1) item sets, builds ACTION
 *          and GOTO tables, and performs bottom-up syntax analysis.
 *          Used alongside the LL(1) recursive descent parser for dual
 *          validation of PL/0 programs.
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
// LR(1) Parser Terminal Symbols
//============================================================================

enum class LR1Terminal {
    tIDENT,          // identifier
    tNUMBER,         // number literal
    tPLUS,           // +
    tMINUS,          // -
    tSTAR,           // *
    tSLASH,          // /
    tEQ,             // = (relop)
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
    tCONST,          // const keyword
    tVAR,            // var keyword
    tPROCEDURE,      // procedure keyword
    tBEGIN,          // begin keyword
    tEND,            // end keyword
    tIF,             // if keyword
    tTHEN,           // then keyword
    tWHILE,          // while keyword
    tDO,             // do keyword
    tCALL,           // call keyword
    tREAD,           // read keyword
    tWRITE,          // write keyword
    tODD,            // odd keyword
    tEOF,            // end of file
    tEPSILON         // epsilon (empty)
};

//============================================================================
// LR(1) Parser Non-Terminal Symbols
//============================================================================

enum class LR1NonTerminal {
    P,               // Program
    B,               // Block
    DL,              // Declaration list
    C,               // Const declaration
    CL,              // Const list tail
    V,               // Var declaration
    VL,              // Var list tail
    PR,              // Procedure declaration
    S,               // Statement
    L,               // Statement list (inside begin-end)
    CO,              // Condition
    E,               // Expression
    ET,              // Expression tail (right-recursive)
    T,               // Term
    TT,              // Term tail (right-recursive)
    F,               // Factor
    A                // Expression argument list (write)
};

//============================================================================
// Grammar Symbol (terminal or non-terminal)
//============================================================================

struct LR1Symbol {
    bool isTerminal;
    union {
        LR1Terminal terminal;
        LR1NonTerminal nonTerminal;
    };

    LR1Symbol() : isTerminal(true), terminal(LR1Terminal::tEPSILON) {}
    LR1Symbol(LR1Terminal t) : isTerminal(true), terminal(t) {}
    LR1Symbol(LR1NonTerminal nt) : isTerminal(false), nonTerminal(nt) {}

    bool operator==(const LR1Symbol& other) const {
        if (isTerminal != other.isTerminal) return false;
        if (isTerminal) return terminal == other.terminal;
        return nonTerminal == other.nonTerminal;
    }

    bool operator<(const LR1Symbol& other) const {
        if (isTerminal != other.isTerminal) return isTerminal < other.isTerminal;
        if (isTerminal) return static_cast<int>(terminal) < static_cast<int>(other.terminal);
        return static_cast<int>(nonTerminal) < static_cast<int>(other.nonTerminal);
    }
};

//============================================================================
// Grammar Production
//============================================================================

struct LR1Production {
    int id;
    LR1NonTerminal lhs;
    std::vector<LR1Symbol> rhs;

    LR1Production() : id(-1), lhs(LR1NonTerminal::P) {}
    LR1Production(int i, LR1NonTerminal l, std::vector<LR1Symbol> r)
        : id(i), lhs(l), rhs(std::move(r)) {}
};

//============================================================================
// LR(1) Item: [A -> alpha . beta, lookahead]
//============================================================================

struct LR1Item {
    int productionId;
    int dot;                // Position of dot (0 = before first symbol)
    LR1Terminal lookahead;  // Lookahead terminal

    LR1Item() : productionId(0), dot(0), lookahead(LR1Terminal::tEOF) {}
    LR1Item(int pid, int d, LR1Terminal la)
        : productionId(pid), dot(d), lookahead(la) {}

    bool operator==(const LR1Item& other) const {
        return productionId == other.productionId &&
               dot == other.dot &&
               lookahead == other.lookahead;
    }

    bool operator<(const LR1Item& other) const {
        if (productionId != other.productionId)
            return productionId < other.productionId;
        if (dot != other.dot) return dot < other.dot;
        return static_cast<int>(lookahead) < static_cast<int>(other.lookahead);
    }
};

//============================================================================
// LR(1) State: a set of LR(1) items
//============================================================================

using LR1State = std::set<LR1Item>;

//============================================================================
// Action types
//============================================================================

enum class LRActionType {
    SHIFT,
    REDUCE,
    ACCEPT,
    ERROR
};

struct LRAction {
    LRActionType type;
    int value;  // For SHIFT: next state; For REDUCE: production id

    LRAction() : type(LRActionType::ERROR), value(-1) {}
    LRAction(LRActionType t, int v) : type(t), value(v) {}

    bool operator==(const LRAction& other) const {
        return type == other.type && value == other.value;
    }
};

//============================================================================
// Parse result node (for building AST / comparison)
//============================================================================

struct LR1ParseNode {
    LR1NonTerminal symbol;
    int productionId;
    std::vector<LR1ParseNode> children;
    Token token;  // For leaf nodes (terminals)

    LR1ParseNode() : symbol(LR1NonTerminal::P), productionId(-1) {}
};

//============================================================================
// Dual Parser Mode
//============================================================================

enum class ParserMode {
    LL1_ONLY,       // Only use LL(1) recursive descent parser
    LR1_ONLY,       // Only use LR(1) shift-reduce parser
    DUAL            // Use both parsers and compare results
};

//============================================================================
// LR1Parser Class
//============================================================================

class LR1Parser {
public:
    LR1Parser();
    ~LR1Parser();

    // Syntax analysis
    bool parse(const std::vector<Token>& tokens);

    // Error handling
    bool hasError() const { return hasError_; }
    const std::string& getErrorMessage() const { return errorMessage_; }

    // Generated code output
    const std::vector<Quadruple>& getQuadruples() const { return quadruples_; }

    // Internal state for diagnostic output
    const std::vector<LR1State>& getStates() const { return states_; }
    void printParseTable(std::ostream& os) const;
    void printStates(std::ostream& os) const;

private:
    // Grammar
    std::vector<LR1Production> productions_;
    std::map<LR1Symbol, std::set<LR1Terminal>> firstSets_;
    std::map<LR1NonTerminal, std::set<LR1Terminal>> followSets_;

    // LR(1) canonical collection
    std::vector<LR1State> states_;

    // Parse tables
    // ACTION[state, terminal] -> action
    std::map<std::pair<int, LR1Terminal>, LRAction> actionTable_;
    // GOTO[state, non-terminal] -> next state
    std::map<std::pair<int, LR1NonTerminal>, int> gotoTable_;

    // Parse result
    bool hasError_;
    std::string errorMessage_;
    std::vector<Quadruple> quadruples_;

    // Token stream and current position
    const std::vector<Token>* tokens_;
    size_t tokenPos_;

    // Symbol table for LR1 parser's own semantic analysis
    SymbolTable symTable_;

    // Grammar initialization
    void initGrammar();
    void computeFirstSets();
    void computeFollowSets();
    std::set<LR1Terminal> computeFirst(const std::vector<LR1Symbol>& symbols, size_t startPos);

    // LR(1) item set construction
    LR1State computeClosure(const LR1State& items);
    void buildCanonicalCollection();

    // Parse table construction
    void buildParseTables();

    // Token-to-terminal mapping
    LR1Terminal tokenToTerminal(const Token& token) const;

    // Terminal string conversion (for diagnostics)
    std::string terminalToString(LR1Terminal t) const;
    std::string nonTerminalToString(LR1NonTerminal nt) const;
    std::string symbolToString(const LR1Symbol& sym) const;
    std::string productionToString(int prodId) const;

    // Semantic actions executed on reduction
    void executeSemanticAction(int productionId);

    // Comparison helpers
    std::string getTokenValue(const Token& token) const;
};

//============================================================================
// Dual Parser Validator
//============================================================================

struct ValidationResult {
    bool bothSucceeded;
    bool resultsMatch;
    std::string ll1Error;
    std::string lr1Error;
    std::string diagnosticInfo;
    std::vector<Quadruple> ll1Quads;
    std::vector<Quadruple> lr1Quads;
};

ValidationResult compareParserResults(
    bool ll1Success, const std::string& ll1Error,
    const std::vector<Quadruple>& ll1Quads,
    bool lr1Success, const std::string& lr1Error,
    const std::vector<Quadruple>& lr1Quads
);

} // namespace PL0

#endif // PL0_LR1_PARSER_HPP
