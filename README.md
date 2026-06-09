# PL/0 编译器软件说明书

## 项目简介

PL/0 编译器是一个基于现代 C++17 标准实现的 PL/0 语言编译器，支持完整的词法分析、语法分析、语义分析和中间代码生成功能。本项目采用模块化多文件架构，代码结构清晰，易于维护和扩展。

### 版本信息
- 版本：v3.0 (C++ Edition)
- 开发日期：2026-06-06
- 编程语言：C++17
- 编译器：GCC (g++)

---

## 功能特性

### 1. 词法分析 (Lexical Analysis)
- 支持 PL/0 语言的所有关键字、标识符、数字、运算符和分隔符
- 基于 DFA（确定性有限自动机）的正则表达式验证
- 自动识别并跳过单行注释 (`//`) 和多行注释 (`/* */`)
- 支持双字符运算符（如 `:=`, `<=`, `>=`）
- 提供详细的词法分析报告，包括：
  - Token 列表
  - Token 统计信息
  - 单词分类表
  - 状态转换图
  - 词法识别流程图

### 2. 语法分析 (Syntax Analysis)
- 采用递归下降分析法
- 支持 PL/0 语言的所有语法结构：
  - 常量声明 (`const`)
  - 变量声明 (`var`)
  - 过程声明 (`procedure`)
  - 赋值语句 (`:=`)
  - 条件语句 (`if ... then`)
  - 循环语句 (`while ... do`)
  - 过程调用 (`call`)
  - 输入输出语句 (`read`, `write`)
  - 复合语句 (`begin ... end`)
- 提供详细的语法错误信息和行号定位

### 3. 语义分析 (Semantic Analysis)
- 符号表管理
- 作用域支持（全局和局部作用域）
- 符号类型检查（常量、变量、过程）
- 变量重复声明检测
- 未定义标识符检测

### 4. 代码生成 (Code Generation)
- 生成四元式中间代码
- 支持所有 PL/0 操作码：
  - 算术运算：`+`, `-`, `*`, `/`
  - 比较运算：`=`, `#`, `<`, `<=`, `>`, `>=`, `odd`
  - 赋值：`:=`
  - 跳转：`jump`, `jz`, `jnz`
  - 过程：`call`, `ret`
  - 系统：`syss`, `sysc`
  - I/O：`read`, `write`

---

## 项目结构

```
Pl0/
├── scr/                          # 源代码目录
│   ├── include/                  # 头文件目录
│   │   ├── pl0_core.hpp         # 核心类型定义
│   │   ├── pl0_lexer.hpp        # 词法分析器
│   │   ├── pl0_symtable.hpp     # 符号表
│   │   ├── pl0_codegen.hpp      # 代码生成器
│   │   ├── pl0_parser.hpp       # 语法分析器
│   │   └── pl0_compiler.hpp     # 主编译器
│   ├── src/                      # 实现文件目录
│   │   ├── lexer.cpp            # 词法分析器实现
│   │   ├── symtable.cpp         # 符号表实现
│   │   ├── codegen.cpp          # 代码生成器实现
│   │   ├── parser.cpp           # 语法分析器实现
│   │   └── compiler.cpp         # 主编译器实现
│   ├── dfa/                      # DFA 模块目录
│   │   ├── regex_parser.cpp     # 正则表达式解析器
│   │   ├── nfa.cpp              # NFA 构造
│   │   ├── dfa.cpp              # DFA 构造和最小化
│   │   ├── dfa_wrapper.cpp      # C 接口包装
│   │   ├── regex_access.cpp     # 正则表达式访问
│   │   ├── dfa_wrapper.h        # DFA 包装头文件
│   │   ├── regex_access.h       # 正则表达式访问头文件
│   │   └── pl0_regex.h          # PL/0 正则表达式定义
│   ├── main.cpp                  # 主程序入口
│   ├── Makefile_cpp             # C++ Makefile
│   ├── test.pl0                  # 基础测试文件
│   ├── test_complex.pl0          # 复杂测试文件
│   └── test_procedure.pl0        # 过程测试文件
└── README.md                     # 本说明文件
```

