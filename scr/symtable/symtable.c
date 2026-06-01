/**
 * @file symtable.c
 * @brief Symbol Table Management Module Implementation
 * 
 * This module implements a symbol table that stores symbols in scope order,
 * supporting nested scopes and scope chain lookup.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#include "symtable.h"
#include "../compiler.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Creates a new symbol
 */
static Symbol* symbol_create(const char* name, SymbolKind kind, int val, 
                             int level, int address, int quad_index) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    if (!symbol) {
        return NULL;
    }
    strncpy(symbol->name, name, MAX_TOKEN_LEN - 1);
    symbol->name[MAX_TOKEN_LEN - 1] = '\0';
    symbol->kind = kind;
    symbol->val = val;
    symbol->level = level;
    symbol->address = address;
    symbol->quad_index = quad_index;
    return symbol;
}

/**
 * @brief Destroys a symbol
 */
static void symbol_destroy(Symbol* symbol) {
    if (symbol) {
        free(symbol);
    }
}

/**
 * @brief Creates a new scope
 */
static Scope* scope_create(int level, const char* procedure_name) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    if (!scope) {
        return NULL;
    }
    scope->level = level;
    if (procedure_name) {
        strncpy(scope->procedure_name, procedure_name, MAX_TOKEN_LEN - 1);
        scope->procedure_name[MAX_TOKEN_LEN - 1] = '\0';
    } else {
        scope->procedure_name[0] = '\0';
    }
    scope->symbols = list_create((void (*)(void*))symbol_destroy);
    if (!scope->symbols) {
        free(scope);
        return NULL;
    }
    return scope;
}

/**
 * @brief Destroys a scope
 */
static void scope_destroy(Scope* scope) {
    if (scope) {
        list_destroy(scope->symbols);
        free(scope);
    }
}

/**
 * @brief Creates a new symbol table
 */
SymbolTable* symtable_create(void) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) {
        return NULL;
    }
    
    table->scopes = list_create((void (*)(void*))scope_destroy);
    if (!table->scopes) {
        free(table);
        return NULL;
    }
    
    // Create the global scope (level 0)
    Scope* global_scope = scope_create(0, NULL);
    if (!global_scope) {
        list_destroy(table->scopes);
        free(table);
        return NULL;
    }
    
    list_append(table->scopes, global_scope);
    table->current_level = 0;
    table->next_address = 0;
    
    return table;
}

/**
 * @brief Destroys a symbol table and frees all allocated memory
 */
void symtable_destroy(SymbolTable* table) {
    if (!table) {
        return;
    }
    list_destroy(table->scopes);
    free(table);
}

/**
 * @brief Enters a new scope (called when entering a procedure)
 */
bool symtable_enter_scope(SymbolTable* table, const char* procedure_name) {
    if (!table) {
        return false;
    }
    
    Scope* new_scope = scope_create(table->current_level + 1, procedure_name);
    if (!new_scope) {
        return false;
    }
    
    list_append(table->scopes, new_scope);
    table->current_level++;
    table->next_address = 0;  // Reset address counter for new scope
    
    return true;
}

/**
 * @brief Exits the current scope (called when leaving a procedure)
 */
bool symtable_exit_scope(SymbolTable* table) {
    if (!table || table->current_level == 0) {
        return false;
    }
    
    // Remove the last scope from the list
    Scope* scope = (Scope*)list_remove_last(table->scopes);
    if (scope) {
        scope_destroy(scope);
        table->current_level--;
        
        // Update next_address based on previous scope's usage
        Scope* current_scope = (Scope*)list_get_last(table->scopes);
        if (current_scope) {
            table->next_address = list_size(current_scope->symbols);
        } else {
            table->next_address = 0;
        }
    }
    
    return true;
}

/**
 * @brief Inserts a new symbol into the current scope
 */
bool symtable_insert(SymbolTable* table, const char* name, SymbolKind kind, 
                     int val, int address, int quad_index) {
    if (!table || !name) {
        return false;
    }
    
    // Check if symbol already exists in current scope
    if (symtable_exists_in_current_scope(table, name)) {
        return false;
    }
    
    Scope* current_scope = (Scope*)list_get_last(table->scopes);
    if (!current_scope) {
        return false;
    }
    
    Symbol* symbol = symbol_create(name, kind, val, table->current_level, 
                                   address, quad_index);
    if (!symbol) {
        return false;
    }
    
    return list_append(current_scope->symbols, symbol);
}

