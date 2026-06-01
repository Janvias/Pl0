/**
 * @file parser.c
 * @brief PL/0 Syntax and Semantic Analyzer Implementation
 * @description This file implements the syntax analysis, semantic checking,
 *              and intermediate code generation for the PL/0 compiler.
 *              Uses recursive descent parsing technique.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 2.0
 */

#include "compiler.h"

/*============================================================================
 * Intermediate Code Generation Functions
 *============================================================================*/

/**
 * @brief Generate a new temporary variable name
 * @param state Compiler state
 * @param temp Buffer to store the generated name
 * @details Creates names like T1, T2, T3, etc. for intermediate results
 */
void new_temp(CompilerState* state, char *temp) {
    sprintf(temp, "T%d", ++state->temp_count);
}

/**
 * @brief Generate and add a quadruple to the quadruple list
 * @param state Compiler state
 * @param op Operator (e.g., "+", ":=", "j")
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param result Result destination
 * @details Quadruple format: (op, arg1, arg2, result)
 */
void gen_quad(CompilerState* state, char *op, char *arg1, char *arg2, char *result) {
    strcpy(state->quad_list[state->quad_count].op, op);
    strcpy(state->quad_list[state->quad_count].arg1, arg1);
    strcpy(state->quad_list[state->quad_count].arg2, arg2);
    strcpy(state->quad_list[state->quad_count].result, result);
    state->quad_count++;
    state->next_instr++;
}

/*============================================================================
 * Expression Parsing Functions (Recursive Descent)
 *============================================================================*/

/**
 * @brief Parse a factor (primary expression element)
 * @param state Compiler state
 * @param result Buffer to store the factor value
 * @details Handles:
 *          - Identifiers: variable or constant names
 *          - Numbers: integer literals
 *          - Parenthesized expressions: (expression)
 */
void factor(CompilerState* state, char *result) {
    Token t = current_t(state);
    
    /* Case 1: Identifier */
    if (t.type == IDENTIFIER) {
        /* Check if identifier is declared */
        Symbol* sym = symtable_lookup(state->symbol_table, t.value);
        if (!sym) {
            printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                   t.line, t.value);
            state->semantic_error = 1;
        }
        strcpy(result, t.value);
        next_token_func(state);
    }
    
    /* Case 2: Number literal */
    else if (t.type == NUMBER) {
        strcpy(result, t.value);
        next_token_func(state);
    }
    
    /* Case 3: Parenthesized expression */
    else if (t.type == DELIMITER && strcmp(t.value, "(") == 0) {
        next_token_func(state);  /* Skip '(' */
        expression(state, result);
        match(state, DELIMITER, ")");  /* Expect ')' */
    }
    
    /* Error: Invalid factor */
    else {
        printf("(Syntax Error, Line: %d) Expected factor\n", t.line);
        state->syntax_error = 1;
        strcpy(result, "");
    }
}

/**
 * @brief Parse a term (factor with multiplication/division)
 * @param state Compiler state
 * @param result Buffer to store the term value
 * @details Grammar: term → factor { (* | /) factor }
 */
void term(CompilerState* state, char *result) {
    /* Parse first factor */
    factor(state, result);
    
    /* Handle multiplication and division operators */
    while (current_t(state).type == OPERATOR && 
           (strcmp(current_t(state).value, "*") == 0 || 
            strcmp(current_t(state).value, "/") == 0)) {
        
        /* Save operator */
        char op[MAX_TOKEN_LEN];
        strcpy(op, current_t(state).value);
        next_token_func(state);
        
        /* Parse second factor */
        char temp2[MAX_TOKEN_LEN];
        factor(state, temp2);
        
        /* Generate intermediate code */
        char temp3[MAX_TOKEN_LEN];
        new_temp(state, temp3);
        gen_quad(state, op, result, temp2, temp3);
        strcpy(result, temp3);
    }
}

/**
 * @brief Parse an expression (term with addition/subtraction)
 * @param state Compiler state
 * @param result Buffer to store the expression value
 * @details Grammar: expression → [ + | - ] term { (+ | -) term }
 */
