# CLAUDE.md

## Project Overview

PL/0 compiler in C++17 with **dual-validation syntax analysis**: LL(1) recursive descent + LR(1) shift-reduce parsers running side-by-side for cross-validation.

## Build & Run

```bash
# Build (from scr/)
cd scr
g++ -std=c++17 -O2 -Wall -I. -c src/lexer.cpp src/parser.cpp src/symtable.cpp src/codegen.cpp
g++ -std=c++17 -O2 -Wall -I. -c src/lr1_grammar.cpp src/lr1_items.cpp src/lr1_parser_driver.cpp src/lr1_diagnostics.cpp
g++ -std=c++17 -O2 -Wall -I. -c src/compiler.cpp main.cpp dfa/regex_access.cpp dfa/dfa_stubs.cpp
g++ -std=c++17 -O2 -Wall -I. *.o -o build/pl0_compiler
# Or: make

# Run (dual mode by default)
./build/pl0_compiler test.pl0 --dual
```

## Architecture

```
Lexer (token stream)
  │
  ├── LL(1) Parser (parser.cpp) — recursive descent, inline semantic actions
  └── LR(1) Parser (4 modules) — canonical LR(1), 382 states, shift-reduce
        │
        └── Compiler (compiler.cpp) — compares both results
              │
              └── CodeGenerator (codegen.cpp) — quadruple IR
```

## Key Files

| File | Purpose |
|------|---------|
| `include/pl0_core.hpp` | TokenType, OpCode, Token, Symbol, Quadruple structs |
| `include/pl0_lr1_parser.hpp` | LR(1) types: LR1Terminal, LR1NonTerminal, LR1Item, LRAction, ParserMode enum |
| `src/parser.cpp` | LL(1) recursive descent — reference implementation |
| `src/lr1_grammar.cpp` | 46 grammar productions, FIRST/FOLLOW sets |
| `src/lr1_items.cpp` | LR(1) closure, canonical collection (382 states), parse tables |
| `src/lr1_parser_driver.cpp` | Shift-reduce loop, token→terminal mapping, semantic actions |
| `src/lr1_diagnostics.cpp` | Symbol naming, parse table output, `compareParserResults()` |
| `src/compiler.cpp` | Orchestrates dual parsing; `ParserMode::LL1_ONLY / LR1_ONLY / DUAL` |
| `dfa/dfa_stubs.cpp` | Stub DFA functions (bypass slow DFA construction for fast dev) |

## LR(1) Grammar (46 productions)

The grammar uses right-recursion for expressions (LR handles left-recursion naturally but this matches the LL(1) semantics):
- `E → T ET` where `ET → + T ET | - T ET | ε`
- `T → F TT` where `TT → * F TT | / F TT | ε`

Non-terminals: P, B, DL, C, CL, V, VL, PR, S, L, CO, E, ET, T, TT, F, A

## Parser Modes

- `--ll1`: Only LL(1) — used when LR(1) construction is unnecessary
- `--lr1`: Only LR(1) — 382 states computed, parsing via ACTION/GOTO tables
- `--dual`: Both run; compilation succeeds if both agree on parse outcome. Quad differences are informational (different but equivalent codegen).

## Known Issues

1. **MinGW -O1/-O2 segfault**: `lexer.cpp` and `compiler.cpp` crash with optimization levels above -O0 on MinGW g++. These files must be compiled with `-O0`. See Makefile for per-file flags.
2. **DFA stubs**: Full DFA construction (`dfa/dfa.cpp`, `dfa/nfa.cpp`) is slow. Using `dfa_stubs.cpp` bypasses DFA validation for fast development.
3. **EOF detection**: On MinGW, `ifstream::eof()` may not trigger after `get()` returns EOF. Workaround: detect consecutive ERROR tokens (≥3) as end-of-input.
4. **LL/LR quad differences**: LL(1) and LR(1) use different code generation strategies (e.g., LL1 uses `ADD val` + `ASSIGN`; LR1 puts value directly in `ASSIGN`). Both are semantically equivalent. DUAL mode treats parse success as the validation criterion, not quad identity.

## Test Files

- `test.pl0` — basic: var declarations, assignment, arithmetic, write
- `test_complex.pl0` — constants, if/while, nested blocks
- `test_procedure.pl0` — procedure declaration and call
