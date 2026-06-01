/**
 * @file parser.c
 * @brief PL/0 Syntax and Semantic Analyzer Implementation
 * @description This file implements the syntax analysis, semantic checking,
 *              and intermediate code generation for the PL/0 compiler.
 *              Uses recursive descent parsing technique.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-05-30
 * @version 1.0
 */

#include "compiler.h"

/*============================================================================
 * Symbol Table Management Functions
 *============================================================================*/

/**
 * @brief Look up a symbol in the symbol table
 * @param name The symbol name to search for
 * @return Index of the symbol if found, -1 if not found
 * @details Searches from most recent to oldest to handle scope correctly
 */
int lookup_symbol(char *name) {
    /* Search backwards to find most recent declaration first */
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;  /* Symbol not found */
}

/**
 * @brief Add a new symbol to the symbol table
 * @param name Symbol name
 * @param kind Symbol kind (CONST_KIND, VAR_KIND, or PROCEDURE_KIND)
 * @param val Value for constants (0 for variables and procedures)
 * @details Checks for redeclaration errors before adding
 */
void add_symbol(char *name, SymbolKind kind, int val) {
    /* Check for redeclaration in current scope */
    if (lookup_symbol(name) >= 0) {
        printf("(Semantic Error, Line: %d) Redeclared: %s\n", 
               current_t().line, name);
        semantic_error = 1;
        return;
    }
    
    /* Add symbol to table */
    strcpy(symbol_table[symbol_count].name, name);
    symbol_table[symbol_count].kind = kind;
    symbol_table[symbol_count].val = val;
    symbol_table[symbol_count].level = current_level;
    symbol_table[symbol_count].address = symbol_count;
    symbol_count++;
}

/*============================================================================
 * Intermediate Code Generation Functions
 *============================================================================*/

/**
 * @brief Generate a new temporary variable name
 * @param temp Buffer to store the generated name
 * @details Creates names like T1, T2, T3, etc. for intermediate results
 */
void new_temp(char *temp) {
    sprintf(temp, "T%d", ++temp_count);
}

/**
 * @brief Generate and add a quadruple to the quadruple list
 * @param op Operator (e.g., "+", ":=", "j")
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param result Result destination
 * @details Quadruple format: (op, arg1, arg2, result)
 */
void gen_quad(char *op, char *arg1, char *arg2, char *result) {
    strcpy(quad_list[quad_count].op, op);
    strcpy(quad_list[quad_count].arg1, arg1);
    strcpy(quad_list[quad_count].arg2, arg2);
    strcpy(quad_list[quad_count].result, result);
    quad_count++;
    next_instr++;
}

/*============================================================================
 * Expression Parsing Functions (Recursive Descent)
 *============================================================================*/

/**
 * @brief Parse a factor (primary expression element)
 * @param result Buffer to store the factor value
 * @details Handles:
 *          - Identifiers: variable or constant names
 *          - Numbers: integer literals
 *          - Parenthesized expressions: (expression)
 */
void factor(char *result) {
    Token t = current_t();
    
    /* Case 1: Identifier */
    if (t.type == IDENTIFIER) {
        /* Check if identifier is declared */
        int idx = lookup_symbol(t.value);
        if (idx < 0) {
            printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                   t.line, t.value);
            semantic_error = 1;
        }
        strcpy(result, t.value);
        next_token_func();
    }
    
    /* Case 2: Number literal */
    else if (t.type == NUMBER) {
        strcpy(result, t.value);
        next_token_func();
    }
    
    /* Case 3: Parenthesized expression */
    else if (t.type == DELIMITER && strcmp(t.value, "(") == 0) {
        next_token_func();  /* Skip '(' */
        expression(result);
        match(DELIMITER, ")");  /* Expect ')' */
    }
    
    /* Error: Invalid factor */
    else {
        printf("(Syntax Error, Line: %d) Expected factor\n", t.line);
        syntax_error = 1;
        strcpy(result, "");
    }
}

/**
 * @brief Parse a term (factor with multiplication/division)
 * @param result Buffer to store the term value
 * @details Grammar: term → factor { (* | /) factor }
 */
void term(char *result) {
    /* Parse first factor */
    factor(result);
    
    /* Handle multiplication and division operators */
    while (current_t().type == OPERATOR && 
           (strcmp(current_t().value, "*") == 0 || 
            strcmp(current_t().value, "/") == 0)) {
        
        /* Save operator */
        char op[MAX_TOKEN_LEN];
        strcpy(op, current_t().value);
        next_token_func();
        
        /* Parse second factor */
        char temp2[MAX_TOKEN_LEN];
        factor(temp2);
        
        /* Generate intermediate code */
        char temp3[MAX_TOKEN_LEN];
        new_temp(temp3);
        gen_quad(op, result, temp2, temp3);
        strcpy(result, temp3);
    }
}

