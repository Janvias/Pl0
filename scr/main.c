/**
 * @file main.c
 * @brief PL/0 Compiler Main Program
 * @description This is the main entry point for the PL/0 compiler.
 *              It coordinates the compilation process:
 *              1. Opens input source file
 *              2. Performs lexical analysis
 *              3. Performs syntax and semantic analysis
 *              4. Generates intermediate code (quadruples)
 *              5. Outputs results to console and file
 * 
 * @author PL/0 Compiler Project
 * @date 2026-05-30
 * @version 1.0
 */

#include "compiler.h"

/*============================================================================
 * Global Variable Definitions
 *============================================================================*/

/** @brief Input file pointer for reading source code */
FILE *fp_in = NULL;

/** @brief Current character being processed */
char ch;

/** @brief Current line number in source file */
int line = 1;

/** @brief Array of all tokens from lexical analysis */
Token token_list[MAX_TOKENS];

/** @brief Total count of tokens */
int token_count = 0;

/** @brief Current token index being processed by parser */
int current_token = 0;

/** @brief Symbol table storing all declared identifiers */
Symbol symbol_table[MAX_SYMBOLS];

/** @brief Total count of symbols in symbol table */
int symbol_count = 0;

/** @brief Current nesting level (scope depth) */
int current_level = 0;

/** @brief List of generated quadruples */
Quad quad_list[MAX_QUADS];

/** @brief Total count of quadruples */
int quad_count = 0;

/** @brief Next instruction address for code generation */
int next_instr = 100;

/** @brief Counter for temporary variable names */
int temp_count = 0;

/** @brief Flag indicating syntax error occurred */
int syntax_error = 0;

/** @brief Flag indicating semantic error occurred */
int semantic_error = 0;

/*============================================================================
 * Main Function
 *============================================================================*/

/**
 * @brief Main entry point of the PL/0 compiler
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, 1 on error
 * @details Usage: compiler <source_file>
 *          The compiler will:
 *          1. Read the source file
 *          2. Perform lexical analysis
 *          3. Perform syntax and semantic analysis
 *          4. Generate quadruples
 *          5. Output results to console and file
 */
int main(int argc, char *argv[]) {
    /* Check command-line arguments */
    if (argc != 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        printf("Example: %s pl0_source.txt\n", argv[0]);
        return 1;
    }
    
    /* Open input source file */
    fp_in = fopen(argv[1], "r");
    if (fp_in == NULL) {
        printf("Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    /* Print compilation header */
    printf("========================================\n");
    printf("       PL/0 Compiler v1.0\n");
    printf("========================================\n");
    printf("Source file: %s\n\n", argv[1]);
    
    /*------------------------------------------------------------------------
     * Phase 1: Lexical Analysis
     *------------------------------------------------------------------------*/
    printf("Phase 1: Lexical Analysis...\n");
    
    /* Initialize character reading */
    get_char();
    
    /* Perform lexical analysis */
    lexical_analysis();
    
    /* Print lexical analysis results */
    print_tokens();
    print_statistics();
    
    printf("\nLexical analysis completed. Total tokens: %d\n", token_count);
    
    /*------------------------------------------------------------------------
     * Phase 2: Syntax and Semantic Analysis
     *------------------------------------------------------------------------*/
    printf("\n========================================\n");
    printf("Phase 2: Syntax and Semantic Analysis...\n\n");
    
    /* Parse the program */
    program();
    
    /* Report analysis results */
    if (syntax_error) {
        printf("\nSyntax analysis completed with errors.\n");
    } else {
        printf("\nSyntax analysis completed successfully.\n");
    }
    
    if (semantic_error) {
        printf("Semantic analysis completed with errors.\n");
    } else {
        printf("Semantic analysis completed successfully.\n");
    }
    
    /*------------------------------------------------------------------------
     * Phase 3: Intermediate Code Generation Output
     *------------------------------------------------------------------------*/
    printf("\n========================================\n");
    printf("Phase 3: Intermediate Code Generation...\n");
    
    /* Print quadruples to console */
    print_quad();
    
    /* Print symbol table */
    print_symbol_table();
    
    /*------------------------------------------------------------------------
     * Phase 4: Output to File
     *------------------------------------------------------------------------*/
    printf("\n========================================\n");
    printf("Phase 4: Writing output file...\n");
    
    /* Create output filename based on input filename */
    char output_filename[256];
    char *dot_pos = strrchr(argv[1], '.');
    
    if (dot_pos != NULL) {
        /* Replace extension with _quad.txt */
        int base_len = dot_pos - argv[1];
        strncpy(output_filename, argv[1], base_len);
        output_filename[base_len] = '\0';
        strcat(output_filename, "_quad.txt");
    } else {
        /* Append _quad.txt if no extension */
        strcpy(output_filename, argv[1]);
        strcat(output_filename, "_quad.txt");
    }
    
    /* Open output file */
    FILE *fp_out = fopen(output_filename, "w");
    if (fp_out == NULL) {
        printf("Error: Cannot create output file '%s'\n", output_filename);
        fclose(fp_in);
        return 1;
    }
    
    /* Write quadruples to output file */
    print_quad_to_file(fp_out);
    
    /* Close output file */
    fclose(fp_out);
    printf("Quadruples written to: %s\n", output_filename);
    
    /*------------------------------------------------------------------------
     * Cleanup and Exit
     *------------------------------------------------------------------------*/
    fclose(fp_in);
    
    printf("\n========================================\n");
    printf("Compilation completed.\n");
    printf("========================================\n");
    
    /* Return error code if errors occurred */
    if (syntax_error || semantic_error) {
        return 1;
    }
    
    return 0;
}