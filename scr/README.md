# PL/0 编译器技术文档

## 文档目录

本文档集提供 PL/0 编译器的完整技术参考。

---

## 📚 文档索引

### 🔷 头文件模块 (include/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [pl0_core.md](include/pl0_core.md) | [pl0_core.hpp](include/pl0_core.hpp) | 核心类型定义（Token, Symbol, OpCode等） |
| [pl0_lexer.md](include/pl0_lexer.md) | [pl0_lexer.hpp](include/pl0_lexer.hpp) | 词法分析器类定义 |
| [pl0_symtable.md](include/pl0_symtable.md) | [pl0_symtable.hpp](include/pl0_symtable.hpp) | 符号表类定义 |
| [pl0_codegen.md](include/pl0_codegen.md) | [pl0_codegen.hpp](include/pl0_codegen.hpp) | 代码生成器类定义 |
| [pl0_parser.md](include/pl0_parser.md) | [pl0_parser.hpp](include/pl0_parser.hpp) | 语法分析器类定义 |
| [pl0_compiler.md](include/pl0_compiler.md) | [pl0_compiler.hpp](include/pl0_compiler.hpp) | 主编译器类定义 |

---

### 🔷 源代码模块 (src/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [src/README.md](src/README.md) | [src/lexer.cpp](src/lexer.cpp) | 词法分析器实现 |
| | [src/symtable.cpp](src/symtable.cpp) | 符号表实现 |
| | [src/codegen.cpp](src/codegen.cpp) | 代码生成器实现 |
| | [src/parser.cpp](src/parser.cpp) | 语法分析器实现 |
| | [src/compiler.cpp](src/compiler.cpp) | 主编译器实现 |

---

### 🔷 DFA 模块 (dfa/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [dfa/README.md](dfa/README.md) | [dfa/regex_parser.cpp](dfa/regex_parser.cpp) | 正则表达式解析器 |
| | [dfa/nfa.cpp](dfa/nfa.cpp) | NFA 构造 |
| | [dfa/dfa.cpp](dfa/dfa.cpp) | DFA 构造和最小化 |
| | [dfa/dfa_wrapper.cpp](dfa/dfa_wrapper.cpp) | C 接口包装 |
| | [dfa/regex_access.cpp](dfa/regex_access.cpp) | 正则表达式访问 |
| | [dfa/pl0_regex.h](dfa/pl0_regex.h) | PL/0 正则定义 |

---

### 🔷 可视化工具 (visualizer/)

| 文档 | 文件 | 描述 |
|------|------|------|
| [visualizer/README.md](visualizer/README.md) | [visualizer/lexer_visualizer.cpp](visualizer/lexer_visualizer.cpp) | 词法分析可视化程序 |

---

## 📖 按主题分类

### 🏗️ 架构设计

- [主编译器说明](include/pl0_compiler.md) - 编译器整体架构
- [源代码模块说明](src/README.md) - 模块依赖关系

### 🔍 编译阶段

| 阶段 | 文档 | 关键功能 |
|------|------|----------|
| 词法分析 | [词法分析器](include/pl0_lexer.md) | Token 生成、DFA 验证 |
| 语法分析 | [语法分析器](include/pl0_parser.md) | 递归下降分析 |
| 语义分析 | [符号表](include/pl0_symtable.md) | 作用域管理、符号查找 |
| 代码生成 | [代码生成器](include/pl0_codegen.md) | 四元式生成 |

### ⚙️ 核心组件

| 组件 | 文档 | 数据结构 |
|------|------|----------|
| 类型系统 | [核心类型](include/pl0_core.md) | TokenType, OpCode, SymbolKind |
| 符号管理 | [符号表](include/pl0_symtable.md) | Scope, Symbol |
| 中间代码 | [代码生成](include/pl0_codegen.md) | Quadruple |
| 状态机 | [DFA模块](dfa/README.md) | NFA, DFA, DFAState |

---

## 🎯 快速查找

### 新手入门

1. [README.md](../../README.md) - 项目概述
2. [pl0_compiler.md](include/pl0_compiler.md) - 编译器使用
3. [pl0_parser.md](include/pl0_parser.md) - 语法分析

### 开发参考

1. [pl0_core.md](include/pl0_core.md) - 类型定义
2. [src/README.md](src/README.md) - 实现细节
3. [dfa/README.md](dfa/README.md) - DFA 实现

### 问题排查

| 问题类型 | 相关文档 |
|----------|----------|
| 编译错误 | [src/README.md](src/README.md) - 编译命令 |
| 词法错误 | [pl0_lexer.md](include/pl0_lexer.md) - Token 处理 |
| 语法错误 | [pl0_parser.md](include/pl0_parser.md) - 错误处理 |
| DFA 验证 | [dfa/README.md](dfa/README.md) - 正则验证 |

---

## 📋 文档清单

### 必需文档

- [x] [项目总览](../../README.md)
- [x] [核心类型定义](include/pl0_core.md)
- [x] [词法分析器](include/pl0_lexer.md)
- [x] [符号表](include/pl0_symtable.md)
- [x] [代码生成器](include/pl0_codegen.md)
- [x] [语法分析器](include/pl0_parser.md)
- [x] [主编译器](include/pl0_compiler.md)
- [x] [DFA 模块](dfa/README.md)
- [x] [源代码模块](src/README.md)

### 补充文档

- [x] API 参考
- [x] 算法说明
- [x] 使用示例
- [x] 错误处理
- [x] 性能优化

---

## 🔗 相关链接

### 外部资源

- [PL/0 语言规范](https://en.wikipedia.org/wiki/PL/I)
- [编译原理](https://en.wikipedia.org/wiki/Compiler)
- [有限状态机](https://en.wikipedia.org/wiki/Finite-state_machine)

### 项目文件

```
Pl0/
├── README.md                 # 项目总览
├── scr/
│   ├── include/              # 头文件
│   │   ├── pl0_core.hpp      # 核心类型
│   │   ├── pl0_lexer.hpp     # 词法分析器
│   │   ├── pl0_symtable.hpp  # 符号表
│   │   ├── pl0_codegen.hpp   # 代码生成器
│   │   ├── pl0_parser.hpp    # 语法分析器
│   │   └── pl0_compiler.hpp  # 主编译器
│   ├── src/                  # 源代码
│   │   ├── lexer.cpp
│   │   ├── symtable.cpp
│   │   ├── codegen.cpp
│   │   ├── parser.cpp
│   │   └── compiler.cpp
│   ├── dfa/                  # DFA 模块
│   │   ├── regex_parser.cpp
│   │   ├── nfa.cpp
│   │   ├── dfa.cpp
│   │   ├── dfa_wrapper.cpp
│   │   ├── regex_access.cpp
│   │   └── pl0_regex.h
│   ├── main.cpp              # 主程序
│   └── Makefile_cpp          # 构建文件
└── 文档/
    └── ...                   # 本文档目录
```

---

## 📝 更新日志

### v3.0 (2026-06-06)
- 添加完整模块文档
- 添加 API 参考
- 添加使用示例
- 添加错误处理指南

---

## ❓ 获取帮助

如有问题，请查看：

1. 常见问题 - [pl0_lexer.md](include/pl0_lexer.md#常见问题)
2. 错误处理 - [pl0_parser.md](include/pl0_parser.md#错误处理)
3. API 参考 - 各模块文档的"使用示例"部分

---

## 📄 许可证

本文档及 PL/0 编译器仅供学习和研究使用。