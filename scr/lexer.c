/**
 * @file lexer.c
 * @brief PL/0 Lexical Analyzer Implementation
 * @description This file implements the lexical analysis phase of the PL/0 compiler.
 *              It reads source code and converts it into a stream of tokens.
 *              Integrated with DFA-based regex matching for token validation.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 2.0
 */

#include "compiler.h"
#include "pl0RegexDfa/dfa_wrapper.h"

/*============================================================================
 * Private Data - Keyword Table
 *============================================================================*/

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
 * DFA Instances for Token Validation
 *============================================================================*/

static void* dfa_ident = NULL;
static void* dfa_number = NULL;

/*============================================================================
 * Character Handling Functions
 *============================================================================*/

void get_char(CompilerState* state) {
    state->ch = fgetc(state->fp_in);
    if (state->ch == '\n') {
        state->line++;
    }
}

void skip_white(CompilerState* state) {
    while (state->ch != EOF && isspace(state->ch)) {
        get_char(state);
    }
}

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
 * DFA Initialization and Cleanup
 *============================================================================*/

void init_dfa() {
    dfa_ident = create_dfa_from_regex("[a-zA-Z][a-zA-Z0-9]*");
    dfa_number = create_dfa_from_regex("[0-9]+");
}

void destroy_dfa_instances() {
    if (dfa_ident) {
        destroy_dfa(dfa_ident);
        dfa_ident = NULL;
    }
    if (dfa_number) {
        destroy_dfa(dfa_number);
        dfa_number = NULL;
    }
}

/*============================================================================
 * Comment Handling Functions
 *============================================================================*/

void skip_single_comment(CompilerState* state) {
    while (state->ch != EOF && state->ch != '\n') {
        get_char(state);
    }
    if (state->ch == '\n') {
        get_char(state);
    }
}

void skip_multi_comment(CompilerState* state) {
    get_char(state);
    
    while (state->ch != EOF) {
        if (state->ch == '*') {
            get_char(state);
            if (state->ch == '/') {
                get_char(state);
                return;
            }
        } else {
            get_char(state);
        }
    }
    
    printf("(Lexical Error, Line: %d) Unclosed multi-line comment\n", state->line);
}

/*============================================================================
 * Token Recognition Functions
 *============================================================================*/

Token get_token(CompilerState* state) {
    Token t;
    char token_str[MAX_TOKEN_LEN];
    int i = 0;
    
    memset(token_str, 0, MAX_TOKEN_LEN);
    memset(&t, 0, sizeof(Token));
    t.line = state->line;
    
    skip_white(state);
    
    if (isalpha(state->ch)) {
        token_str[i++] = state->ch;
        get_char(state);
        while (isalnum(state->ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        if (is_keyword(token_str)) {
            t.type = KEYWORD;
        } else {
            int dfa_result = dfa_accepts(dfa_ident, token_str);
            if (!dfa_result) {
                printf("(Lexical Error, Line: %d) Invalid identifier format: %s\n", 
                       t.line, token_str);
                t.type = ERROR;
            } else if (i > 8) {
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
    
    else if (isdigit(state->ch)) {
        token_str[i++] = state->ch;
        get_char(state);
        while (isdigit(state->ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        if (isalpha(state->ch)) {
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
        
        int dfa_result = dfa_accepts(dfa_number, token_str);
        if (!dfa_result) {
            printf("(Lexical Error, Line: %d) Invalid number format: %s\n", 
                   t.line, token_str);
            t.type = ERROR;
        } else if (i > 8) {
            printf("(Lexical Error, Line: %d) Number too long: %s\n", 
                   t.line, token_str);
            t.type = ERROR;
        } else {
            t.type = NUMBER;
        }
        strcpy(t.value, token_str);
        return t;
    }
    
    else if (state->ch == '+' || state->ch == '-' || state->ch == '*' || state->ch == '/' ||
             state->ch == '=' || state->ch == '#' || state->ch == '<' || state->ch == '>') {
        token_str[i++] = state->ch;
        char c1 = state->ch;
        get_char(state);
        
        if ((c1 == '<' && state->ch == '=') || (c1 == '>' && state->ch == '=')) {
            token_str[i++] = state->ch;
            get_char(state);
        }
        
        if (c1 == '/' && state->ch == '/') {
            skip_single_comment(state);
            return get_token(state);
        }
        
        if (c1 == '/' && state->ch == '*') {
            skip_multi_comment(state);
            return get_token(state);
        }
        
        t.type = OPERATOR;
        strcpy(t.value, token_str);
        return t;
    }
    
    else if (state->ch == ';' || state->ch == ',' || state->ch == '.' || state->ch == '(' || 
             state->ch == ')' || state->ch == ':') {
        token_str[i++] = state->ch;
        char c1 = state->ch;
        get_char(state);
        
        if (c1 == ':' && state->ch == '=') {
            token_str[i++] = state->ch;
            get_char(state);
            t.type = OPERATOR;
        } else {
            t.type = DELIMITER;
        }
        strcpy(t.value, token_str);
        return t;
    }
    
    else if (state->ch == EOF) {
        t.type = ERROR;
        strcpy(t.value, "EOF");
        return t;
    }
    
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

void lexical_analysis(CompilerState* state) {
    init_dfa();
    
    Token t;
    while (1) {
        t = get_token(state);
        if (t.type == ERROR && strcmp(t.value, "EOF") == 0) {
            break;
        }
        state->token_list[state->token_count++] = t;
    }
    
    destroy_dfa_instances();
}

/*============================================================================
 * Token Access Functions
 *============================================================================*/

Token current_t(CompilerState* state) {
    return state->token_list[state->current_token];
}

void match(CompilerState* state, TokenType type, const char *value) {
    Token t = current_t(state);
    if (t.type == type && strcmp(t.value, value) == 0) {
        state->current_token++;
    } else {
        printf("(Syntax Error, Line: %d) Expected '%s', got '%s'\n", 
               t.line, value, t.value);
        state->syntax_error = 1;
    }
}

void next_token_func(CompilerState* state) {
    state->current_token++;
}

/*============================================================================
 * Output Functions
 *============================================================================*/

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

void print_statistics(CompilerState* state) {
    int count_key = 0, count_id = 0, count_num = 0, count_op = 0, count_del = 0;
    
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
