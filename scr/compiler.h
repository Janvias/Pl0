/**
 * @file compiler.h
 * @brief PL/0 Compiler Header File
 * @description This header file contains all shared data structures, 
 *              type definitions, and function declarations for the 
 *              PL/0 compiler project.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-05-30
 * @version 1.0
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*============================================================================
 * Constants and Configuration
 *============================================================================*/

/** Maximum length of a token string */
#define MAX_TOKEN_LEN 100

/** Maximum number of tokens from lexical analysis */
#define MAX_TOKENS 1000

/** Maximum number of quadruples in intermediate code */
#define MAX_QUADS 1000

/** Maximum number of symbols in symbol table */
#define MAX_SYMBOLS 100

/*============================================================================
 * Type Definitions
 *============================================================================*/

/**
 * @enum TokenType
 * @brief Enumeration of token types recognized by the lexer
 */
typedef enum {
    KEYWORD,      /**< Reserved keywords (const, var, procedure, etc.) */
    IDENTIFIER,   /**< User-defined identifiers */
    NUMBER,       /**< Unsigned integer literals */
    OPERATOR,     /**< Arithmetic and relational operators */
    DELIMITER,    /**< Punctuation marks (semicolon, comma, etc.) */
    ERROR         /**< Invalid or erroneous tokens */
} TokenType;

/**
 * @enum SymbolKind
 * @brief Enumeration of symbol kinds in the symbol table
 */
typedef enum {
    CONST_KIND,      /**< Constant declaration */
    VAR_KIND,        /**< Variable declaration */
    PROCEDURE_KIND   /**< Procedure declaration */
} SymbolKind;

/**
 * @struct Token
 * @brief Represents a lexical token with its type, value, and source line
 */
typedef struct {
    TokenType type;               /**< Type of the token */
    char value[MAX_TOKEN_LEN];    /**< String value of the token */
    int line;                     /**< Source line number where token appears */
} Token;

/**
 * @struct Symbol
 * @brief Represents an entry in the symbol table
 */
typedef struct {
    char name[MAX_TOKEN_LEN];     /**< Name of the symbol */
    SymbolKind kind;              /**< Kind of symbol (const, var, procedure) */
    int val;                      /**< Value (for constants) */
    int level;                    /**< Nesting level (scope depth) */
    int address;                  /**< Address offset in memory */
} Symbol;

/**
 * @struct Quad
 * @brief Represents a quadruple in intermediate code generation
 * @description Format: (op, arg1, arg2, result)
 */
typedef struct {
    char op[MAX_TOKEN_LEN];       /**< Operator */
    char arg1[MAX_TOKEN_LEN];     /**< First argument */
    char arg2[MAX_TOKEN_LEN];     /**< Second argument */
    char result[MAX_TOKEN_LEN];   /**< Result destination */
} Quad;

/*============================================================================
 * Global Variables (External Declarations)
 *============================================================================*/

/** Input file pointer for source code */
extern FILE *fp_in;

/** Current character being processed */
extern char ch;

/** Current line number in source file */
extern int line;

/** Array of all tokens from lexical analysis */
extern Token token_list[MAX_TOKENS];

/** Total count of tokens */
extern int token_count;

/** Current token index during parsing */
extern int current_token;

/** Symbol table array */
extern Symbol symbol_table[MAX_SYMBOLS];

/** Total count of symbols in table */
extern int symbol_count;

/** Current nesting level (scope depth) */
extern int current_level;

/** Array of generated quadruples */
extern Quad quad_list[MAX_QUADS];

/** Total count of quadruples */
extern int quad_count;

/** Counter for temporary variables (T1, T2, ...) */
extern int temp_count;

/** Flag indicating syntax error occurred */
extern int syntax_error;

/** Flag indicating semantic error occurred */
extern int semantic_error;

/** Next instruction address for quadruples */
extern int next_instr;

/*============================================================================
 * Function Declarations - Lexical Analysis (lexer.c)
 *============================================================================*/