---

## 编译说明

### 环境要求
- GCC 编译器 (g++) 支持 C++17 标准
- Windows PowerShell 或 Linux/macOS 终端

### Windows 编译步骤

#### 方法一：手动编译（推荐）
```powershell
cd c:\Users\j1978\Desktop\Pl0\scr

# 编译 DFA 模块
g++ -Wall -g -std=c++17 -I. -c dfa/regex_parser.cpp -o dfa/regex_parser.o
g++ -Wall -g -std=c++17 -I. -c dfa/nfa.cpp -o dfa/nfa.o
g++ -Wall -g -std=c++17 -I. -c dfa/dfa.cpp -o dfa/dfa.o
g++ -Wall -g -std=c++17 -I. -c dfa/dfa_wrapper.cpp -o dfa/dfa_wrapper.o
g++ -Wall -g -std=c++17 -I. -c dfa/regex_access.cpp -o dfa/regex_access.o

# 编译编译器模块
g++ -Wall -g -std=c++17 -I. -c src/lexer.cpp -o src/lexer.o
g++ -Wall -g -std=c++17 -I. -c src/symtable.cpp -o src/symtable.o
g++ -Wall -g -std=c++17 -I. -c src/codegen.cpp -o src/codegen.o
g++ -Wall -g -std=c++17 -I. -c src/parser.cpp -o src/parser.o
g++ -Wall -g -std=c++17 -I. -c src/compiler.cpp -o src/compiler.o

# 编译主程序
g++ -Wall -g -std=c++17 -I. -c main.cpp -o main.o

# 链接生成可执行文件
g++ -Wall -g -std=c++17 -I. -o compiler main.o src/lexer.o src/symtable.o src/codegen.o src/parser.o src/compiler.o dfa/regex_parser.o dfa/nfa.o dfa/dfa.o dfa/dfa_wrapper.o dfa/regex_access.o
```

#### 方法二：使用 Makefile（Linux/macOS）
```bash
cd c:\Users\j1978\Desktop\Pl0\scr
make -f Makefile_cpp clean
make -f Makefile_cpp
```

---

## 使用方法

### 基本用法

```bash
./compiler <源文件.pl0> [选项]
```

### 命令行选项

| 选项 | 说明 |
|------|------|
| `-v, --verbose` | 启用详细输出模式 |
| `-h, --help` | 显示帮助信息 |

### 使用示例

#### 1. 基础编译
```bash
./compiler test.pl0
```

#### 2. 详细输出模式
```bash
./compiler test.pl0 -v
```

#### 3. 显示帮助
```bash
./compiler -h
```

---

## PL/0 语言语法

### 程序结构
```
<程序> ::= <分程序>.

<分程序> ::= [<常量说明部分>] [<变量说明部分>] [<过程说明部分>] <语句>
```

### 常量说明
```
<常量说明部分> ::= const <常量定义> {, <常量定义>};
<常量定义> ::= <标识符> = <无符号整数>
```

### 变量说明
```
<变量说明部分> ::= var <标识符> {, <标识符>};
```

### 过程说明
```
<过程说明部分> ::= <过程首部> <分程序>; {<过程说明部分>}
<过程首部> ::= procedure <标识符>;
```

### 语句
```
<语句> ::= <赋值语句> | <条件语句> | <循环语句> | <过程调用语句> |
          <读语句> | <写语句> | <复合语句> | <空>

<赋值语句> ::= <标识符> := <表达式>

<条件语句> ::= if <条件> then <语句>

<循环语句> ::= while <条件> do <语句>

<过程调用语句> ::= call <标识符>

<读语句> ::= read (<标识符>)

<写语句> ::= write (<表达式> {, <表达式>})

<复合语句> ::= begin <语句> {; <语句>} end
```

