# PL/0 Compiler Test Suite

## 目录结构

```
tests/
├── test_framework.hpp         # 轻量级测试框架（断言宏、结果收集、彩色输出）
├── run_tests.cpp              # 测试主入口，注册并运行所有测试
├── Makefile                   # 测试构建与运行
├── README.md                  # 本文档
│
├── unit/                      # 单元测试
│   ├── lexer/                 # 词法分析测试 (7个文件)
│   │   ├── keywords.pl0       # 关键字识别
│   │   ├── identifiers.pl0    # 标识符识别
│   │   ├── numbers.pl0        # 数字识别
│   │   ├── operators.pl0      # 运算符识别
│   │   ├── delimiters.pl0     # 分隔符识别
│   │   ├── comments_whitespace.pl0  # 注释和空白处理
│   │   └── error_tokens.pl0   # 错误Token处理
│   │
│   ├── parser/                # 语法分析测试 (16个文件)
│   │   ├── const_decl.pl0     # 常量声明
│   │   ├── multi_const.pl0    # 多常量声明
│   │   ├── var_decl.pl0       # 变量声明
│   │   ├── proc_decl.pl0      # 过程声明
│   │   ├── nested_proc.pl0    # 嵌套过程声明
│   │   ├── assign_stmt.pl0    # 赋值语句
│   │   ├── compound_stmt.pl0  # 复合语句
│   │   ├── if_stmt.pl0        # if语句
│   │   ├── while_stmt.pl0     # while语句
│   │   ├── call_stmt.pl0      # 过程调用语句
│   │   ├── read_write_stmt.pl0# read/write语句
│   │   ├── simple_expr.pl0    # 简单表达式
│   │   ├── complex_expr.pl0   # 复杂表达式
│   │   ├── paren_expr.pl0     # 括号表达式
│   │   ├── neg_expr.pl0       # 负号表达式
│   │   └── conditions.pl0     # odd条件及关系条件
│   │
│   ├── lr1_parser/            # LR(1)解析器测试 (4个文件)
│   │   ├── lr1_shift_reduce.pl0    # 移进-归约测试
│   │   ├── lr1_left_recursion.pl0  # 左递归等价测试
│   │   ├── lr1_382_states.pl0      # 382状态覆盖测试
│   │   └── lr1_goto_action.pl0     # GOTO/ACTION表测试
│   │
│   ├── symtable/              # 符号表测试 (3个文件)
│   │   ├── symbol_insert.pl0  # 符号插入
│   │   ├── scope_lookup.pl0   # 跨作用域查找
│   │   └── scope_shadow.pl0   # 作用域遮蔽
│   │
│   ├── codegen/               # 代码生成测试 (5个文件)
│   │   ├── assign_codegen.pl0       # 赋值四元式
│   │   ├── if_while_codegen.pl0     # 条件/循环四元式
│   │   ├── call_codegen.pl0         # 过程调用四元式
│   │   ├── io_codegen.pl0           # IO四元式
│   │   └── complex_arith_codegen.pl0# 复杂算术四元式
│   │
│   └── astbuilder/            # AST构建器测试
│       └── ast_nodes.pl0      # AST节点构建
│
├── integration/               # 集成测试
│   ├── compilation/           # 编译流程测试 (2个文件)
│   │   ├── full_compile.pl0   # 完整程序编译
│   │   └── proc_compile.pl0   # 多过程程序编译
│   │
│   └── dual_parser/           # 双解析器验证测试 (2个文件)
│       ├── dual_simple.pl0    # 简单程序双解析器一致
│       └── dual_complex.pl0   # 复杂程序双解析器一致
│
├── system/                    # 系统测试
│   ├── functional/            # 功能测试 (5个文件)
│   │   ├── complete_program.pl0# 完整计算程序
│   │   ├── fibonacci.pl0      # 斐波那契数列
│   │   ├── factorial.pl0      # 阶乘计算
│   │   ├── odd_even.pl0       # odd奇偶判断
│   │   └── gcd.pl0            # GCD最大公约数
│   │
│   ├── boundary/              # 边界测试 (6个文件)
│   │   ├── empty_program.pl0  # 空程序（仅点号）
│   │   ├── empty_compound.pl0 # 空复合语句
│   │   ├── max_identifier.pl0 # 最大标识符长度(8字符)
│   │   ├── max_number.pl0     # 最大数字(8位)
│   │   ├── deep_nesting.pl0   # 深度嵌套过程
│   │   └── many_vars.pl0      # 大量变量声明
│   │
│   └── error_handling/        # 错误处理测试 (7个文件)
│       ├── lexer_error.pl0    # 词法错误
│       ├── syntax_error.pl0   # 语法错误
│       ├── missing_semicolon.pl0# 缺少分号
│       ├── undefined_var.pl0  # 未定义变量
│       ├── duplicate_def.pl0  # 重复定义
│       ├── wrong_assign.pl0   # 常量赋值错误
│       └── dual_error.pl0     # 双解析器均报错
│
└── performance/               # 性能测试
    └── compile_speed/         # 编译速度测试 (1个文件)
        └── medium_prog.pl0    # 中型程序编译性能
```