/**
 * @brief Read next character from input file
 * @description Updates global variable ch and increments line counter on newline
 */
void get_char();

/**
 * @brief Skip whitespace characters
 * @description Advances past spaces, tabs, and newlines
 */
void skip_white();

/**
 * @brief Check if string is a PL/0 keyword
 * @param s String to check
 * @return 1 if keyword, 0 otherwise
 */
int is_keyword(char *s);

/**
 * @brief Skip single-line comment starting with double slash
 * @description Advances until newline or EOF
 */
void skip_single_comment();

/**
 * @brief Skip multi-line comment delimited by slash-star
 * @description Handles nested content and reports unclosed comments
 */
void skip_multi_comment();

/**
 * @brief Get next token from source file
 * @return Token structure with type, value, and line number
 */
Token get_token();

/**
 * @brief Perform complete lexical analysis
 * @description Reads entire source file and populates token_list
 */
void lexical_analysis();

/**
 * @brief Get current token being processed
 * @return Current token from token_list
 */
Token current_t();

/**
 * @brief Match current token against expected type and value
 * @param type Expected token type
 * @param value Expected token value
 * @description Advances to next token if match succeeds
 */
void match(TokenType type, const char *value);

/**
 * @brief Advance to next token in token list
 */
void next_token_func();

/**
 * @brief Print all tokens to console
 * @description Format: (type,value) for each token
 */
void print_tokens();

/**
 * @brief Print token statistics summary
 * @description Shows counts for each token category
 */
void print_statistics();

/*============================================================================
 * Function Declarations - Syntax/Semantic Analysis (parser.c)
 *============================================================================*/

/**
 * @brief Look up a symbol in the symbol table
 * @param name Symbol name to search
 * @return Index if found, -1 if not found
 */
int lookup_symbol(char *name);

/**
 * @brief Add a new symbol to the symbol table
 * @param name Symbol name
 * @param kind Symbol kind (CONST_KIND, VAR_KIND, PROCEDURE_KIND)
 * @param val Value for constants
 */
void add_symbol(char *name, SymbolKind kind, int val);

/**
 * @brief Generate a new temporary variable name
 * @param temp Buffer to store generated name (T1, T2, etc.)
 */
void new_temp(char *temp);

/**
 * @brief Generate and add a quadruple to the list
 * @param op Operator
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param result Result destination
 */
void gen_quad(char *op, char *arg1, char *arg2, char *result);

/**
 * @brief Parse a factor (identifier, number, or parenthesized expression)
 * @param result Buffer to store factor value
 */
void factor(char *result);

/**
 * @brief Parse a term (factor with multiplication/division)
 * @param result Buffer to store term value
 */
void term(char *result);

/**
 * @brief Parse an expression (term with addition/subtraction)
 * @param result Buffer to store expression value
 */
void expression(char *result);

/**
 * @brief Parse a condition for if/while statements
 * @param true_list Pointer to store true jump index
 * @param false_list Pointer to store false jump index
 */
void condition(int *true_list, int *false_list);

/**
 * @brief Parse a statement
 * @description Handles assignment, compound, if, while, call, read, write
 */
void statement();

/**
 * @brief Parse a statement list (sequence of statements)
 */
void statement_list();

/**
 * @brief Parse constant declaration section
 */
void const_declaration();

/**
 * @brief Parse variable declaration section
 */
void var_declaration();

/**
 * @brief Parse procedure declaration section
 */
void procedure_declaration();

/**
 * @brief Parse a block (declarations + statements)
 */
void block();

/**
 * @brief Parse entire program
 */
void program();

/**
 * @brief Print all quadruples to console
 */
void print_quad();

/**
 * @brief Write all quadruples to output file
 * @param fp File pointer for output
 */
void print_quad_to_file(FILE *fp);

/**
 * @brief Print symbol table to console
 */
void print_symbol_table();

#endif /* COMPILER_H */