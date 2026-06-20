# AI Code Generation Guidelines for PL/0 Compiler Project

## Output Language

**All user-facing output MUST be in English.** This includes:
- Compiler messages (errors, warnings, info)
- Test output (pass/fail messages, summaries)
- Console/log output
- Generated reports
- Any text written to stdout/stderr

## Comment Language

**All code comments MUST be in Chinese (中文).** This includes:
- File header `@file` / `@brief` / `@details` / `@author` / `@date` Doxygen blocks
- Function documentation comments
- Inline comments explaining logic
- Section separator comments (`//====...`)

Example:
```cpp
/**
 * @file lexer.cpp
 * @brief PL/0编译器词法分析器实现
 * @details 逐字符读取源代码，识别Token类型
 */
void Lexer::readChar() {
    // 读取下一个字符，更新行号
    file_.get(currentChar_);
    if (currentChar_ == '\n') currentLine_++;
}
```

## Naming Conventions

- **Identifiers**: English only (variables, functions, classes, enums, macros)
- **File names**: English only, snake_case (`pl0_lexer.hpp`, `lr1_parser_driver.cpp`)
- **Git commits**: English (Chinese allowed in commit body for detailed explanation)

## Build & Test Commands

```bash
# Build main compiler (from scr/)
cd scr
mingw32-make

# Build & run tests (from scr/tests/)
cd scr/tests
mingw32-make          # build
mingw32-make run      # run tests (58 tests, expect 100% pass)
```

## Code Style Quick Reference

| Element | Language |
|---------|----------|
| Comments | 中文 (Chinese) |
| User-facing output | English |
| Identifiers (vars, functions, classes) | English |
| File names | English |
| Git commit messages | English |
| Error messages | English |
| Test assertions / messages | English |

## Project-Specific Rules

1. **PL/0 language limitations**: No `else` clause, no multi-const (`const a=1,b=2;`), identifiers max 8 chars, numbers max 8 digits, no underscore in identifiers
2. **Compilation quirks**: `lexer.cpp` and `compiler.cpp` require `-O0` on MinGW to avoid segfault
3. **DFA stubs**: Use `dfa_stubs.cpp` not `dfa_wrapper.cpp` (they conflict — stubs are the fast-dev path)
4. **Dual parser**: LL(1) + LR(1) must both succeed; quad differences are informational (allowed)
5. **Working directory**: Always `cd` to the correct directory before running make commands