## 测试统计

| 测试类别 | 测试数量 | 说明 |
|---------|---------|------|
| 单元测试 - 词法分析 | 6 | 关键字、标识符、数字、运算符、分隔符、注释空白 |
| 单元测试 - 语法分析(声明) | 5 | 常量、多常量、变量、过程、嵌套过程 |
| 单元测试 - 语法分析(语句) | 6 | 赋值、复合、if、while、call、read/write |
| 单元测试 - 语法分析(表达式) | 4 | 简单、复杂、括号、负号 |
| 单元测试 - 语法分析(条件) | 1 | odd及全部关系条件 |
| 单元测试 - 符号表 | 3 | 插入、查找、遮蔽 |
| 单元测试 - 代码生成 | 5 | 赋值、条件循环、调用、IO、复杂算术 |
| 单元测试 - LR(1)解析器 | 4 | 移进归约、左递归、382状态、GOTO/ACTION |
| 单元测试 - AST构建 | 1 | AST节点构建 |
| **单元测试小计** | **35** | |
| 集成测试 - 编译流程 | 2 | 完整程序、多过程 |
| 集成测试 - 双解析器验证 | 2 | 简单双解析、复杂双解析 |
| 系统测试 - 功能测试 | 5 | 完整计算、Fibonacci、Factorial、Odd、GCD |
| 系统测试 - 边界测试 | 6 | 空程序、空语句、最大ID、最大数字、深度嵌套、大量变量 |
| 系统测试 - 错误处理 | 7 | 词法错、语法错、缺分号、未定义、重复定义、类型错、双重报错 |
| 性能测试 | 1 | 中型程序编译速度 |
| **总计** | **58** | |

## 构建与运行

### 构建测试

```bash
cd scr/tests
make
```

### 运行测试

```bash
cd scr/tests
make run
```

### 清理

```bash
make clean       # 清理构建产物
make distclean   # 清理构建产物和测试输出文件
```

### 手动运行

```bash
cd scr
tests/build/pl0_test_runner.exe
```

## 测试设计

### 测试策略

- **单元测试**: 验证每个编译器模块独立正确性
  - Lexer: 使用包含特定token类型的源文件，验证词法分析正确性
  - Parser: 使用包含特定语法结构的源文件，验证解析成功
  - LR1 Parser: 验证LR(1)移进-归约解析器，覆盖382个状态
  - SymbolTable: 验证符号插入、查找、作用域管理
  - CodeGenerator: 验证四元式中间代码生成
  - ASTBuilder: 验证抽象语法树构建

- **集成测试**: 验证模块间协作
  - 编译流程: 完整编译链路（词法→语法→语义→代码生成）
  - 双解析器: LL(1) vs LR(1) 结果一致性验证

- **系统测试**: 端到端功能验证
  - 功能测试: 实际可运行的程序（斐波那契、阶乘、GCD等）
  - 边界测试: 极端输入条件
  - 错误处理: 各类错误检测和建议

- **性能测试**: 编译速度基准测试

### 测试断言

每个测试用例包含以下属性：
- `suite`: 测试套件名称
- `name`: 测试用例名称
- `file`: 测试源文件路径
- `expectSuccess`: 是否预期编译成功
- `expectDual`: 是否使用双解析器模式验证

### 通过标准

- 成功测试: `Compiler::compile()` 返回 `true` 且双解析器均成功
- 错误测试: `Compiler::compile()` 返回 `false`（正确检测到预期错误）

## 与主项目的集成

测试套件使用与主项目相同的编译器源文件，通过链接编译器的所有模块来执行真实编译。
测试运行在 `scr/` 工作目录下执行，以便正确解析测试文件的相对路径。

## 扩展指南

### 添加新测试

1. 在对应目录创建 `.pl0` 测试源文件
2. 在 `run_tests.cpp` 中添加 `REGISTER_TEST` 调用
3. 重新构建并运行

### 测试文件命名规范

- 使用小写字母和下划线
- 文件名描述测试内容
- 成功测试：正常PL/0语法
- 错误测试：包含预期错误的语法
