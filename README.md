# PL/0 编译器 — 双向验证语法分析系统

## 项目简介

PL/0 编译器基于 C++17 实现，支持完整的词法分析、语法分析、语义分析和中间代码生成。核心特性是**双向验证语法分析**：同时运行 LL(1) 递归下降分析和 LR(1) 移进归约分析，交叉验证确保语法正确性。

### 版本信息
- 版本：v4.0 (Dual Parser Edition)
- 编程语言：C++17
- 编译器：GCC (g++) / MinGW

---

## 项目结构

```
Pl0/
├── scr/
│   ├── include/                     # 头文件
│   │   ├── pl0_core.hpp            # 核心类型（Token、Quadruple、Symbol）
│   │   ├── pl0_lexer.hpp           # 词法分析器
│   │   ├── pl0_parser.hpp          # LL(1) 递归下降语法分析器
│   │   ├── pl0_lr1_parser.hpp      # LR(1) 移进归约语法分析器
│   │   ├── pl0_symtable.hpp        # 符号表
│   │   ├── pl0_codegen.hpp         # 代码生成器（四元式）
│   │   └── pl0_compiler.hpp        # 主编译器（协调双解析器）
│   ├── src/                         # 实现文件
│   │   ├── lexer.cpp               # 词法分析
│   │   ├── parser.cpp              # LL(1) 解析器（递归下降）
│   │   ├── lr1_grammar.cpp         # LR(1) 文法定义 + FIRST/FOLLOW 集
│   │   ├── lr1_items.cpp           # LR(1) 闭包 + 规范族 + 分析表
│   │   ├── lr1_parser_driver.cpp   # LR(1) 移进归约引擎 + 语义动作
│   │   ├── lr1_diagnostics.cpp     # LR(1) 诊断输出 + 双重验证
│   │   ├── symtable.cpp            # 符号表实现
│   │   ├── codegen.cpp             # 代码生成器实现
│   │   └── compiler.cpp            # 编译器协调器
│   ├── dfa/                         # DFA 词法验证模块
│   │   ├── dfa.cpp / nfa.cpp       # DFA/NFA 构造
│   │   ├── regex_parser.cpp        # 正则解析
│   │   ├── dfa_wrapper.cpp         # C 接口包装
│   │   ├── dfa_stubs.cpp           # DFA 函数桩（快速模式）
│   │   └── regex_access.cpp        # 正则表达式访问
│   ├── pl0_regex/
│   │   └── pl0_regex.h             # PL/0 正则表达式定义
│   ├── main.cpp                     # 主程序入口
│   ├── Makefile                     # Makefile
│   ├── test.pl0                     # 基础测试用例
│   ├── test_complex.pl0             # 复杂测试用例
│   └── test_procedure.pl0           # 过程测试用例
└── README.md
```

---

## 快速开始

### 编译

```bash
cd scr
make
```

手动编译：

```bash
cd scr
g++ -std=c++17 -O2 -Wall -I. -c src/*.cpp dfa/dfa_stubs.cpp dfa/regex_access.cpp
g++ -std=c++17 -O2 -Wall -I. *.o -o build/pl0_compiler
```

### 运行

```bash
# 双重验证（默认）
./build/pl0_compiler test.pl0 --dual

# 仅 LL(1) 递归下降
./build/pl0_compiler test.pl0 --ll1

# 仅 LR(1) 移进归约
./build/pl0_compiler test.pl0 --lr1
```

---

## 解析模式

| 选项 | 模式 | 描述 |
|------|------|------|
| `--ll1` | LL(1) 单独 | 仅使用递归下降分析 |
| `--lr1` | LR(1) 单独 | 仅使用移进归约分析（382 个 LR(1) 状态） |
| `--dual` | 双重验证 | 同时运行两种分析器，对比解析结果 |
| `-v` | 详细输出 | 显示分析过程详情 |
| `-h` | 帮助 | 显示使用说明 |

---

## PL/0 语法（EBNF）

```
程序     → 分程序 '.'
分程序   → { 声明 } 语句
声明     → 常量声明 | 变量声明 | 过程声明
常量声明 → 'const' 标识符 '=' 数字 { ',' 标识符 '=' 数字 } ';'
变量声明 → 'var' 标识符 { ',' 标识符 } ';'
过程声明 → 'procedure' 标识符 ';' 分程序 ';'
语句     → 标识符 ':=' 表达式
         | 'begin' 语句 { ';' 语句 } 'end'
         | 'if' 条件 'then' 语句
         | 'while' 条件 'do' 语句
         | 'call' 标识符
         | 'read' '(' 标识符 ')'
         | 'write' '(' 表达式 { ',' 表达式 } ')'
         | ε
条件     → 'odd' 表达式 | 表达式 关系运算符 表达式
表达式   → ['+' | '-'] 项 { '+' 项 | '-' 项 }
项       → 因子 { '*' 因子 | '/' 因子 }
因子     → 标识符 | 数字 | '(' 表达式 ')'
```

---

## LR(1) 分析器架构

LR(1) 实现采用四模块多文件架构：

| 模块 | 文件 | 职责 |
|------|------|------|
| **文法** | `lr1_grammar.cpp` | 46 条产生式定义、FIRST/FOLLOW 集计算 |
| **项目集** | `lr1_items.cpp` | LR(1) 闭包运算、规范族构造（382 状态）、ACTION/GOTO 表 |
| **驱动** | `lr1_parser_driver.cpp` | 移进归约引擎、Token 映射、语义动作（四元式生成） |
| **诊断** | `lr1_diagnostics.cpp` | 符号名称转换、分析表输出、LL/LR 双重验证对比 |

---

## 限制

- 标识符最大长度：8 字符
- 数字最大长度：8 位
- 不支持 `else` 分支
- PL/0 是教学语言，非完整 Pascal

---

## 许可证

仅供学习和研究使用。