/**
 * @brief Parse an expression (term with addition/subtraction)
 * @param result Buffer to store the expression value
 * @details Grammar: expression → [ + | - ] term { (+ | -) term }
 */
void expression(char *result) {
    Token t = current_t();
    
    /* Handle leading sign */
    if (t.type == OPERATOR && 
        (strcmp(t.value, "+") == 0 || strcmp(t.value, "-") == 0)) {
        
        /* Save sign operator */
        char op[MAX_TOKEN_LEN];
        strcpy(op, t.value);
        next_token_func();
        
        /* Parse term */
        term(result);
        
        /* Generate code for sign operation */
        char temp[MAX_TOKEN_LEN];
        new_temp(temp);
        gen_quad(op, result, "", temp);
        strcpy(result, temp);
    } else {
        /* Parse first term without leading sign */
        term(result);
        
        /* Handle addition and subtraction operators */
        while (current_t().type == OPERATOR && 
               (strcmp(current_t().value, "+") == 0 || 
                strcmp(current_t().value, "-") == 0)) {
            
            /* Save operator */
            char op[MAX_TOKEN_LEN];
            strcpy(op, current_t().value);
            next_token_func();
            
            /* Parse second term */
            char temp[MAX_TOKEN_LEN];
            term(temp);
            
            /* Generate intermediate code */
            char temp2[MAX_TOKEN_LEN];
            new_temp(temp2);
            gen_quad(op, result, temp, temp2);
            strcpy(result, temp2);
        }
    }
}

/*============================================================================
 * Condition Parsing Functions
 *============================================================================*/

/**
 * @brief Parse a condition for if/while statements
 * @param true_list Pointer to store jump quadruple index for true condition
 * @param false_list Pointer to store jump quadruple index for false condition
 * @details Handles:
 *          - odd expression
 *          - expression relational_operator expression
 */
