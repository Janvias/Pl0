/**
 * @file compiler.h
 * @brief PL/0 Compiler Header File
 * @description This header file contains all shared data structures, 
 *              type definitions, and function declarations for the 
 *              PL/0 compiler project.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 2.0
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
 * @struct Token
 * @brief Represents a lexical token with its type, value, and source line
 */
typedef struct {
    TokenType type;               /**< Type of the token */
    char value[MAX_TOKEN_LEN];    /**< String value of the token */
    int line;                     /**< Source line number where token appears */
} Token;

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
 * Symbol Table Include (after basic types are defined)
 *============================================================================*/

#include "symtable/symtable.h"

/*============================================================================
 * Compiler State Structure
 *============================================================================*/

/**
 * @struct CompilerState
 * @brief Contains all compiler state variables, replacing global variables
 */
typedef struct {
    /* Lexer state */
    FILE* fp_in;                  /**< Input file pointer for source code */
    char ch;                      /**< Current character being processed */
    int line;                     /**< Current line number in source file */
    Token token_list[MAX_TOKENS]; /**< Array of all tokens from lexical analysis */
    int token_count;              /**< Total count of tokens */
    int current_token;            /**< Current token index during parsing */
    
    /* Parser/Semantic state */
    SymbolTable* symbol_table;    /**< Symbol table (using linked list implementation) */
    Quad quad_list[MAX_QUADS];    /**< Array of generated quadruples */
    int quad_count;               /**< Total count of quadruples */
    int temp_count;               /**< Counter for temporary variables (T1, T2, ...) */
    int syntax_error;             /**< Flag indicating syntax error occurred */
    int semantic_error;           /**< Flag indicating semantic error occurred */
    int next_instr;               /**< Next instruction address for quadruples */
} CompilerState;

/*============================================================================
 * Function Declarations - Lexical Analysis (lexer.c)
 *============================================================================*/

/**
 * @brief Read next character from input file
 * @param state Compiler state
 * @description Updates state->ch and increments line counter on newline
 */
void get_char(CompilerState* state);

/**
 * @brief Skip whitespace characters
 * @param state Compiler state
 * @description Advances past spaces, tabs, and newlines
 */
void skip_white(CompilerState* state);

/**
 * @brief Check if string is a PL/0 keyword
 * @param s String to check
 * @return 1 if keyword, 0 otherwise
 */
int is_keyword(char *s);

/**
 * @brief Skip single-line comment starting with double slash
 * @param state Compiler state
 * @description Advances until newline or EOF
 */
void skip_single_comment(CompilerState* state);

/**
 * @brief Skip multi-line comment delimited by slash-star
 * @param state Compiler state
 * @description Handles nested content and reports unclosed comments
 */
void skip_multi_comment(CompilerState* state);

/**
 * @brief Get next token from source file
 * @param state Compiler state
 * @return Token structure with type, value, and line number
 */
Token get_token(CompilerState* state);

/**
 * @brief Perform complete lexical analysis
 * @param state Compiler state
 * @description Reads entire source file and populates token_list
 */
void lexical_analysis(CompilerState* state);

/**
 * @brief Get current token being processed
 * @param state Compiler state
 * @return Current token from token_list
 */
Token current_t(CompilerState* state);

/**
 * @brief Match current token against expected type and value
 * @param state Compiler state
 * @param type Expected token type
 * @param value Expected token value
 * @description Advances to next token if match succeeds
 */
void match(CompilerState* state, TokenType type, const char *value);

/**
 * @brief Advance to next token in token list
 * @param state Compiler state
 */
void next_token_func(CompilerState* state);

/**
 * @brief Print all tokens to console
 * @param state Compiler state
 * @description Format: (type,value) for each token
 */
void print_tokens(CompilerState* state);

/**
 * @brief Print token statistics summary
 * @param state Compiler state
 * @description Shows counts for each token category
 */
void print_statistics(CompilerState* state);

