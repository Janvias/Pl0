/**
 * @file symtable.h
 * @brief Symbol Table Management Module Header
 * 
 * This module provides a symbol table implementation that stores symbols
 * in scope order. It supports nested scopes (procedure nesting) and
 * implements scope chain lookup for symbol resolution.
 * 
 * Features:
 *   - Scope-based symbol storage
 *   - Support for constants, variables, and procedures
 *   - Scope chain lookup (outer scope access)
 *   - Proper handling of nested procedures
 *   - Symbol insertion and deletion
 *   - Duplicate declaration detection
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include <stdio.h>
#include "../list/list.h"

/*============================================================================
 * Constants
 *============================================================================*/

/** Maximum length of a token string (matching compiler.h) */
#define SYMTABLE_MAX_TOKEN_LEN 100

/*============================================================================
 * Type Definitions
 *============================================================================*/

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
 * @struct Symbol
 * @brief Represents an entry in the symbol table
 */
typedef struct {
    char name[SYMTABLE_MAX_TOKEN_LEN];  /**< Name of the symbol */
    SymbolKind kind;                     /**< Kind of symbol (const, var, procedure) */
    int val;                             /**< Value (for constants) */
    int level;                           /**< Nesting level (scope depth) */
    int address;                         /**< Address offset in memory */
    int quad_index;                      /**< Quadruple index (for procedures) */
} Symbol;

/**
 * @struct Scope
 * @brief Represents a scope level in the symbol table
 */
typedef struct {
    int level;                           /**< Nesting level of this scope */
    char procedure_name[SYMTABLE_MAX_TOKEN_LEN];  /**< Name of procedure */
    List* symbols;                       /**< List of symbols in this scope */
} Scope;

/**
 * @struct SymbolTable
 * @brief Represents the complete symbol table with multiple scopes
 */
typedef struct SymbolTable {
    List* scopes;                        /**< List of scopes in order of creation */
    int current_level;                   /**< Current nesting level */
    int next_address;                    /**< Next available address offset */
} SymbolTable;

/*============================================================================
 * Symbol Table Operations
 *============================================================================*/

/**
 * @brief Creates a new symbol table
 * 
 * @return Pointer to the newly created symbol table, or NULL on failure
 */
SymbolTable* symtable_create(void);

/**
 * @brief Destroys a symbol table and frees all allocated memory
 * 
 * @param table Pointer to the symbol table to destroy
 */
void symtable_destroy(SymbolTable* table);

/**
 * @brief Enters a new scope (called when entering a procedure)
 * 
 * @param table Pointer to the symbol table
 * @param procedure_name Name of the procedure (NULL for main program)
 * @return true if successful, false otherwise
 */
bool symtable_enter_scope(SymbolTable* table, const char* procedure_name);

/**
 * @brief Exits the current scope (called when leaving a procedure)
 * 
 * @param table Pointer to the symbol table
 * @return true if successful, false if already at global scope
 */
bool symtable_exit_scope(SymbolTable* table);

/**
 * @brief Inserts a new symbol into the current scope
 * 
 * @param table Pointer to the symbol table
 * @param name Name of the symbol
 * @param kind Kind of symbol (const, var, procedure)
 * @param val Value (for constants)
 * @param address Address offset (for variables)
 * @param quad_index Quadruple index (for procedures)
 * @return true if successful, false if symbol already exists in current scope
 */
bool symtable_insert(SymbolTable* table, const char* name, SymbolKind kind, 
                     int val, int address, int quad_index);

/**
 * @brief Looks up a symbol in the symbol table (scope chain search)
 * 
 * Searches for the symbol starting from the current scope and moving
 * outward through parent scopes.
 * 
 * @param table Pointer to the symbol table
 * @param name Name of the symbol to find
 * @return Pointer to the symbol if found, NULL otherwise
 */
Symbol* symtable_lookup(SymbolTable* table, const char* name);

/**
 * @brief Checks if a symbol exists in the current scope only
 * 
 * @param table Pointer to the symbol table
 * @param name Name of the symbol to check
 * @return true if exists in current scope, false otherwise
 */
bool symtable_exists_in_current_scope(SymbolTable* table, const char* name);

/**
 * @brief Returns the current nesting level
 * 
 * @param table Pointer to the symbol table
 * @return Current nesting level
 */
int symtable_get_current_level(SymbolTable* table);

/**
 * @brief Returns the next available address offset for variables
 * 
 * @param table Pointer to the symbol table
 * @return Next available address
 */
int symtable_get_next_address(SymbolTable* table);

/**
 * @brief Increments the next address counter
 * 
 * @param table Pointer to the symbol table
 */
void symtable_increment_address(SymbolTable* table);

/**
 * @brief Prints the symbol table contents to a file
 * 
 * @param table Pointer to the symbol table
 * @param fp Output file pointer
 */
void symtable_print(SymbolTable* table, FILE* fp);

/**
 * @brief Prints the symbol table to stdout
 * 
 * @param table Pointer to the symbol table
 */
void symtable_dump(SymbolTable* table);

#endif /* SYMTABLE_H */