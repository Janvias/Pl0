/**
 * @file main.c
 * @brief PL/0 Compiler Main Program
 * @description This is the main entry point for the PL/0 compiler.
 *              It coordinates the compilation process using the CompilerState
 *              structure to manage all compiler state, eliminating global variables.
 *              1. Opens input source file
 *              2. Performs lexical analysis
 *              3. Performs syntax and semantic analysis
 *              4. Generates intermediate code (quadruples)
 *              5. Outputs results to console and file
 *              6. Writes compiler state to cache file
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 2.0
 */

#include "compiler.h"

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
 *          6. Write compiler state to cache file
 */
int main(int argc, char *argv[]) {
    /* Check command-line arguments */
    if (argc != 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        printf("Example: %s pl0_source.txt\n", argv[0]);
        return 1;
    }
    
    /* Open input source file */
    FILE* fp_in = fopen(argv[1], "r");
    if (fp_in == NULL) {
        printf("Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    /* Initialize compiler state - replaces global variables */
    CompilerState state;
    if (!init_compiler_state(&state, fp_in)) {
        printf("Error: Cannot initialize compiler state\n");
        fclose(fp_in);
        return 1;
    }
    
    /* Print compilation header */
    printf("========================================\n");
    printf("       PL/0 Compiler v2.0\n");
    printf("========================================\n");
    printf("Source file: %s\n\n", argv[1]);
    
    /*------------------------------------------------------------------------
     * Phase 1: Lexical Analysis
     *------------------------------------------------------------------------*/
    printf("Phase 1: Lexical Analysis...\n");
    
    /* Initialize character reading */
    get_char(&state);
    
    /* Perform lexical analysis */
    lexical_analysis(&state);
    
    /* Print lexical analysis results */
    print_tokens(&state);
    print_statistics(&state);
    print_classification_table(&state);
    print_state_transition_diagram();
    print_recognition_flowchart();
    
    printf("\nLexical analysis completed. Total tokens: %d\n", state.token_count);
    
    /*------------------------------------------------------------------------
     * Phase 2: Syntax and Semantic Analysis
     *------------------------------------------------------------------------*/
    printf("\n========================================\n");
    printf("Phase 2: Syntax and Semantic Analysis...\n\n");
    
    /* Parse the program */
    program(&state);
    
    /* Report analysis results */
    if (state.syntax_error) {
        printf("\nSyntax analysis completed with errors.\n");
    } else {
        printf("\nSyntax analysis completed successfully.\n");
    }
    
    if (state.semantic_error) {
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
    print_quad(&state);
    
    /* Print symbol table */
    print_symbol_table(&state);
    
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
        destroy_compiler_state(&state);
        fclose(fp_in);
        return 1;
    }
    
    /* Write quadruples to output file */
    print_quad_to_file(&state, fp_out);
    
    /* Close output file */
    fclose(fp_out);
    printf("Quadruples written to: %s\n", output_filename);
    
    /*------------------------------------------------------------------------
     * Phase 5: Write Cache File
     *------------------------------------------------------------------------*/
    printf("\n========================================\n");
    printf("Phase 5: Writing cache file...\n");
    
    /* Create cache filename based on input filename */
    char cache_filename[256];
    if (dot_pos != NULL) {
        /* Replace extension with _cache.txt */
        int base_len = dot_pos - argv[1];
        strncpy(cache_filename, argv[1], base_len);
        cache_filename[base_len] = '\0';
        strcat(cache_filename, "_cache.txt");
    } else {
        /* Append _cache.txt if no extension */
        strcpy(cache_filename, argv[1]);
        strcat(cache_filename, "_cache.txt");
    }
    
    /* Write compiler state to cache file */
    write_compiler_state_to_cache(&state, cache_filename);
    
    /*------------------------------------------------------------------------
     * Cleanup and Exit
     *------------------------------------------------------------------------*/
    /* Destroy compiler state */
    destroy_compiler_state(&state);
    
    /* Close input file */
    fclose(fp_in);
    
    printf("\n========================================\n");
    printf("Compilation completed.\n");
    printf("========================================\n");
    
    /* Return error code if errors occurred */
    if (state.syntax_error || state.semantic_error) {
        return 1;
    }
    
    return 0;
}
