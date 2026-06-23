# PL/0 编译器技术文档

## 文档目录

本文档集提供 PL/0 编译器的完整技术参考，包含词法分析、语法分析（LL(1)和LR(1)）、语义分析、中间代码生成等核心模块。

---

## 📚 文档索引

### 🔷 头文件模块 (include/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [pl0_core.md](include/pl0_core.md) | [pl0_core.hpp](include/pl0_core.hpp) | 核心类型定义（Token, Symbol, OpCode等） |
| [pl0_lexer.md](include/pl0_lexer.md) | [pl0_lexer.hpp](include/pl0_lexer.hpp) | 词法分析器类定义 |
| [pl0_symtable.md](include/pl0_symtable.md) | [pl0_symtable.hpp](include/pl0_symtable.hpp) | 符号表类定义 |
| [pl0_codegen.md](include/pl0_codegen.md) | [pl0_codegen.hpp](include/pl0_codegen.hpp) | 代码生成器类定义 |
| [pl0_parser.md](include/pl0_parser.md) | [pl0_parser.hpp](include/pl0_parser.hpp) | LL(1)语法分析器类定义 |
| [pl0_lr1_parser.md](include/pl0_lr1_parser.hpp) | [pl0_lr1_parser.hpp](include/pl0_lr1_parser.hpp) | LR(1)语法分析器类定义 |
| [pl0_compiler.md](include/pl0_compiler.md) | [pl0_compiler.hpp](include/pl0_compiler.hpp) | 主编译器类定义 |
| [pl0_ast.hpp](include/pl0_ast.hpp) | [pl0_ast.hpp](include/pl0_ast.hpp) | AST节点类型定义 |
| [pl0_visualizer.hpp](include/pl0_visualizer.hpp) | [pl0_visualizer.hpp](include/pl0_visualizer.hpp) | 可视化工具类定义 |
| [pl0_grammar_normalizer.hpp](include/pl0_grammar_normalizer.hpp) | [pl0_grammar_normalizer.hpp](include/pl0_grammar_normalizer.hpp) | 文法规范化器类定义 |
| [pl0_error_suggestor.hpp](include/pl0_error_suggestor.hpp) | [pl0_error_suggestor.hpp](include/pl0_error_suggestor.hpp) | 错误修复建议器类定义 |

---

### 🔷 源代码模块 (src/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [src/README.md](src/README.md) | [src/lexer.cpp](src/lexer.cpp) | 词法分析器实现 |
| | [src/symtable.cpp](src/symtable.cpp) | 符号表实现 |
| | [src/codegen.cpp](src/codegen.cpp) | 代码生成器实现 |
| | [src/parser.cpp](src/parser.cpp) | LL(1)递归下降语法分析器实现 |
| | [src/compiler.cpp](src/compiler.cpp) | 主编译器实现 |
| | [src/ast_builder.cpp](src/ast_builder.cpp) | AST构建器实现 |
| | [src/visualizer.cpp](src/visualizer.cpp) | 可视化工具实现 |
| | [src/lr1_grammar.cpp](src/lr1_grammar.cpp) | LR(1)文法定义与FIRST/FOLLOW集计算 |
| | [src/lr1_items.cpp](src/lr1_items.cpp) | LR(1)项目闭包与规范项目集族构造 |
| | [src/lr1_parser_driver.cpp](src/lr1_parser_driver.cpp) | LR(1)移进-归约解析器实现 |
| | [src/lr1_diagnostics.cpp](src/lr1_diagnostics.cpp) | LR(1)诊断与双分析器验证 |
| | [src/grammar_normalizer.cpp](src/grammar_normalizer.cpp) | 文法规范化（左因子提取、左递归消除） |
| | [src/error_suggestor.cpp](src/error_suggestor.cpp) | 错误修复建议器实现 |

---

### 🔷 DFA 模块 (dfa/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [dfa/README.md](dfa/README.md) | [dfa/regex_parser.cpp](dfa/regex_parser.cpp) | 正则表达式解析器 |
| | [dfa/nfa.cpp](dfa/nfa.cpp) | NFA构造（Thompson构造法） |
| | [dfa/dfa.cpp](dfa/dfa.cpp) | DFA构造（子集构造法）和最小化（Hopcroft算法） |
| | [dfa/dfa_wrapper.cpp](dfa/dfa_wrapper.cpp) | C接口包装 |
| | [dfa/regex_access.cpp](dfa/regex_access.cpp) | 正则表达式访问接口 |
| | [dfa/pl0_regex.h](dfa/pl0_regex.h) | PL/0正则定义 |
| | [dfa/dfa_stubs.cpp](dfa/dfa_stubs.cpp) | DFA桩函数（调试用） |