void expression(CompilerState* state, char *result) {
    Token t = current_t(state);
    
    /* Handle leading sign */
    if (t.type == OPERATOR && 
        (strcmp(t.value, "+") == 0 || strcmp(t.value, "-") == 0)) {
        
        /* Save sign operator */
        char op[MAX_TOKEN_LEN];
        strcpy(op, t.value);
        next_token_func(state);
        
        /* Parse term */
        term(state, result);
        
        /* Generate code for sign operation */
        char temp[MAX_TOKEN_LEN];
        new_temp(state, temp);
        gen_quad(state, op, result, "", temp);
        strcpy(result, temp);
    } else {
        /* Parse first term without leading sign */
        term(state, result);
        
        /* Handle addition and subtraction operators */
        while (current_t(state).type == OPERATOR && 
               (strcmp(current_t(state).value, "+") == 0 || 
                strcmp(current_t(state).value, "-") == 0)) {
            
            /* Save operator */
            char op[MAX_TOKEN_LEN];
            strcpy(op, current_t(state).value);
            next_token_func(state);
            
            /* Parse second term */
            char temp[MAX_TOKEN_LEN];
            term(state, temp);
            
            /* Generate intermediate code */
            char temp2[MAX_TOKEN_LEN];
            new_temp(state, temp2);
            gen_quad(state, op, result, temp, temp2);
            strcpy(result, temp2);
        }
    }
}

/*============================================================================
 * Condition Parsing Functions
 *============================================================================*/

/**
 * @brief Parse a condition for if/while statements
 * @param state Compiler state
 * @param true_list Pointer to store jump quadruple index for true condition
 * @param false_list Pointer to store jump quadruple index for false condition
 * @details Handles:
 *          - odd expression
 *          - expression relational_operator expression
 */
void condition(CompilerState* state, int *true_list, int *false_list) {
    Token t = current_t(state);
    
    /* Case 1: odd condition */
    if (t.type == KEYWORD && strcmp(t.value, "odd") == 0) {
        next_token_func(state);
        
        /* Parse expression */
        char result[MAX_TOKEN_LEN];
        expression(state, result);
        
        /* Generate odd check */
        char temp[MAX_TOKEN_LEN];
        new_temp(state, temp);
        gen_quad(state, "odd", result, "", temp);
        
        /* Generate jump quadruples (backpatched later) */
        *true_list = state->quad_count;
        gen_quad(state, "j", "", "", "");
        *false_list = state->quad_count;
        gen_quad(state, "j", "", "", "");
    }
    
    /* Case 2: Relational expression */
    else {
        /* Parse left expression */
        char e1[MAX_TOKEN_LEN];
        expression(state, e1);
        
        /* Expect relational operator */
        if (current_t(state).type == OPERATOR) {
            /* Save operator */
            char op[MAX_TOKEN_LEN];
            strcpy(op, current_t(state).value);
            next_token_func(state);
            
            /* Parse right expression */
            char e2[MAX_TOKEN_LEN];
            expression(state, e2);
            
            /* Generate comparison quadruple */
            *true_list = state->quad_count;
            gen_quad(state, op, e1, e2, "");
            
            /* Generate jump quadruple */
            *false_list = state->quad_count;
            gen_quad(state, "j", "", "", "");
        } else {
            /* Error: Missing relational operator */
            printf("(Syntax Error, Line: %d) Expected relational operator\n", 
                   current_t(state).line);
            state->syntax_error = 1;
            *true_list = -1;
            *false_list = -1;
        }
    }
}

/*============================================================================
 * Statement Parsing Functions
 *============================================================================*/

/**
 * @brief Parse a statement
 * @param state Compiler state
 * @details Handles:
 *          - Assignment: identifier := expression
 *          - Compound: begin statement_list end
 *          - Conditional: if condition then statement
 *          - Loop: while condition do statement
 *          - Procedure call: call identifier
 *          - Input: read(identifier_list)
 *          - Output: write(expression_list)
 */
