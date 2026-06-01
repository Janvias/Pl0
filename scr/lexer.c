/**
 * @file lexer.c
 * @brief PL/0 Lexical Analyzer Implementation
 * @description This file implements the lexical analysis phase of the PL/0 compiler.
 *              It reads source code and converts it into a stream of tokens.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 2.0
 */

#include "compiler.h"

/*============================================================================
 * Private Data - Keyword Table
 *============================================================================*/

/**
 * @brief Array of PL/0 reserved keywords
 * @description These keywords have special meaning and cannot be used as identifiers
 */
char *keyword[] = {
    "const",      /**< Constant declaration keyword */
    "var",        /**< Variable declaration keyword */
    "procedure",  /**< Procedure declaration keyword */
    "begin",      /**< Block start keyword */
    "end",        /**< Block end keyword */
    "if",         /**< Conditional statement keyword */
    "then",       /**< Conditional branch keyword */
    "while",      /**< Loop statement keyword */
    "do",         /**< Loop body keyword */
    "call",       /**< Procedure call keyword */
    "read",       /**< Input statement keyword */
    "write",      /**< Output statement keyword */
    "odd",        /**< Odd condition keyword */
    NULL          /**< End of keyword list marker */
};

/*============================================================================
 * Character Handling Functions
 *============================================================================*/

/**
 * @brief Read next character from input file
 * @param state Compiler state
 * @details Updates state->ch and increments line counter on newline
 */
void get_char(CompilerState* state) {
    state->ch = fgetc(state->fp_in);
    if (state->ch == '\n') {
        state->line++;
    }
}

/**
 * @brief Skip whitespace characters (spaces, tabs, newlines)
 * @param state Compiler state
 * @details Continues reading until a non-whitespace character or EOF is found
 */
void skip_white(CompilerState* state) {
    while (state->ch != EOF && isspace(state->ch)) {
        get_char(state);
    }
}

/**
 * @brief Check if a string matches a PL/0 keyword
 * @param s The string to check
 * @return 1 if the string is a keyword, 0 otherwise
 */