void condition(int *true_list, int *false_list) {
    Token t = current_t();
    
    /* Case 1: odd condition */
    if (t.type == KEYWORD && strcmp(t.value, "odd") == 0) {
        next_token_func();
        
        /* Parse expression */
        char result[MAX_TOKEN_LEN];
        expression(result);
        
        /* Generate odd check */
        char temp[MAX_TOKEN_LEN];
        new_temp(temp);
        gen_quad("odd", result, "", temp);
        
        /* Generate jump quadruples (backpatched later) */
        *true_list = quad_count;
        gen_quad("j", "", "", "");
        *false_list = quad_count;
        gen_quad("j", "", "", "");
    }
    
    /* Case 2: Relational expression */
    else {
        /* Parse left expression */
        char e1[MAX_TOKEN_LEN];
        expression(e1);
        
        /* Expect relational operator */
        if (current_t().type == OPERATOR) {
            /* Save operator */
            char op[MAX_TOKEN_LEN];
            strcpy(op, current_t().value);
            next_token_func();
            
            /* Parse right expression */
            char e2[MAX_TOKEN_LEN];
            expression(e2);
            
            /* Generate comparison quadruple */
            *true_list = quad_count;
            gen_quad(op, e1, e2, "");
            
            /* Generate jump quadruple */
            *false_list = quad_count;
            gen_quad("j", "", "", "");
        } else {
            /* Error: Missing relational operator */
            printf("(Syntax Error, Line: %d) Expected relational operator\n", 
                   current_t().line);
            syntax_error = 1;
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
 * @details Handles:
 *          - Assignment: identifier := expression
 *          - Compound: begin statement_list end
 *          - Conditional: if condition then statement
 *          - Loop: while condition do statement
 *          - Procedure call: call identifier
 *          - Input: read(identifier_list)
 *          - Output: write(expression_list)
 */
void statement() {
    Token t = current_t();
    
    /*------------------------------------------------------------------------
     * Assignment Statement
     *------------------------------------------------------------------------*/
    if (t.type == IDENTIFIER) {
        /* Save identifier name */
        char name[MAX_TOKEN_LEN];
        strcpy(name, t.value);
        
        /* Check if declared and not a procedure */
        int idx = lookup_symbol(name);
        if (idx < 0) {
            printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                   t.line, name);
            semantic_error = 1;
        } else if (symbol_table[idx].kind == PROCEDURE_KIND) {
            printf("(Semantic Error, Line: %d) Procedure used as variable: %s\n", 
                   t.line, name);
            semantic_error = 1;
        }
        
        next_token_func();
        match(OPERATOR, ":=");
        
        /* Parse expression and generate assignment code */
        char result[MAX_TOKEN_LEN];
        expression(result);
        gen_quad(":=", result, "", name);
    }
    
    /*------------------------------------------------------------------------
     * Compound Statement (begin...end)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "begin") == 0) {
        next_token_func();  /* Skip 'begin' */
        statement();
        statement_list();
        match(KEYWORD, "end");
    }
    
    /*------------------------------------------------------------------------
     * Conditional Statement (if...then)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "if") == 0) {
        next_token_func();  /* Skip 'if' */
        
        /* Parse condition */
        int true_list, false_list;
        condition(&true_list, &false_list);
        
        match(KEYWORD, "then");
        
        /* Backpatch true jump to next instruction */
        sprintf(quad_list[true_list].result, "%d", next_instr);
        
        /* Parse then statement */
        statement();
        
        /* Backpatch false jump to next instruction */
        if (false_list >= 0) {
            sprintf(quad_list[false_list].result, "%d", next_instr);
        }
    }
    
    /*------------------------------------------------------------------------
     * While Loop Statement
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "while") == 0) {
        next_token_func();  /* Skip 'while' */
        
        /* Save loop start position */
        int while_instr = next_instr;
        
        /* Parse condition */
        int true_list, false_list;
        condition(&true_list, &false_list);
        
        match(KEYWORD, "do");
        
        /* Backpatch true jump to loop body */
        sprintf(quad_list[true_list].result, "%d", next_instr);
        
        /* Parse loop body */
        statement();
        
        /* Generate jump back to loop start */
        gen_quad("j", "", "", "");
        sprintf(quad_list[quad_count - 1].result, "%d", while_instr);
        
        /* Backpatch false jump to after loop */
        if (false_list >= 0) {
            sprintf(quad_list[false_list].result, "%d", next_instr);
        }
    }
    
    /*------------------------------------------------------------------------
     * Procedure Call Statement
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "call") == 0) {
        next_token_func();  /* Skip 'call' */
        
        if (current_t().type == IDENTIFIER) {
            /* Save procedure name */
            char name[MAX_TOKEN_LEN];
            strcpy(name, current_t().value);
            
            /* Check if declared as procedure */
            int idx = lookup_symbol(name);
            if (idx < 0) {
                printf("(Semantic Error, Line: %d) Undeclared procedure: %s\n", 
                       current_t().line, name);
                semantic_error = 1;
            } else if (symbol_table[idx].kind != PROCEDURE_KIND) {
                printf("(Semantic Error, Line: %d) Not a procedure: %s\n", 
                       current_t().line, name);
                semantic_error = 1;
            }
            
            /* Generate call quadruple */
            gen_quad("call", name, "", "");
            next_token_func();
        } else {
            printf("(Syntax Error, Line: %d) Expected procedure name\n", 
                   current_t().line);
            syntax_error = 1;
        }
    }
    
    /*------------------------------------------------------------------------
     * Read Statement (Input)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "read") == 0) {
        next_token_func();  /* Skip 'read' */
        match(DELIMITER, "(");
        
        if (current_t().type == IDENTIFIER) {
            /* Read first identifier */
            char name[MAX_TOKEN_LEN];
            strcpy(name, current_t().value);
            
            /* Check if declared */
            int idx = lookup_symbol(name);
            if (idx < 0) {
                printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                       current_t().line, name);
                semantic_error = 1;
            }
            
            gen_quad("read", name, "", "");
            next_token_func();
            
            /* Handle additional identifiers */
            while (current_t().type == DELIMITER && 
                   strcmp(current_t().value, ",") == 0) {
                next_token_func();  /* Skip ',' */
                
                if (current_t().type == IDENTIFIER) {
                    strcpy(name, current_t().value);
                    idx = lookup_symbol(name);
                    if (idx < 0) {
                        printf("(Semantic Error, Line: %d) Undeclared: %s\n", 
                               current_t().line, name);
                        semantic_error = 1;
                    }
                    gen_quad("read", name, "", "");
                    next_token_func();
                }
            }
        }
        
        match(DELIMITER, ")");
    }
    
    /*------------------------------------------------------------------------
     * Write Statement (Output)
     *------------------------------------------------------------------------*/
    else if (t.type == KEYWORD && strcmp(t.value, "write") == 0) {
        next_token_func();  /* Skip 'write' */
        match(DELIMITER, "(");
        
        /* Parse and write first expression */
        char result[MAX_TOKEN_LEN];
        expression(result);
        gen_quad("write", result, "", "");
        
        /* Handle additional expressions */
        while (current_t().type == DELIMITER && 
               strcmp(current_t().value, ",") == 0) {
            next_token_func();  /* Skip ',' */
            expression(result);
            gen_quad("write", result, "", "");
        }
        
        match(DELIMITER, ")");
    }
}

/**
 * @brief Parse a statement list (sequence of statements)
 * @details Grammar: statement_list → { ; statement }
 */
void statement_list() {
    /* Parse statements separated by semicolons */
    while (current_t().type == DELIMITER && 
           strcmp(current_t().value, ";") == 0) {
        next_token_func();  /* Skip ';' */
        statement();
    }
}