/**
 * @brief Print word classification table
 * @param state Compiler state
 * @description Shows detailed classification of all tokens with line numbers
 */
void print_classification_table(CompilerState* state);

/**
 * @brief Print state transition diagram (text-based)
 * @description Shows the finite state machine for lexical analysis
 */
void print_state_transition_diagram();

/**
 * @brief Print lexical recognition flowchart (text-based)
 * @description Shows the flow of token recognition process
 */
void print_recognition_flowchart();

/*============================================================================
 * Function Declarations - Syntax/Semantic Analysis (parser.c)
 *============================================================================*/

/**
 * @brief Generate a new temporary variable name
 * @param state Compiler state
 * @param temp Buffer to store generated name (T1, T2, etc.)
 */
void new_temp(CompilerState* state, char *temp);

/**
 * @brief Generate and add a quadruple to the list
 * @param state Compiler state
 * @param op Operator
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param result Result destination
 */
void gen_quad(CompilerState* state, char *op, char *arg1, char *arg2, char *result);

/**
 * @brief Parse a factor (identifier, number, or parenthesized expression)
 * @param state Compiler state
 * @param result Buffer to store factor value
 */
void factor(CompilerState* state, char *result);

/**
 * @brief Parse a term (factor with multiplication/division)
 * @param state Compiler state
 * @param result Buffer to store term value
 */
void term(CompilerState* state, char *result);

/**
 * @brief Parse an expression (term with addition/subtraction)
 * @param state Compiler state
 * @param result Buffer to store expression value
 */
void expression(CompilerState* state, char *result);

/**
 * @brief Parse a condition for if/while statements
 * @param state Compiler state
 * @param true_list Pointer to store true jump index
 * @param false_list Pointer to store false jump index
 */
void condition(CompilerState* state, int *true_list, int *false_list);

/**
 * @brief Parse a statement
 * @param state Compiler state
 * @description Handles assignment, compound, if, while, call, read, write
 */
void statement(CompilerState* state);

/**
 * @brief Parse a statement list (sequence of statements)
 * @param state Compiler state
 */
void statement_list(CompilerState* state);

/**
 * @brief Parse constant declaration section
 * @param state Compiler state
 */
void const_declaration(CompilerState* state);

/**
 * @brief Parse variable declaration section
 * @param state Compiler state
 */
void var_declaration(CompilerState* state);

/**
 * @brief Parse procedure declaration section
 * @param state Compiler state
 */
void procedure_declaration(CompilerState* state);

/**
 * @brief Parse a block (declarations + statements)
 * @param state Compiler state
 */
void block(CompilerState* state);

/**
 * @brief Parse entire program
 * @param state Compiler state
 */
void program(CompilerState* state);

/**
 * @brief Print all quadruples to console
 * @param state Compiler state
 */
void print_quad(CompilerState* state);

/**
 * @brief Write all quadruples to output file
 * @param state Compiler state
 * @param fp File pointer for output
 */
void print_quad_to_file(CompilerState* state, FILE *fp);

/**
 * @brief Print symbol table to console
 * @param state Compiler state
 */
void print_symbol_table(CompilerState* state);

/**
 * @brief Write compiler state to cache file
 * @param state Compiler state
 * @param filename Output cache filename
 * @return true if successful, false otherwise
 */
bool write_compiler_state_to_cache(CompilerState* state, const char* filename);

/**
 * @brief Read compiler state from cache file
 * @param state Compiler state (will be initialized)
 * @param filename Input cache filename
 * @return true if successful, false otherwise
 */
bool read_compiler_state_from_cache(CompilerState* state, const char* filename);

/**
 * @brief Initialize compiler state
 * @param state Compiler state to initialize
 * @param fp_in Input file pointer
 * @return true if successful, false otherwise
 */
bool init_compiler_state(CompilerState* state, FILE* fp_in);

/**
 * @brief Destroy compiler state and free resources
 * @param state Compiler state to destroy
 */
void destroy_compiler_state(CompilerState* state);

#endif /* COMPILER_H */