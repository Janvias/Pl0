# PL/0 Syntax Analyzer Makefile (精简版)
# LL(1) + SLR(1) 双模块语法分析系统

CXX = g++
CXXFLAGS = -Wall -std=c++17 -I.
TARGET = syntax_analyzer

# 精简版：单文件编译
all: $(TARGET)

$(TARGET): main_syntax.cpp include/syntax_analyzer.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main_syntax.cpp

# 运行
run: $(TARGET)
	./$(TARGET)

# 清理
clean:
	rm -f $(TARGET) *.o src/*.o dfa/*.o

# 测试
test: $(TARGET)
	./$(TARGET)

.PHONY: all run clean test