/**
 * @brief Looks up a symbol in the symbol table (scope chain search)
 */
Symbol* symtable_lookup(SymbolTable* table, const char* name) {
    if (!table || !name) {
        return NULL;
    }
    
    // Search from innermost to outermost scope
    int num_scopes = list_size(table->scopes);
    for (int i = num_scopes - 1; i >= 0; i--) {
        Scope* scope = (Scope*)list_get(table->scopes, i);
        if (!scope) {
            continue;
        }
        
        // Search through symbols in this scope
        List* symbols = scope->symbols;
        int num_symbols = list_size(symbols);
        for (int j = 0; j < num_symbols; j++) {
            Symbol* symbol = (Symbol*)list_get(symbols, j);
            if (symbol && strcmp(symbol->name, name) == 0) {
                return symbol;
            }
        }
    }
    
    return NULL;
}

/**
 * @brief Checks if a symbol exists in the current scope only
 */
bool symtable_exists_in_current_scope(SymbolTable* table, const char* name) {
    if (!table || !name) {
        return false;
    }
    
    Scope* current_scope = (Scope*)list_get_last(table->scopes);
    if (!current_scope) {
        return false;
    }
    
    List* symbols = current_scope->symbols;
    int num_symbols = list_size(symbols);
    for (int i = 0; i < num_symbols; i++) {
        Symbol* symbol = (Symbol*)list_get(symbols, i);
        if (symbol && strcmp(symbol->name, name) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Returns the current nesting level
 */
int symtable_get_current_level(SymbolTable* table) {
    if (!table) {
        return -1;
    }
    return table->current_level;
}

/**
 * @brief Returns the next available address offset for variables
 */
int symtable_get_next_address(SymbolTable* table) {
    if (!table) {
        return -1;
    }
    return table->next_address;
}

/**
 * @brief Increments the next address counter
 */
void symtable_increment_address(SymbolTable* table) {
    if (table) {
        table->next_address++;
    }
}

/**
 * @brief Prints the symbol table contents to a file
 */
void symtable_print(SymbolTable* table, FILE* fp) {
    if (!table || !fp) {
        return;
    }
    
    fprintf(fp, "===== SYMBOL TABLE =====\n");
    fprintf(fp, "Name\t\tKind\t\tValue\tLevel\tAddress\n");
    fprintf(fp, "----\t\t----\t\t-----\t-----\t-------\n");
    
    int num_scopes = list_size(table->scopes);
    for (int i = 0; i < num_scopes; i++) {
        Scope* scope = (Scope*)list_get(table->scopes, i);
        if (!scope) {
            continue;
        }
        
        if (scope->level > 0 && scope->procedure_name[0] != '\0') {
            fprintf(fp, "--- Scope %d: procedure '%s' ---\n", 
                    scope->level, scope->procedure_name);
        } else if (scope->level == 0) {
            fprintf(fp, "--- Global Scope ---\n");
        }
        
        List* symbols = scope->symbols;
        int num_symbols = list_size(symbols);
        for (int j = 0; j < num_symbols; j++) {
            Symbol* symbol = (Symbol*)list_get(symbols, j);
            if (!symbol) {
                continue;
            }
            
            const char* kind_str;
            switch (symbol->kind) {
                case CONST_KIND:
                    kind_str = "const";
                    break;
                case VAR_KIND:
                    kind_str = "var";
                    break;
                case PROCEDURE_KIND:
                    kind_str = "procedure";
                    break;
                default:
                    kind_str = "unknown";
            }
            
            fprintf(fp, "%-16s %-12s %-7d %-5d %d\n",
                    symbol->name, kind_str, symbol->val, 
                    symbol->level, symbol->address);
        }
    }
    
    fprintf(fp, "===== END SYMBOL TABLE =====\n");
}

/**
 * @brief Prints the symbol table to stdout
 */
void symtable_dump(SymbolTable* table) {
    symtable_print(table, stdout);
}