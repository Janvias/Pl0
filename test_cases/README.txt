PL/0 Compiler Test Suite
========================

This directory contains test cases for the PL/0 compiler.

Test Files
----------

Normal Test Cases:
------------------
01_basic.txt         - Basic declarations and arithmetic expressions
02_if_then.txt       - Conditional statement (if-then)
03_while_loop.txt    - While loop statement
04_procedure.txt     - Procedure declaration and call
05_read_write.txt    - Input and output statements
06_comments.txt      - Single-line and multi-line comments
07_complex.txt       - Complex program with all features

Error Test Cases:
-----------------
08_error_undeclared.txt     - Undeclared variable error
09_error_redeclared.txt     - Redeclared identifier error
10_error_invalid_id.txt     - Invalid identifier (starts with number)

Running Tests
-------------

Method 1: Using the test script
    run_tests.bat

Method 2: Manual testing
    cd ..
    compiler.exe test_cases\01_basic.txt

Expected Results
----------------

Normal Test Cases:
    - All should compile successfully
    - No syntax or semantic errors
    - Quadruples should be generated

Error Test Cases:
    - Should report appropriate error messages
    - Error type and line number should be displayed

Test Coverage
-------------

This test suite covers:
    - Lexical analysis:
        * Keywords, identifiers, numbers
        * Operators and delimiters
        * Single-line comments (//)
        * Multi-line comments (/* */)
        * Error detection (invalid chars, long identifiers)

    - Syntax analysis:
        * Constant declarations
        * Variable declarations
        * Procedure declarations
        * Assignment statements
        * Compound statements
        * Conditional statements (if-then)
        * Loop statements (while-do)
        * Procedure calls

    - Semantic analysis:
        * Undeclared variable detection
        * Redeclared identifier detection
        * Procedure used as variable detection

    - Code generation:
        * Quadruple generation
        * Temporary variable generation (T1, T2, ...)
        * Jump instruction handling