int is_keyword(char *s) {
    int i = 0;
    while (keyword[i] != NULL) {
        if (strcmp(s, keyword[i]) == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

/*============================================================================
 * Comment Handling Functions
 *============================================================================*/

/**
 * @brief Skip single-line comment starting with double slash
 * @param state Compiler state
 * @details Reads until newline or EOF, then continues to next line
 */
void skip_single_comment(CompilerState* state) {
    /* Read until end of line */
    while (state->ch != EOF && state->ch != '\n') {
        get_char(state);
    }
    /* Skip the newline character */
    if (state->ch == '\n') {
        get_char(state);
    }
}

/**
 * @brief Skip multi-line comment delimited by slash-star
 * @param state Compiler state
 * @details Handles nested content within comment block
 * @note Reports error if comment is not properly closed
 */
void skip_multi_comment(CompilerState* state) {
    get_char(state);  /* Skip the initial star character */
    
    /* Search for closing star-slash sequence */
    while (state->ch != EOF) {
        if (state->ch == '*') {
            get_char(state);
            if (state->ch == '/') {
                get_char(state);  /* Skip the closing slash */
                return;      /* Comment successfully closed */
            }
        } else {
            get_char(state);
        }
    }
    
    /* Error: comment not closed before EOF */
    printf("(Lexical Error, Line: %d) Unclosed multi-line comment\n", state->line);
}

/*============================================================================
 * Token Recognition Functions
 *============================================================================*/

/**
 * @brief Get the next token from source file
 * @param state Compiler state
 * @details Recognizes identifiers, keywords, numbers, operators, and delimiters
 * @return Token structure containing type, value, and line number
 */
Token get_token(CompilerState* state) {
    Token t;
    char token_str[MAX_TOKEN_LEN];
    int i = 0;
    
    /* Initialize token structure */
    memset(token_str, 0, MAX_TOKEN_LEN);
    memset(&t, 0, sizeof(Token));
    t.line = state->line;
    
    /* Skip leading whitespace */
    skip_white(state);
    
    /*------------------------------------------------------------------------
     * Identifier or Keyword Recognition
     *------------------------------------------------------------------------*/
    if (isalpha(state->ch)) {
        /* Read alphanumeric characters */
        token_str[i++] = state->ch;
        get_char(state);
        while (isalnum(state->ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        /* Check if it's a keyword or identifier */
        if (is_keyword(token_str)) {
            t.type = KEYWORD;
        } else {
            /* Check identifier length constraint (max 8 characters) */
            if (i > 8) {
                printf("(Lexical Error, Line: %d) Identifier too long: %s\n", 
                       t.line, token_str);
                t.type = ERROR;
            } else {
                t.type = IDENTIFIER;
            }
        }
        strcpy(t.value, token_str);
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Number Recognition
     *------------------------------------------------------------------------*/
    else if (isdigit(state->ch)) {
        /* Read digits */
        token_str[i++] = state->ch;
        get_char(state);
        while (isdigit(state->ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        /* Check for invalid identifier starting with digit */
        if (isalpha(state->ch)) {
            /* Read remaining alphanumeric characters */
            while (isalnum(state->ch) && i < MAX_TOKEN_LEN - 1) {
                token_str[i++] = state->ch;
                get_char(state);
            }
            printf("(Lexical Error, Line: %d) Invalid word: %s\n", 
                   t.line, token_str);
            t.type = ERROR;
            strcpy(t.value, token_str);
            return t;
        }
        
        /* Check number length constraint (max 8 digits) */
        if (i > 8) {
            printf("(Lexical Error, Line: %d) Number too long: %s\n", 
                   t.line, token_str);
            t.type = ERROR;
        } else {
            t.type = NUMBER;
        }
        strcpy(t.value, token_str);
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Operator Recognition
     *------------------------------------------------------------------------*/
    else if (state->ch == '+' || state->ch == '-' || state->ch == '*' || state->ch == '/' ||
             state->ch == '=' || state->ch == '#' || state->ch == '<' || state->ch == '>') {
        token_str[i++] = state->ch;
        char c1 = state->ch;
        get_char(state);
        
        /* Check for two-character operators: <=, >= */
        if ((c1 == '<' && state->ch == '=') || (c1 == '>' && state->ch == '=')) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        /* Check for single-line comment: double slash */
        if (c1 == '/' && state->ch == '/') {
            skip_single_comment(state);
            return get_token(state);  /* Get next token after comment */
        }
        
        /* Check for multi-line comment: slash-star */
        if (c1 == '/' && state->ch == '*') {
            skip_multi_comment(state);
            return get_token(state);  /* Get next token after comment */
        }
        
        t.type = OPERATOR;
        strcpy(t.value, token_str);
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Delimiter Recognition
     *------------------------------------------------------------------------*/
    else if (state->ch == ';' || state->ch == ',' || state->ch == '.' || state->ch == '(' || 
             state->ch == ')' || state->ch == ':') {
        token_str[i++] = state->ch;
        char c1 = state->ch;
        get_char(state);
        
        /* Check for assignment operator: := */
        if (c1 == ':' && state->ch == '=') {
            token_str[i++] = state->ch;
            get_char(state);
            t.type = OPERATOR;  /* := is treated as operator */
        } else {
            t.type = DELIMITER;
        }
        strcpy(t.value, token_str);
        return t;
    }
    
    /*------------------------------------------------------------------------
     * End of File
     *------------------------------------------------------------------------*/
    else if (state->ch == EOF) {
        t.type = ERROR;
        strcpy(t.value, "EOF");
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Invalid Character
     *------------------------------------------------------------------------*/
    else {
        token_str[i++] = state->ch;
        printf("(Lexical Error, Line: %d) Invalid character: %c\n", 
               t.line, state->ch);
        t.type = ERROR;
        strcpy(t.value, token_str);
        get_char(state);
        return t;
    }
}

/*============================================================================
 * Lexical Analysis Driver Function
 *============================================================================*/

/**
 * @brief Perform complete lexical analysis on source file
 * @param state Compiler state
 * @details Reads entire source file and populates token_list array
 * @note Stops when EOF token is encountered
 */
void lexical_analysis(CompilerState* state) {
    Token t;
    while (1) {
        t = get_token(state);
        /* Stop when EOF is reached */
        if (t.type == ERROR && strcmp(t.value, "EOF") == 0) {
            break;
        }
        state->token_list[state->token_count++] = t;
    }
}

/*============================================================================
 * Token Access Functions
 *============================================================================*/

/**
 * @brief Get current token from token list
 * @param state Compiler state
 * @return Token at current_token index
 */
Token current_t(CompilerState* state) {
    return state->token_list[state->current_token];
}

/**
 * @brief Match current token against expected type and value
 * @param state Compiler state
 * @param type Expected token type
 * @param value Expected token value string
 * @details If match succeeds, advances to next token; otherwise reports error
 */
void match(CompilerState* state, TokenType type, const char *value) {
    Token t = current_t(state);
    if (t.type == type && strcmp(t.value, value) == 0) {
        state->current_token++;  /* Advance to next token */
    } else {
        printf("(Syntax Error, Line: %d) Expected '%s', got '%s'\n", 
               t.line, value, t.value);
        state->syntax_error = 1;
    }
}

/**
 * @brief Advance to next token in token list
 * @param state Compiler state
 */
void next_token_func(CompilerState* state) {
    state->current_token++;
}

/*============================================================================
 * Output Functions
 *============================================================================*/

/**
 * @brief Print all tokens to console in standard format
 * @param state Compiler state
 * @details Format: (type,value) for each token
 */
void print_tokens(CompilerState* state) {
    printf("\n===== LEXICAL ANALYSIS RESULT =====\n");
    for (int i = 0; i < state->token_count; i++) {
        Token t = state->token_list[i];
        switch (t.type) {
            case KEYWORD:    printf("(keyword,%s)\n", t.value); break;
            case IDENTIFIER: printf("(identifier,%s)\n", t.value); break;
            case NUMBER:     printf("(number,%s)\n", t.value); break;
            case OPERATOR:   printf("(operator,%s)\n", t.value); break;
            case DELIMITER:  printf("(delimiter,%s)\n", t.value); break;
            default:         break;
        }
    }
}
 
 * @brief Print token statistics summary
 * @param state Compiler state
 * @details Shows counts for each token category and total
 */
void print_statistics(CompilerState* state) {
    int count_key = 0, count_id = 0, count_num = 0, count_op = 0, count_del = 0;
    
    /* Count tokens by type */
    for (int i = 0; i < state->token_count; i++) {
        switch (state->token_list[i].type) {
            case KEYWORD:    count_key++; break;
            case IDENTIFIER: count_id++; break;
            case NUMBER:     count_num++; break;
            case OPERATOR:   count_op++; break;
            case DELIMITER:  count_del++; break;
            default:         break;
        }
    }
    
    /* Print statistics */
    printf("\n===== TOKEN STATISTICS =====\n");
    printf("keyword: %d\n", count_key);
    printf("identifier: %d\n", count_id);
    printf("number: %d\n", count_num);
    printf("operator: %d\n", count_op);
    printf("delimiter: %d\n", count_del);
    printf("total: %d\n", count_key + count_id + count_num + count_op + count_del);
}

/*============================================================================
 * Visualization Functions
 *============================================================================*/

/**
 * @brief Print word classification table
 * @param state Compiler state
 * @details Shows detailed classification of all tokens with line numbers
 */
void print_classification_table(CompilerState* state) {
    printf("\n===== WORD CLASSIFICATION TABLE =====\n");
    printf("%-6s %-12s %-15s %-6s\n", "Index", "Type", "Value", "Line");
    printf("------ ------------ --------------- ------\n");
    
    for (int i = 0; i < state->token_count; i++) {
        Token t = state->token_list[i];
        const char* type_str;
        switch (t.type) {
            case KEYWORD:    type_str = "KEYWORD";    break;
            case IDENTIFIER: type_str = "IDENTIFIER"; break;
            case NUMBER:     type_str = "NUMBER";     break;
            case OPERATOR:   type_str = "OPERATOR";   break;
            case DELIMITER:  type_str = "DELIMITER";  break;
            default:         type_str = "ERROR";      break;
        }
        printf("%-6d %-12s %-15s %-6d\n", i, type_str, t.value, t.line);
    }
}

/**
 * @brief Print state transition diagram (text-based)
 * @details Shows the finite state machine for lexical analysis
 */
void print_state_transition_diagram() {
    printf("\n===== STATE TRANSITION DIAGRAM =====\n");
    printf("Finite State Machine for PL/0 Lexical Analysis\n");
    printf("================================================\n\n");
    
    printf("States:\n");
    printf("  S0: Start state\n");
    printf("  S1: Identifier/Keyword (letter)\n");
    printf("  S2: Number (digit)\n");
    printf("  S3: Operator (+, -, *, /, =, #, <, >)\n");
    printf("  S4: Delimiter (;, ,, ., (, ), :)\n");
    printf("  S5: Two-char operator (<=, >=, :=)\n");
    printf("  S6: Comment (// or /*)\n");
    printf("  SF: Final state (token recognized)\n\n");
    
    printf("Transition Table:\n");
    printf("+-------+--------+--------+--------+--------+--------+--------+--------+\n");
    printf("| State | letter | digit  | op_char| del_char|  :     |  /     |  EOF   |\n");
    printf("+-------+--------+--------+--------+--------+--------+--------+--------+\n");
    printf("|   S0  |  S1    |  S2    |  S3    |  S4    |  S4    |  S3    |  SF    |\n");
    printf("|   S1  |  S1    |  S1    |  SF    |  SF    |  SF    |  SF    |  SF    |\n");
    printf("|   S2  |  ERROR |  S2    |  SF    |  SF    |  SF    |  SF    |  SF    |\n");
    printf("|   S3  |  SF    |  SF    |  SF    |  SF    |  S5    |  S6    |  SF    |\n");
    printf("|   S4  |  SF    |  SF    |  SF    |  SF    |  S5    |  SF    |  SF    |\n");
    printf("|   S5  |  SF    |  SF    |  SF    |  SF    |  SF    |  SF    |  SF    |\n");
    printf("|   S6  |  S6    |  S6    |  S6    |  S6    |  S6    |  S6*   |  ERROR |\n");
    printf("+-------+--------+--------+--------+--------+--------+--------+--------+\n");
    printf("Note: S6* transitions to SF when closing */ is found\n");
}

/**
 * @brief Print lexical recognition flowchart (text-based)
 * @details Shows the flow of token recognition process
 */
void print_recognition_flowchart() {
    printf("\n===== LEXICAL RECOGNITION FLOWCHART =====\n");
    printf("Token Recognition Process Flow\n");
    printf("===============================\n\n");
    
    printf("                          Start\n");
    printf("                            |\n");
    printf("                            v\n");
    printf("                  Read next character\n");
    printf("                            |\n");
    printf("                            v\n");
    printf("              +------------+------------+\n");
    printf("              |                         |\n");
    printf("              v                         v\n");
    printf("         [Whitespace?]              [EOF?]\n");
    printf("              |                         |\n");
    printf("         Yes  |  No              Yes  |  No\n");
    printf("              v  v                     v\n");
    printf("         Skip char              +------+------+\n");
    printf("              |                 |             |\n");
    printf("              +------+          v             v\n");
    printf("                     |    [Letter?]      [Digit?]\n");
    printf("                     |       |             |\n");
    printf("                     |  Yes  |  No    Yes  |  No\n");
    printf("                     |       v  v          v\n");
    printf("                     |  Identifier    Number\n");
    printf("                     |       |             |\n");
    printf("                     |       v             v\n");
    printf("                     |  [Keyword?]      +---+---+\n");
    printf("                     |       |           |       |\n");
    printf("                     |  Yes  |  No   [Valid?]  [Valid?]\n");
    printf("                     |       v  v       | Yes   | No\n");
    printf("                     |  Keyword  ID   +--+--+  +--+--+\n");
    printf("                     |                 |     |  |     |\n");
    printf("                     +--------+--------+     |  |     |\n");
    printf("                              |              |  |     |\n");
    printf("                              v              v  v     v\n");
    printf("                         +---+---+       Valid  ERROR\n");
    printf("                         |       |       Num    Num\n");
    printf("                         v       v\n");
    printf("                    [Operator?]  [Delimiter?]\n");
    printf("                         |       |\n");
    printf("                    Yes  |  No   v\n");
    printf("                         v  v  Process\n");
    printf("                    Process  ERROR\n");
    printf("                         |\n");
    printf("                         v\n");
    printf("                      Output Token\n");
    printf("                         |\n");
    printf("                         v\n");
    printf("                       Repeat\n");
}