/*============================================================================
 * Declaration Parsing Functions
 *============================================================================*/

/**
 * @brief Parse constant declaration section
 * @details Grammar: const identifier = number { , identifier = number } ;
 */
void const_declaration() {
    if (current_t().type == KEYWORD && strcmp(current_t().value, "const") == 0) {
        next_token_func();  /* Skip 'const' */
        
        /* Parse constant definitions */
        while (1) {
            if (current_t().type == IDENTIFIER) {
                /* Save constant name */
                char name[MAX_TOKEN_LEN];
                strcpy(name, current_t().value);
                next_token_func();
                
                match(OPERATOR, "=");
                
                /* Get constant value */
                if (current_t().type == NUMBER) {
                    int val = atoi(current_t().value);
                    add_symbol(name, CONST_KIND, val);
                    next_token_func();
                }
                
                /* Check for more constants */
                if (current_t().type == DELIMITER && 
                    strcmp(current_t().value, ",") == 0) {
                    next_token_func();
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        match(DELIMITER, ";");
    }
}

/**
 * @brief Parse variable declaration section
 * @details Grammar: var identifier { , identifier } ;
 */
void var_declaration() {
    if (current_t().type == KEYWORD && strcmp(current_t().value, "var") == 0) {
        next_token_func();  /* Skip 'var' */
        
        /* Parse variable names */
        while (1) {
            if (current_t().type == IDENTIFIER) {
                add_symbol(current_t().value, VAR_KIND, 0);
                next_token_func();
                
                /* Check for more variables */
                if (current_t().type == DELIMITER && 
                    strcmp(current_t().value, ",") == 0) {
                    next_token_func();
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        match(DELIMITER, ";");
    }
}

/**
 * @brief Parse procedure declaration section
 * @details Grammar: procedure identifier ; block ;
 */
void procedure_declaration() {
    while (current_t().type == KEYWORD && 
           strcmp(current_t().value, "procedure") == 0) {
        next_token_func();  /* Skip 'procedure' */
        
        if (current_t().type == IDENTIFIER) {
            /* Add procedure to symbol table */
            add_symbol(current_t().value, PROCEDURE_KIND, 0);
            next_token_func();
        }
        
        match(DELIMITER, ";");
        
        /* Increase nesting level for procedure body */
        current_level++;
        block();
        current_level--;
        
        match(DELIMITER, ";");
    }
}

/*============================================================================
 * Block and Program Parsing Functions
 *============================================================================*/

/**
 * @brief Parse a block (declarations + statements)
 * @details Grammar: block → const_declaration var_declaration 
 *                      procedure_declaration statement
 */
void block() {
    const_declaration();
    var_declaration();
    procedure_declaration();
    statement();
}

/**
 * @brief Parse entire program
 * @details Grammar: program → block .
 *          Generates system quadruples at start and end
 */
void program() {
    /* Generate system start quadruple */
    gen_quad("syss", "", "", "");
    
    /* Parse program block */
    block();
    
    /* Expect program terminator */
    match(DELIMITER, ".");
    
    /* Generate system end quadruple */
    gen_quad("sysc", "", "", "");
}

/*============================================================================
 * Output Functions
 *============================================================================*/

/**
 * @brief Print all quadruples to console
 * @details Format: (address)(op,arg1,arg2,result)
 */
void print_quad() {
    printf("\n===== QUADRUPLES =====\n");
    for (int i = 0; i < quad_count; i++) {
        Quad q = quad_list[i];
        printf("(%d)(%s,%s,%s,%s)\n", 100 + i, q.op, q.arg1, q.arg2, q.result);
    }
}

/**
 * @brief Write all quadruples to output file
 * @param fp File pointer for output
 * @details Same format as print_quad but to file
 */
void print_quad_to_file(FILE *fp) {
    fprintf(fp, "===== QUADRUPLES =====\n");
    for (int i = 0; i < quad_count; i++) {
        Quad q = quad_list[i];
        fprintf(fp, "(%d)(%s,%s,%s,%s)\n", 100 + i, q.op, q.arg1, q.arg2, q.result);
    }
}

/**
 * @brief Print symbol table to console
 * @details Shows name, kind, value, and level for each symbol
 */
void print_symbol_table() {
    printf("\n===== SYMBOL TABLE =====\n");
    printf("Name\tType\tValue\tLevel\n");
    
    for (int i = 0; i < symbol_count; i++) {
        Symbol s = symbol_table[i];
        char *kind;
        
        /* Convert kind enum to string */
        if (s.kind == CONST_KIND) kind = "const";
        else if (s.kind == VAR_KIND) kind = "var";
        else kind = "procedure";
        
        printf("%s\t%s\t%d\t%d\n", s.name, kind, s.val, s.level);
    }
}