void statement(CompilerState* state) {
    Token t = current_t(state);
    
    /*------------------------------------------------------------------------
     * Assignment Statement
     *------------------------------------------------------------------------*/
    if (t.type == IDENTIFIER) {
        /* Save identifier name */
        char name[MAX_TOKEN_LEN];
        strcpy(name, t.value);
        
        /* Check if declared and not a procedure */
        Symbol* sym = symtable_lookup(state->symbol_table, name);
        if (!sym) {
            printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                   t.line, name);
            state->semantic_error = 1;
        } else if (sym->kind == PROCEDURE_KIND) {
            printf("(Semantic Error, Line: %d) Procedure used as variable: %s\n", 
                   t.line, name);
            state->semantic_error = 1;
        }
        
        next_token_func(state);
        match(state, OPERATOR, ":=");
        
        /* Parse expression and generate assignment code */
        char result[MAX_TOKEN_LEN];
        expression(state, result);
        gen_quad(state, ":=", result, "", name);
    }
    
    /*------------------------------------------------------------------------
     * Compound Statement (begin...end)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "begin") == 0) {
        next_token_func(state);  /* Skip 'begin' */
        statement(state);
        statement_list(state);
        match(state, KEYWORD, "end");
    }
    
    /*------------------------------------------------------------------------
     * Conditional Statement (if...then)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "if") == 0) {
        next_token_func(state);  /* Skip 'if' */
        
        /* Parse condition */
        int true_list, false_list;
        condition(state, &true_list, &false_list);
        
        match(state, KEYWORD, "then");
        
        /* Backpatch true jump to next instruction */
        sprintf(state->quad_list[true_list].result, "%d", state->next_instr);
        
        /* Parse then statement */
        statement(state);
        
        /* Backpatch false jump to next instruction */
        if (false_list >= 0) {
            sprintf(state->quad_list[false_list].result, "%d", state->next_instr);
        }
    }
    
    /*------------------------------------------------------------------------
     * While Loop Statement
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "while") == 0) {
        next_token_func(state);  /* Skip 'while' */
        
        /* Save loop start position */
        int while_instr = state->next_instr;
        
        /* Parse condition */
        int true_list, false_list;
        condition(state, &true_list, &false_list);
        
        match(state, KEYWORD, "do");
        
        /* Backpatch true jump to loop body */
        sprintf(state->quad_list[true_list].result, "%d", state->next_instr);
        
        /* Parse loop body */
        statement(state);
        
        /* Generate jump back to loop start */
        gen_quad(state, "j", "", "", "");
        sprintf(state->quad_list[state->quad_count - 1].result, "%d", while_instr);
        
        /* Backpatch false jump to after loop */
        if (false_list >= 0) {
            sprintf(state->quad_list[false_list].result, "%d", state->next_instr);
        }
    }
    
    /*------------------------------------------------------------------------
     * Procedure Call Statement
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "call") == 0) {
        next_token_func(state);  /* Skip 'call' */
        
        if (current_t(state).type == IDENTIFIER) {
            /* Save procedure name */
            char name[MAX_TOKEN_LEN];
            strcpy(name, current_t(state).value);
            
            /* Check if declared as procedure */
            Symbol* sym = symtable_lookup(state->symbol_table, name);
            if (!sym) {
                printf("(Semantic Error, Line: %d) Undeclared procedure: %s\n", 
                       current_t(state).line, name);
                state->semantic_error = 1;
            } else if (sym->kind != PROCEDURE_KIND) {
                printf("(Semantic Error, Line: %d) Not a procedure: %s\n", 
                       current_t(state).line, name);
                state->semantic_error = 1;
            }
            
            /* Generate call quadruple */
            gen_quad(state, "call", name, "", "");
            next_token_func(state);
        } else {
            printf("(Syntax Error, Line: %d) Expected procedure name\n", 
                   current_t(state).line);
            state->syntax_error = 1;
        }
    }
    
    /*------------------------------------------------------------------------
     * Read Statement (Input)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "read") == 0) {
        next_token_func(state);  /* Skip 'read' */
        match(state, DELIMITER, "(");
        
        if (current_t(state).type == IDENTIFIER) {
            /* Read first identifier */
            char name[MAX_TOKEN_LEN];
            strcpy(name, current_t(state).value);
            
            /* Check if declared */
            Symbol* sym = symtable_lookup(state->symbol_table, name);
            if (!sym) {
                printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                       current_t(state).line, name);
                state->semantic_error = 1;
            }
            
            gen_quad(state, "read", name, "", "");
            next_token_func(state);
            
            /* Handle additional identifiers */
            while (current_t(state).type == DELIMITER && 
                   strcmp(current_t(state).value, ",") == 0) {
                next_token_func(state);  /* Skip ',' */
                
                if (current_t(state).type == IDENTIFIER) {
                    strcpy(name, current_t(state).value);
                    sym = symtable_lookup(state->symbol_table, name);
                    if (!sym) {
                        printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                               current_t(state).line, name);
                        state->semantic_error = 1;
                    }
                    gen_quad(state, "read", name, "", "");
                    next_token_func(state);
                }
            }
        }
        
        match(state, DELIMITER, ")");
    }
    
    /*------------------------------------------------------------------------
     * Write Statement (Output)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "write") == 0) {
        next_token_func(state);  /* Skip 'write' */
        match(state, DELIMITER, "(");
        
        /* Parse and write first expression */
        char result[MAX_TOKEN_LEN];
        expression(state, result);
        gen_quad(state, "write", result, "", "");
        
        /* Handle additional expressions */
        while (current_t(state).type == DELIMITER && 
               strcmp(current_t(state).value, ",") == 0) {
            next_token_func(state);  /* Skip ',' */
            expression(state, result);
            gen_quad(state, "write", result, "", "");
        }
        
        match(state, DELIMITER, ")");
    }
}