---

## 📖 按主题分类

### 🏗️ 架构设计

- [主编译器说明](include/pl0_compiler.md) - 编译器整体架构
- [源代码模块说明](src/README.md) - 模块依赖关系
- [双分析器验证](#双分析器验证) - LL(1)与LR(1)对比验证机制

### 🔍 编译阶段

| 阶段 | 文档 | 关键功能 |
|------|------|----------|
| 词法分析 | [词法分析器](include/pl0_lexer.md) | Token生成、DFA验证 |
| 语法分析 | [LL(1)分析器](include/pl0_parser.md) | 递归下降分析、L-翻译模式 |
| 语法分析 | [LR(1)分析器](include/pl0_lr1_parser.hpp) | 移进-归约分析、S-翻译模式 |
| 语义分析 | [符号表](include/pl0_symtable.md) | 作用域管理、符号查找 |
| 代码生成 | [代码生成器](include/pl0_codegen.md) | 四元式生成 |

### ⚙️ 核心组件

| 组件 | 文档 | 数据结构 |
|------|------|----------|
| 类型系统 | [核心类型](include/pl0_core.md) | TokenType, OpCode, SymbolKind |
| 符号管理 | [符号表](include/pl0_symtable.md) | Scope, Symbol |
| 中间代码 | [代码生成](include/pl0_codegen.md) | Quadruple |
| 状态机 | [DFA模块](dfa/README.md) | NFA, DFA, DFAState |
| 语法树 | [AST构建器](src/ast_builder.cpp) | ASTNode, ASTBuilder |
| 文法规范化 | [文法规范化器](src/grammar_normalizer.cpp) | NormProduction |

### ✨ 扩展功能

| 功能 | 文件 | 描述 |
|------|------|------|
| AST可视化 | [visualizer.cpp](src/visualizer.cpp) | 生成Graphviz DOT格式语法树 |
| NFA/DFA可视化 | [visualizer.cpp](src/visualizer.cpp) | 生成有限状态机图 |
| 文法规范化 | [grammar_normalizer.cpp](src/grammar_normalizer.cpp) | 左因子提取、左递归消除 |
| 错误建议 | [error_suggestor.cpp](src/error_suggestor.cpp) | 基于模式的错误诊断与修复建议 |

---

## 🎯 快速查找

### 新手入门

1. [README.md](../../README.md) - 项目概述
2. [pl0_compiler.md](include/pl0_compiler.md) - 编译器使用
3. [pl0_parser.md](include/pl0_parser.md) - LL(1)语法分析
4. [pl0_lr1_parser.hpp](include/pl0_lr1_parser.hpp) - LR(1)语法分析

### 开发参考

1. [pl0_core.md](include/pl0_core.md) - 类型定义
2. [src/README.md](src/README.md) - 实现细节
3. [dfa/README.md](dfa/README.md) - DFA实现
4. [lr1_grammar.cpp](src/lr1_grammar.cpp) - LR(1)文法定义

### 问题排查

| 问题类型 | 相关文档 |
|----------|----------|
| 编译错误 | [src/README.md](src/README.md) - 编译命令 |
| 词法错误 | [pl0_lexer.md](include/pl0_lexer.md) - Token处理 |
| 语法错误 | [pl0_parser.md](include/pl0_parser.md) - 错误处理 |
| DFA验证 | [dfa/README.md](dfa/README.md) - 正则验证 |
| 错误建议 | [error_suggestor.cpp](src/error_suggestor.cpp) - 修复建议 |

---

## � 双分析器验证

编译器支持同时使用LL(1)和LR(1)两种语法分析方法进行交叉验证：

### LL(1) 分析器特点
- **方法**：自顶向下递归下降
- **翻译模式**：L-翻译模式（边分析边执行语义动作）
- **优势**：实现简单、易于调试
- **适用**：LL(1)文法

### LR(1) 分析器特点
- **方法**：自底向上移进-归约
- **翻译模式**：S-翻译模式（归约时执行语义动作）
- **优势**：功能更强、能处理更多文法
- **适用**：广泛的上下文无关文法

### 验证流程
```
源代码 → 词法分析 → Token序列
                    ↓
        ┌─────────────────────────┐
        ↓                         ↓
     LL(1)解析器             LR(1)解析器
        ↓                         ↓
     四元式输出               四元式输出
        ↓                         ↓
        └────────→ 对比验证 ←──────┘
                     ↓
              结果一致性报告
```

---

## �📋 文档清单

### 必需文档

- [x] [项目总览](../../README.md)
- [x] [核心类型定义](include/pl0_core.md)
- [x] [词法分析器](include/pl0_lexer.md)
- [x] [符号表](include/pl0_symtable.md)
- [x] [代码生成器](include/pl0_codegen.md)
- [x] [LL(1)语法分析器](include/pl0_parser.md)
- [x] [LR(1)语法分析器](include/pl0_lr1_parser.hpp)
- [x] [主编译器](include/pl0_compiler.md)
- [x] [DFA模块](dfa/README.md)
- [x] [源代码模块](src/README.md)

### 补充文档

- [x] API参考
- [x] 算法说明
- [x] 使用示例
- [x] 错误处理
- [x] 性能优化
- [x] AST可视化
- [x] 文法规范化

---

## 🔗 相关链接

### 外部资源

- [PL/0 语言规范](https://en.wikipedia.org/wiki/PL/0)
- [编译原理](https://en.wikipedia.org/wiki/Compiler)
- [有限状态机](https://en.wikipedia.org/wiki/Finite-state_machine)
- [LR解析器](https://en.wikipedia.org/wiki/LR_parser)
- [Graphviz](https://graphviz.org/)

### 项目文件

```
Pl0/
├── README.md                    # 项目总览
├── scr/
│   ├── include/                 # 头文件
│   │   ├── pl0_core.hpp         # 核心类型（Token, OpCode等）
│   │   ├── pl0_lexer.hpp        # 词法分析器
│   │   ├── pl0_symtable.hpp     # 符号表
│   │   ├── pl0_codegen.hpp      # 代码生成器
│   │   ├── pl0_parser.hpp       # LL(1)语法分析器
│   │   ├── pl0_lr1_parser.hpp   # LR(1)语法分析器
│   │   ├── pl0_compiler.hpp     # 主编译器
│   │   ├── pl0_ast.hpp          # AST定义
│   │   ├── pl0_visualizer.hpp   # 可视化工具
│   │   ├── pl0_grammar_normalizer.hpp  # 文法规范化器
│   │   └── pl0_error_suggestor.hpp     # 错误建议器
│   ├── src/                     # 源代码
│   │   ├── lexer.cpp            # 词法分析器实现
│   │   ├── symtable.cpp         # 符号表实现
│   │   ├── codegen.cpp          # 代码生成器实现
│   │   ├── parser.cpp           # LL(1)解析器实现
│   │   ├── compiler.cpp         # 主编译器实现
│   │   ├── ast_builder.cpp      # AST构建器
│   │   ├── visualizer.cpp       # 可视化工具
│   │   ├── lr1_grammar.cpp      # LR(1)文法定义
│   │   ├── lr1_items.cpp        # LR(1)项目集构造
│   │   ├── lr1_parser_driver.cpp # LR(1)解析驱动器
│   │   ├── lr1_diagnostics.cpp  # LR(1)诊断与验证
│   │   ├── grammar_normalizer.cpp # 文法规范化
│   │   └── error_suggestor.cpp  # 错误建议器
│   ├── dfa/                     # DFA模块
│   │   ├── regex_parser.cpp     # 正则表达式解析器
│   │   ├── nfa.cpp              # NFA构造
│   │   ├── dfa.cpp              # DFA构造与最小化
│   │   ├── dfa_wrapper.cpp      # C接口包装
│   │   ├── regex_access.cpp     # 正则访问接口
│   │   ├── pl0_regex.h          # PL/0正则定义
│   │   └── dfa_stubs.cpp        # DFA桩函数
│   ├── tests/                   # 测试模块
│   │   └── run_tests.cpp        # 测试运行器
│   ├── main.cpp                 # 主程序入口
│   └── Makefile_cpp             # 构建文件
└── 文档/
    └── ...                      # 技术文档目录
```

---

## 📝 更新日志

### v4.0 (2026-06-22)
- 添加LR(1)语法分析器实现
- 添加双分析器验证机制（LL(1)+LR(1)交叉验证）
- 添加AST构建器和可视化工具
- 添加文法规范化器（左因子提取、左递归消除）
- 添加错误修复建议器
- 完善所有模块的中文注释
- 更新技术文档

### v3.0 (2026-06-06)
- 添加完整模块文档
- 添加API参考
- 添加使用示例
- 添加错误处理指南

---

## ❓ 获取帮助

如有问题，请查看：

1. 常见问题 - [pl0_lexer.md](include/pl0_lexer.md#常见问题)
2. 错误处理 - [pl0_parser.md](include/pl0_parser.md#错误处理)
3. API参考 - 各模块文档的"使用示例"部分
4. 错误建议 - [error_suggestor.cpp](src/error_suggestor.cpp)

---

## 📄 许可证

本文档及PL/0编译器仅供学习和研究使用。
