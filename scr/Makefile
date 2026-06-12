# PL/0 Compiler Makefile
# LL(1) + LR(1) Dual Parser System
# Note: lexer.cpp and compiler.cpp use -O0 due to MinGW optimization bugs

CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2 -I.
TARGET = pl0_compiler
BUILD_DIR = build

# Files that need -O0 (MinGW optimization workaround)
O0_SRCS = src/lexer.cpp src/compiler.cpp

# Files compiled with -O2
O2_SRCS = src/parser.cpp src/symtable.cpp src/codegen.cpp \
          src/lr1_grammar.cpp src/lr1_items.cpp src/lr1_parser_driver.cpp src/lr1_diagnostics.cpp \
          main.cpp dfa/regex_access.cpp dfa/dfa_stubs.cpp

OBJS = $(O0_SRCS:.cpp=.o) $(O2_SRCS:.cpp=.o)

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).exe

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Default rule with -O2
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Files requiring -O0 to avoid MinGW segfault
src/lexer.o: src/lexer.cpp
	$(CXX) -Wall -std=c++17 -O0 -I. -c $< -o $@

src/compiler.o: src/compiler.cpp
	$(CXX) -Wall -std=c++17 -O0 -I. -c $< -o $@

$(BUILD_DIR)/$(TARGET).exe: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(BUILD_DIR)/$(TARGET).exe

test: $(BUILD_DIR)/$(TARGET).exe
	@echo "=== test.pl0 ==="
	@for mode in --ll1 --lr1 --dual; do \
		echo -n "  $$mode: "; \
		./$(BUILD_DIR)/$(TARGET).exe test.pl0 $$mode 2>&1 | tail -1; \
	done
	@echo "=== test_procedure.pl0 ==="
	@for mode in --ll1 --lr1 --dual; do \
		echo -n "  $$mode: "; \
		./$(BUILD_DIR)/$(TARGET).exe test_procedure.pl0 $$mode 2>&1 | tail -1; \
	done
	@echo "=== test_complex.pl0 ==="
	@for mode in --ll1 --lr1 --dual; do \
		echo -n "  $$mode: "; \
		./$(BUILD_DIR)/$(TARGET).exe test_complex.pl0 $$mode 2>&1 | tail -1; \
	done

.PHONY: all clean test