/**
 * @brief Parse a statement list (sequence of statements)
 * @param state Compiler state
 * @details Grammar: statement_list → { ; statement }
 */
void statement_list(CompilerState* state) {
    /* Parse statements separated by semicolons */
    while (current_t(state).type == DELIMITER && 
           strcmp(current_t(state).value, ";") == 0) {
        next_token_func(state);  /* Skip ';' */
        statement(state);
    }
}

/*============================================================================
 * Declaration Parsing Functions
 *============================================================================*/

/**
 * @brief Parse constant declaration section
 * @param state Compiler state
 * @details Grammar: const identifier = number { , identifier = number } ;
 */
void const_declaration(CompilerState* state) {
    if (current_t(state).type == KEYWORD && strcmp(current_t(state).value, "const") == 0) {
        next_token_func(state);  /* Skip 'const' */
        
        /* Parse constant definitions */
        while (1) {
            if (current_t(state).type == IDENTIFIER) {
                /* Save constant name */
                char name[MAX_TOKEN_LEN];
                strcpy(name, current_t(state).value);
                next_token_func(state);
                
                match(state, OPERATOR, "=");
                
                /* Get constant value */
                if (current_t(state).type == NUMBER) {
                    int val = atoi(current_t(state).value);
                    if (!symtable_insert(state->symbol_table, name, CONST_KIND, val, 0, 0)) {
                        printf("(Semantic Error, Line: %d) Redeclared: %s\n", 
                               current_t(state).line, name);
                        state->semantic_error = 1;
                    }
                    next_token_func(state);
                }
                
                /* Check for more constants */
                if (current_t(state).type == DELIMITER && 
                    strcmp(current_t(state).value, ",") == 0) {
                    next_token_func(state);
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        match(state, DELIMITER, ";");
    }
}

/**
 * @brief Parse variable declaration section
 * @param state Compiler state
 * @details Grammar: var identifier { , identifier } ;
 */
void var_declaration(CompilerState* state) {
    if (current_t(state).type == KEYWORD && strcmp(current_t(state).value, "var") == 0) {
        next_token_func(state);  /* Skip 'var' */
        
        /* Parse variable names */
        while (1) {
            if (current_t(state).type == IDENTIFIER) {
                int addr = symtable_get_next_address(state->symbol_table);
                if (!symtable_insert(state->symbol_table, current_t(state).value, VAR_KIND, 0, addr, 0)) {
                    printf("(Semantic Error, Line: %d) Redeclared: %s\n", 
                           current_t(state).line, current_t(state).value);
                    state->semantic_error = 1;
                }
                symtable_increment_address(state->symbol_table);
                next_token_func(state);
                
                /* Check for more variables */
                if (current_t(state).type == DELIMITER && 
                    strcmp(current_t(state).value, ",") == 0) {
                    next_token_func(state);
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        match(state, DELIMITER, ";");
    }
}

/**
 * @brief Parse procedure declaration section
 * @param state Compiler state
 * @details Grammar: procedure identifier ; block ;
 */
void procedure_declaration(CompilerState* state) {
    while (current_t(state).type == KEYWORD && 
           strcmp(current_t(state).value, "procedure") == 0) {
        next_token_func(state);  /* Skip 'procedure' */
        
        if (current_t(state).type == IDENTIFIER) {
            /* Add procedure to symbol table */
            char proc_name[MAX_TOKEN_LEN];
            strcpy(proc_name, current_t(state).value);
            if (!symtable_insert(state->symbol_table, proc_name, PROCEDURE_KIND, 0, 0, state->quad_count)) {
                printf("(Semantic Error, Line: %d) Redeclared: %s\n", 
                       current_t(state).line, proc_name);
                state->semantic_error = 1;
            }
            next_token_func(state);
            
            match(state, DELIMITER, ";");
            
            /* Increase nesting level for procedure body */
            symtable_enter_scope(state->symbol_table, proc_name);
            block(state);
            symtable_exit_scope(state->symbol_table);
            
            match(state, DELIMITER, ";");
        } else {
            match(state, DELIMITER, ";");
            state->syntax_error = 1;
        }
    }
}

/*============================================================================
 * Block and Program Parsing Functions
 *============================================================================*/

/**
 * @brief Parse a block (declarations + statements)
 * @param state Compiler state
 * @details Grammar: block → const_declaration var_declaration 
 *                      procedure_declaration statement
 */
void block(CompilerState* state) {
    const_declaration(state);
    var_declaration(state);
    procedure_declaration(state);
    statement(state);
}

/**
 * @brief Parse entire program
 * @param state Compiler state
 * @details Grammar: program → block .
 *          Generates system quadruples at start and end
 */
void program(CompilerState* state) {
    /* Generate system start quadruple */
    gen_quad(state, "syss", "", "", "");
    
    /* Parse program block */
    block(state);
    
    /* Expect program terminator */
    match(state, DELIMITER, ".");
    
    /* Generate system end quadruple */
    gen_quad(state, "sysc", "", "", "");
}

/*============================================================================
 * Output Functions
 *============================================================================*/

/**
 * @brief Print all quadruples to console
 * @param state Compiler state
 * @details Format: (address)(op,arg1,arg2,result)
 */
void print_quad(CompilerState* state) {
    printf("\n===== QUADRUPLES =====\n");
    for (int i = 0; i < state->quad_count; i++) {
        Quad q = state->quad_list[i];
        printf("(%d)(%s,%s,%s,%s)\n", 100 + i, q.op, q.arg1, q.arg2, q.result);
    }
}

/**
 * @brief Write all quadruples to output file
 * @param state Compiler state
 * @param fp File pointer for output
 * @details Same format as print_quad but to file
 */
void print_quad_to_file(CompilerState* state, FILE *fp) {
    fprintf(fp, "===== QUADRUPLES =====\n");
    for (int i = 0; i < state->quad_count; i++) {
        Quad q = state->quad_list[i];
        fprintf(fp, "(%d)(%s,%s,%s,%s)\n", 100 + i, q.op, q.arg1, q.arg2, q.result);
    }
}

/**
 * @brief Print symbol table to console
 * @param state Compiler state
 * @details Shows name, kind, value, level, and address for each symbol
 */
void print_symbol_table(CompilerState* state) {
    symtable_dump(state->symbol_table);
}

/**
 * @brief Write compiler state to cache file
 * @param state Compiler state
 * @param filename Output cache filename
 * @return true if successful, false otherwise
 * @details Format:
 *          [COMPILER_STATE]
 *          token_count=X
 *          quad_count=Y
 *          temp_count=Z
 *          next_instr=X
 *          syntax_error=0/1
 *          semantic_error=0/1
 *          [SYMBOL_TABLE]
 *          name|kind|value|level|address|quad_index
 *          [QUADRUPLES]
 *          address|op|arg1|arg2|result
 *          [TOKENS]
 *          index|type|value|line
 */
bool write_compiler_state_to_cache(CompilerState* state, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error: Cannot create cache file '%s'\n", filename);
        return false;
    }
    
    /* Write header */
    fprintf(fp, "========================================\n");
    fprintf(fp, "PL/0 Compiler Cache File\n");
    fprintf(fp, "Generated: %s %s\n", __TIME__, __DATE__);
    fprintf(fp, "========================================\n\n");
    
    /* Write compiler state section */
    fprintf(fp, "[COMPILER_STATE]\n");
    fprintf(fp, "token_count=%d\n", state->token_count);
    fprintf(fp, "quad_count=%d\n", state->quad_count);
    fprintf(fp, "temp_count=%d\n", state->temp_count);
    fprintf(fp, "next_instr=%d\n", state->next_instr);
    fprintf(fp, "syntax_error=%d\n", state->syntax_error);
    fprintf(fp, "semantic_error=%d\n", state->semantic_error);
    
    /* Write symbol table section */
    fprintf(fp, "\n[SYMBOL_TABLE]\n");
    fprintf(fp, "Format: name|kind|value|level|address|quad_index\n");
    
    List* scopes = state->symbol_table->scopes;
    if (scopes != NULL) {
        for (int i = 0; i < list_size(scopes); i++) {
            Scope* scope = (Scope*)list_get(scopes, i);
            if (scope != NULL && scope->symbols != NULL) {
                fprintf(fp, "\n--- Scope Level %d", scope->level);
                if (strlen(scope->procedure_name) > 0) {
                    fprintf(fp, " (%s)", scope->procedure_name);
                }
                fprintf(fp, " ---\n");
                
                for (int j = 0; j < list_size(scope->symbols); j++) {
                    Symbol* sym = (Symbol*)list_get(scope->symbols, j);
                    if (sym != NULL) {
                        const char* kind_str;
                        switch (sym->kind) {
                            case CONST_KIND: kind_str = "CONST"; break;
                            case VAR_KIND: kind_str = "VAR"; break;
                            case PROCEDURE_KIND: kind_str = "PROCEDURE"; break;
                            default: kind_str = "UNKNOWN";
                        }
                        fprintf(fp, "%s|%s|%d|%d|%d|%d\n",
                                sym->name,
                                kind_str,
                                sym->val,
                                sym->level,
                                sym->address,
                                sym->quad_index);
                    }
                }
            }
        }
    }
    
    /* Write quadruples section */
    fprintf(fp, "\n[QUADRUPLES]\n");
    fprintf(fp, "Format: address|op|arg1|arg2|result\n");
    for (int i = 0; i < state->quad_count; i++) {
        Quad q = state->quad_list[i];
        fprintf(fp, "%d|%s|%s|%s|%s\n",
                100 + i,
                q.op,
                q.arg1,
                q.arg2,
                q.result);
    }
    
    /* Write tokens section */
    fprintf(fp, "\n[TOKENS]\n");
    fprintf(fp, "Format: index|type|value|line\n");
    for (int i = 0; i < state->token_count; i++) {
        Token t = state->token_list[i];
        const char* type_str;
        switch (t.type) {
            case KEYWORD: type_str = "KEYWORD"; break;
            case IDENTIFIER: type_str = "IDENTIFIER"; break;
            case NUMBER: type_str = "NUMBER"; break;
            case OPERATOR: type_str = "OPERATOR"; break;
            case DELIMITER: type_str = "DELIMITER"; break;
            case ERROR: type_str = "ERROR"; break;
            default: type_str = "UNKNOWN";
        }
        fprintf(fp, "%d|%s|%s|%d\n", i, type_str, t.value, t.line);
    }
    
    /* Write footer */
    fprintf(fp, "\n========================================\n");
    fprintf(fp, "End of Cache File\n");
    fprintf(fp, "========================================\n");
    
    fclose(fp);
    printf("Cache file written to: %s\n", filename);
    return true;
}

/**
 * @brief Read compiler state from cache file
 * @param state Compiler state (will be initialized)
 * @param filename Input cache filename
 * @return true if successful, false otherwise
 */
bool read_compiler_state_from_cache(CompilerState* state, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: Cannot open cache file '%s'\n", filename);
        return false;
    }
    
    /* Initialize state */
    memset(state, 0, sizeof(CompilerState));
    state->fp_in = NULL;
    
    char line[512];
    char section[32] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline */
        line[strcspn(line, "\r\n")] = 0;
        
        /* Skip comments and empty lines */
        if (line[0] == '=' || line[0] == '\0') continue;
        
        /* Check for section headers */
        if (line[0] == '[') {
            sscanf(line, "[%31[^]]]", section);
            continue;
        }
        
        /* Parse based on section */
        if (strcmp(section, "COMPILER_STATE") == 0) {
            if (sscanf(line, "token_count=%d", &state->token_count) == 1) continue;
            if (sscanf(line, "quad_count=%d", &state->quad_count) == 1) continue;
            if (sscanf(line, "temp_count=%d", &state->temp_count) == 1) continue;
            if (sscanf(line, "next_instr=%d", &state->next_instr) == 1) continue;
            if (sscanf(line, "syntax_error=%d", &state->syntax_error) == 1) continue;
            if (sscanf(line, "semantic_error=%d", &state->semantic_error) == 1) continue;
        }
        else if (strcmp(section, "TOKENS") == 0) {
            int index, line_num;
            char type_str[32], value[MAX_TOKEN_LEN];
            if (sscanf(line, "%d|%31[^|]|%99[^|]|%d", &index, type_str, value, &line_num) == 4) {
                TokenType type;
                if (strcmp(type_str, "KEYWORD") == 0) type = KEYWORD;
                else if (strcmp(type_str, "IDENTIFIER") == 0) type = IDENTIFIER;
                else if (strcmp(type_str, "NUMBER") == 0) type = NUMBER;
                else if (strcmp(type_str, "OPERATOR") == 0) type = OPERATOR;
                else if (strcmp(type_str, "DELIMITER") == 0) type = DELIMITER;
                else type = ERROR;
                
                state->token_list[index].type = type;
                strcpy(state->token_list[index].value, value);
                state->token_list[index].line = line_num;
            }
        }
        else if (strcmp(section, "QUADRUPLES") == 0) {
            int addr;
            char op[MAX_TOKEN_LEN], arg1[MAX_TOKEN_LEN], arg2[MAX_TOKEN_LEN], result[MAX_TOKEN_LEN];
            if (sscanf(line, "%d|%99[^|]|%99[^|]|%99[^|]|%99[^|]", 
                       &addr, op, arg1, arg2, result) == 5) {
                int idx = addr - 100;
                strcpy(state->quad_list[idx].op, op);
                strcpy(state->quad_list[idx].arg1, arg1);
                strcpy(state->quad_list[idx].arg2, arg2);
                strcpy(state->quad_list[idx].result, result);
            }
        }
        /* Symbol table requires special handling due to linked list structure */
    }
    
    fclose(fp);
    printf("Cache file read from: %s\n", filename);
    return true;
}

/**
 * @brief Initialize compiler state
 * @param state Compiler state to initialize
 * @param fp_in Input file pointer
 * @return true if successful, false otherwise
 */
bool init_compiler_state(CompilerState* state, FILE* fp_in) {
    memset(state, 0, sizeof(CompilerState));
    state->fp_in = fp_in;
    state->line = 1;
    state->next_instr = 100;
    
    /* Create symbol table */
    state->symbol_table = symtable_create();
    if (state->symbol_table == NULL) {
        return false;
    }
    
    /* Enter global scope */
    symtable_enter_scope(state->symbol_table, "main");
    
    return true;
}

/**
 * @brief Destroy compiler state and free resources
 * @param state Compiler state to destroy
 */
void destroy_compiler_state(CompilerState* state) {
    if (state->symbol_table != NULL) {
        symtable_destroy(state->symbol_table);
        state->symbol_table = NULL;
    }
    state->fp_in = NULL;
}