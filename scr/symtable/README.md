# Symbol Table Management Module

This directory contains a symbol table implementation for the PL/0 compiler project.

## Files

- `symtable.h` - Header file with symbol table data structures and function declarations
- `symtable.c` - Implementation of symbol table operations

## Features

- **Scope-based storage**: Symbols are stored in scope order
- **Nested scope support**: Proper handling of procedure nesting
- **Scope chain lookup**: Search from innermost to outermost scope
- **Duplicate declaration detection**: Prevents redeclaration in same scope
- **Multiple symbol types**: Supports constants, variables, and procedures

## Usage Example

```c
#include "symtable/symtable.h"
#include <stdio.h>

int main() {
    // Create symbol table
    SymbolTable* table = symtable_create();
    
    // Insert global symbols
    symtable_insert(table, "x", VAR_KIND, 0, 0, 0);
    symtable_insert(table, "MAX", CONST_KIND, 100, 0, 0);
    
    // Enter a procedure scope
    symtable_enter_scope(table, "calc");
    
    // Insert local symbols
    symtable_insert(table, "temp", VAR_KIND, 0, 0, 0);
    
    // Lookup a symbol (scope chain search)
    Symbol* sym = symtable_lookup(table, "x");  // Found in global scope
    Symbol* local = symtable_lookup(table, "temp");  // Found in current scope
    
    // Exit scope
    symtable_exit_scope(table);
    
    // Print symbol table
    symtable_dump(table);
    
    // Destroy symbol table
    symtable_destroy(table);
    return 0;
}
```

## API Reference

### SymbolTable* symtable_create(void)
Creates a new symbol table with global scope.

### void symtable_destroy(SymbolTable* table)
Destroys a symbol table and frees all memory.

### bool symtable_enter_scope(SymbolTable* table, const char* procedure_name)
Enters a new scope (called when entering a procedure).

### bool symtable_exit_scope(SymbolTable* table)
Exits the current scope (called when leaving a procedure).

### bool symtable_insert(SymbolTable* table, const char* name, SymbolKind kind, int val, int address, int quad_index)
Inserts a new symbol into the current scope.

### Symbol* symtable_lookup(SymbolTable* table, const char* name)
Looks up a symbol in the symbol table (scope chain search).

### bool symtable_exists_in_current_scope(SymbolTable* table, const char* name)
Checks if a symbol exists in the current scope only.

### int symtable_get_current_level(SymbolTable* table)
Returns the current nesting level.

### int symtable_get_next_address(SymbolTable* table)
Returns the next available address offset.

### void symtable_increment_address(SymbolTable* table)
Increments the next address counter.

### void symtable_print(SymbolTable* table, FILE* fp)
Prints the symbol table to a file.

### void symtable_dump(SymbolTable* table)
Prints the symbol table to stdout.

## Scope Chain Lookup

The symbol table implements scope chain lookup as follows:

1. Search starts in the **current (innermost) scope**
2. If not found, search proceeds to **parent scope**
3. This continues until reaching the **global scope**
4. If still not found, returns NULL (symbol not declared)

This ensures proper handling of nested scopes where inner scope symbols
shadow outer scope symbols with the same name.

## Symbol Structure

```c
typedef struct {
    char name[MAX_TOKEN_LEN];  // Symbol name
    SymbolKind kind;           // const, var, or procedure
    int val;                   // Value (for constants)
    int level;                 // Nesting level
    int address;               // Memory address offset
    int quad_index;            // Quadruple index (for procedures)
} Symbol;
```

## Compilation

To compile with your project:
```
gcc -c symtable/symtable.c list/list.c -o symtable/symtable.o
```

Link with your main program:
```
gcc main.c symtable/symtable.o list/list.o -o compiler
```