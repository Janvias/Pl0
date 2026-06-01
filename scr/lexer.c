/**
 * @file lexer.c
 * @brief PL/0 Lexical Analyzer Implementation
 * @description This file implements the lexical analysis phase of the PL/0 compiler.
 *              It reads source code and converts it into a stream of tokens.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-05-30
 * @version 1.0
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
 * @details Updates global variable ch and increments line counter on newline
 */
void get_char() {
    ch = fgetc(fp_in);
    if (ch == '\n') {
        line++;
    }
}

/**
 * @brief Skip whitespace characters (spaces, tabs, newlines)
 * @details Continues reading until a non-whitespace character or EOF is found
 */
void skip_white() {
    while (ch != EOF && isspace(ch)) {
        get_char();
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
 * @details Reads until newline or EOF, then continues to next line
 */
void skip_single_comment() {
    /* Read until end of line */
    while (ch != EOF && ch != '\n') {
        get_char();
    }
    /* Skip the newline character */
    if (ch == '\n') {
        get_char();
    }
}

/**
 * @brief Skip multi-line comment delimited by slash-star
 * @details Handles nested content within comment block
 * @note Reports error if comment is not properly closed
 */
void skip_multi_comment() {
    get_char();  /* Skip the initial star character */
    
    /* Search for closing star-slash sequence */
    while (ch != EOF) {
        if (ch == '*') {
            get_char();
            if (ch == '/') {
                get_char();  /* Skip the closing slash */
                return;      /* Comment successfully closed */
            }
        } else {
            get_char();
        }
    }
    
    /* Error: comment not closed before EOF */
    printf("(Lexical Error, Line: %d) Unclosed multi-line comment\n", line);
}

/*============================================================================
 * Token Recognition Functions
 *============================================================================*/

/**
 * @brief Get the next token from source file
 * @details Recognizes identifiers, keywords, numbers, operators, and delimiters
 * @return Token structure containing type, value, and line number
 */
Token get_token() {
    Token t;
    char token_str[MAX_TOKEN_LEN];
    int i = 0;
    
    /* Initialize token structure */
    memset(token_str, 0, MAX_TOKEN_LEN);
    memset(&t, 0, sizeof(Token));
    t.line = line;
    
    /* Skip leading whitespace */
    skip_white();
    
    /*------------------------------------------------------------------------
     * Identifier or Keyword Recognition
     *------------------------------------------------------------------------*/
    if (isalpha(ch)) {
        /* Read alphanumeric characters */
        token_str[i++] = ch;
        get_char();
        while (isalnum(ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = ch;
            get_char();
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
    else if (isdigit(ch)) {
        /* Read digits */
        token_str[i++] = ch;
        get_char();
        while (isdigit(ch) && i < MAX_TOKEN_LEN - 1) {
            token_str[i++] = ch;
            get_char();
        }
        
        /* Check for invalid identifier starting with digit */
        if (isalpha(ch)) {
            /* Read remaining alphanumeric characters */
            while (isalnum(ch) && i < MAX_TOKEN_LEN - 1) {
                token_str[i++] = ch;
                get_char();
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
    else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
             ch == '=' || ch == '#' || ch == '<' || ch == '>') {
        token_str[i++] = ch;
        char c1 = ch;
        get_char();
        
        /* Check for two-character operators: <=, >= */
        if ((c1 == '<' && ch == '=') || (c1 == '>' && ch == '=')) {
            token_str[i++] = ch;
            get_char();
        }
        
        /* Check for single-line comment: double slash */
        if (c1 == '/' && ch == '/') {
            skip_single_comment();
            return get_token();  /* Get next token after comment */
        }
        
        /* Check for multi-line comment: slash-star */
        if (c1 == '/' && ch == '*') {
            skip_multi_comment();
            return get_token();  /* Get next token after comment */
        }
        
        t.type = OPERATOR;
        strcpy(t.value, token_str);
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Delimiter Recognition
     *------------------------------------------------------------------------*/
    else if (ch == ';' || ch == ',' || ch == '.' || ch == '(' || 
             ch == ')' || ch == ':') {
        token_str[i++] = ch;
        char c1 = ch;
        get_char();
        
        /* Check for assignment operator: := */
        if (c1 == ':' && ch == '=') {
            token_str[i++] = ch;
            get_char();
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
    else if (ch == EOF) {
        t.type = ERROR;
        strcpy(t.value, "EOF");
        return t;
    }
    
    /*------------------------------------------------------------------------
     * Invalid Character
     *------------------------------------------------------------------------*/
    else {
        token_str[i++] = ch;
        printf("(Lexical Error, Line: %d) Invalid character: %c\n", 
               t.line, ch);
        t.type = ERROR;
        strcpy(t.value, token_str);
        get_char();
        return t;
    }
}

/*============================================================================
 * Lexical Analysis Driver Function
 *============================================================================*/

/**
 * @brief Perform complete lexical analysis on source file
 * @details Reads entire source file and populates token_list array
 * @note Stops when EOF token is encountered
 */
void lexical_analysis() {
    Token t;
    while (1) {
        t = get_token();
        /* Stop when EOF is reached */
        if (t.type == ERROR && strcmp(t.value, "EOF") == 0) {
            break;
        }
        token_list[token_count++] = t;
    }
}

/*============================================================================
 * Token Access Functions
 *============================================================================*/

/**
 * @brief Get current token from token list
 * @return Token at current_token index
 */
Token current_t() {
    return token_list[current_token];
}

/**
 * @brief Match current token against expected type and value
 * @param type Expected token type
 * @param value Expected token value string
 * @details If match succeeds, advances to next token; otherwise reports error
 */
void match(TokenType type, const char *value) {
    Token t = current_t();
    if (t.type == type && strcmp(t.value, value) == 0) {
        current_token++;  /* Advance to next token */
    } else {
        printf("(Syntax Error, Line: %d) Expected '%s', got '%s'\n", 
               t.line, value, t.value);
        syntax_error = 1;
    }
}

/**
 * @brief Advance to next token in token list
 */
void next_token_func() {
    current_token++;
}

/*============================================================================
 * Output Functions
 *============================================================================*/

/**
 * @brief Print all tokens to console in standard format
 * @details Format: (type,value) for each token
 */
void print_tokens() {
    printf("\n===== LEXICAL ANALYSIS RESULT =====\n");
    for (int i = 0; i < token_count; i++) {
        Token t = token_list[i];
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

/**
 * @brief Print token statistics summary
 * @details Shows counts for each token category and total
 */
void print_statistics() {
    int count_key = 0, count_id = 0, count_num = 0, count_op = 0, count_del = 0;
    
    /* Count tokens by type */
    for (int i = 0; i < token_count; i++) {
        switch (token_list[i].type) {
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