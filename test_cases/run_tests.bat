@echo off
REM Test Script for PL/0 Compiler
REM This script runs all test cases and displays results

echo ========================================
echo     PL/0 Compiler Test Suite
echo ========================================
echo.

set COMPILER=compiler.exe
set TEST_DIR=test_cases
set OUTPUT_DIR=test_results

REM Create output directory
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

echo Running Test 01: Basic Declarations and Expressions
echo ----------------------------------------
%COMPILER% %TEST_DIR%\01_basic.txt > %OUTPUT_DIR%\01_basic_result.txt
type %OUTPUT_DIR%\01_basic_result.txt
echo.

echo Running Test 02: If-Then Statement
echo ----------------------------------------
%COMPILER% %TEST_DIR%\02_if_then.txt > %OUTPUT_DIR%\02_if_then_result.txt
type %OUTPUT_DIR%\02_if_then_result.txt
echo.

echo Running Test 03: While Loop
echo ----------------------------------------
%COMPILER% %TEST_DIR%\03_while_loop.txt > %OUTPUT_DIR%\03_while_loop_result.txt
type %OUTPUT_DIR%\03_while_loop_result.txt
echo.

echo Running Test 04: Procedure Declaration and Call
echo ----------------------------------------
%COMPILER% %TEST_DIR%\04_procedure.txt > %OUTPUT_DIR%\04_procedure_result.txt
type %OUTPUT_DIR%\04_procedure_result.txt
echo.

echo Running Test 05: Read and Write Statements
echo ----------------------------------------
%COMPILER% %TEST_DIR%\05_read_write.txt > %OUTPUT_DIR%\05_read_write_result.txt
type %OUTPUT_DIR%\05_read_write_result.txt
echo.

echo Running Test 06: Comments
echo ----------------------------------------
%COMPILER% %TEST_DIR%\06_comments.txt > %OUTPUT_DIR%\06_comments_result.txt
type %OUTPUT_DIR%\06_comments_result.txt
echo.

echo Running Test 07: Complex Program
echo ----------------------------------------
%COMPILER% %TEST_DIR%\07_complex.txt > %OUTPUT_DIR%\07_complex_result.txt
type %OUTPUT_DIR%\07_complex_result.txt
echo.

echo ========================================
echo     Error Test Cases
echo ========================================
echo.

echo Running Test 08: Error - Undeclared Variable
echo ----------------------------------------
%COMPILER% %TEST_DIR%\08_error_undeclared.txt > %OUTPUT_DIR%\08_error_result.txt 2>&1
type %OUTPUT_DIR%\08_error_result.txt
echo.

echo Running Test 09: Error - Redeclared Identifier
echo ----------------------------------------
%COMPILER% %TEST_DIR%\09_error_redeclared.txt > %OUTPUT_DIR%\09_error_result.txt 2>&1
type %OUTPUT_DIR%\09_error_result.txt
echo.

echo Running Test 10: Error - Invalid Identifier
echo ----------------------------------------
%COMPILER% %TEST_DIR%\10_error_invalid_id.txt > %OUTPUT_DIR%\10_error_result.txt 2>&1
type %OUTPUT_DIR%\10_error_result.txt
echo.

echo ========================================
echo     Test Suite Completed
echo ========================================
echo Results saved in: %OUTPUT_DIR%