### 表达式
```
<表达式> ::= [+|-] <项> { (+|-) <项> }
<项> ::= <因子> { (*|/) <因子> }
<因子> ::= <标识符> | <无符号整数> | (<表达式>)
```

### 条件
```
<条件> ::= odd <表达式> | <表达式> <关系运算符> <表达式>
<关系运算符> ::= = | # | < | <= | > | >=
```

---

## 示例程序

### 示例 1：基础赋值和输出
```pl0
var x, y;
begin
    x := 10;
    y := x + 5;
    write(y)
end.
```

### 示例 2：常量和循环
```pl0
var x, y, z;
const max = 100;
begin
    x := 10;
    y := 20;
    z := x + y;
    if z > max then
        write(z);
    while x < y do
    begin
        x := x + 1;
        write(x)
    end
end.
```

### 示例 3：过程调用
```pl0
var x, y;
procedure add;
    var z;
    begin
        z := x + y;
        write(z)
    end;
begin
    x := 5;
    y := 3;
    call add
end.
```

---

## 输出文件说明

编译成功后，会自动生成以下文件：

### 1. 词法分析报告 (`<源文件名>_lex.txt`)
- Token 列表
- Token 统计信息
- 单词分类表
- 状态转换图
- 词法识别流程图

### 2. 四元式报告 (`<源文件名>_quad.txt`)
- 生成的所有四元式中间代码

### 3. 缓存文件 (`<源文件名>_cache.txt`)
- 编译器缓存信息
- 符号表内容
- 四元式内容

---

## 技术细节

### DFA 实现
- 使用 Thompson 构造法将正则表达式转换为 NFA
- 使用子集构造法将 NFA 转换为 DFA
- 使用 Hopcroft 算法进行 DFA 最小化

### 正则表达式定义
所有 PL/0 语言的词法规则定义在 `dfa/pl0_regex.h` 文件中：
- 标识符：`([a-zA-Z][a-zA-Z0-9]*)`
- 无符号整数：`([0-9]+)`
- 关键字：`(const|var|procedure|begin|end|if|then|while|do|call|odd|read|write)`

### 模块化设计
项目采用面向对象设计，主要类包括：
- `Lexer`: 词法分析器
- `Parser`: 语法分析器
- `SymbolTable`: 符号表管理
- `CodeGenerator`: 代码生成器
- `Compiler`: 主编译器协调器

---

## 注意事项

1. **语法限制**
   - PL/0 不支持 `else` 分支
   - 标识符长度限制为 8 个字符
   - 数字长度限制为 8 位

2. **错误处理**
   - 编译器会在遇到第一个错误时停止
   - 错误信息包含行号和详细描述

3. **文件编码**
   - 源文件应使用 UTF-8 或 ASCII 编码
   - 避免使用特殊字符

4. **性能考虑**
   - 大文件编译可能需要较长时间
   - DFA 初始化会消耗一定内存

---

## 常见问题

### Q: 编译时出现 "undefined reference" 错误
A: 确保所有 DFA 模块的目标文件都已编译并正确链接。

### Q: 运行时出现 "Cannot open file" 错误
A: 检查源文件路径是否正确，文件是否存在。

### Q: 生成的四元式看不懂
A: 四元式格式为 `(索引)(操作码,参数1,参数2,结果)`，参考"代码生成"部分的说明。

### Q: 如何添加新的关键字？
A: 修改 `dfa/pl0_regex.h` 中的 `REG_KEYWORD` 正则表达式，并在 `src/lexer.cpp` 中的 `keywords_` 表中添加。

---

## 联系方式

如有问题或建议，请联系项目维护者。

---

## 许可证

本项目仅供学习和研究使用。

---

## 更新日志

### v3.0 (2026-06-06)
- 重构为多文件模块化架构
- 采用现代 C++17 标准
- 改进代码可读性和可维护性
- 添加详细的词法分析可视化输出
- 优化错误处理机制

### v2.0
- 添加 DFA 正则表达式验证
- 支持注释识别
- 改进符号表管理

### v1.0
- 初始版本
- 基础 PL/0 编译功能