# PL/0 Compiler Test Suite Makefile
# Builds and runs the comprehensive test harness

CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2 -I../include -I.. -I.
CXXFLAGS_O0 = -Wall -std=c++17 -O0 -I../include -I.. -I.
BUILD_DIR = build
TARGET = pl0_test_runner

# Compiler source files (relative to scr/)
SRC_DIR = ../src
DFA_DIR = ../dfa
INC_DIR = ../include

# Files requiring -O0 to avoid MinGW segfault
O0_SRCS = $(SRC_DIR)/lexer.cpp $(SRC_DIR)/compiler.cpp $(SRC_DIR)/visualizer.cpp \
          $(DFA_DIR)/nfa.cpp $(DFA_DIR)/dfa.cpp $(DFA_DIR)/regex_parser.cpp

# Files compiled with -O2
O2_SRCS = $(SRC_DIR)/parser.cpp $(SRC_DIR)/symtable.cpp $(SRC_DIR)/codegen.cpp \
          $(SRC_DIR)/ast_builder.cpp $(SRC_DIR)/error_suggestor.cpp $(SRC_DIR)/grammar_normalizer.cpp \
          $(SRC_DIR)/lr1_grammar.cpp $(SRC_DIR)/lr1_items.cpp $(SRC_DIR)/lr1_parser_driver.cpp $(SRC_DIR)/lr1_diagnostics.cpp \
          $(DFA_DIR)/regex_access.cpp $(DFA_DIR)/dfa_stubs.cpp

# Test runner source
TEST_SRC = run_tests.cpp

OBJS = $(O0_SRCS:.cpp=.o) $(O2_SRCS:.cpp=.o) $(TEST_SRC:.cpp=.o)

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).exe

$(BUILD_DIR):
	-mkdir $(BUILD_DIR)
	-mkdir output

# Default rule with -O2
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Files requiring -O0 to avoid MinGW segfault
$(SRC_DIR)/lexer.o: $(SRC_DIR)/lexer.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(SRC_DIR)/compiler.o: $(SRC_DIR)/compiler.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(DFA_DIR)/nfa.o: $(DFA_DIR)/nfa.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(DFA_DIR)/dfa.o: $(DFA_DIR)/dfa.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(DFA_DIR)/regex_parser.o: $(DFA_DIR)/regex_parser.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(SRC_DIR)/visualizer.o: $(SRC_DIR)/visualizer.cpp
	$(CXX) $(CXXFLAGS_O0) -c $< -o $@

$(BUILD_DIR)/$(TARGET).exe: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Run tests (works from both scr/ and scr/tests/)
run: $(BUILD_DIR)/$(TARGET).exe
	$(BUILD_DIR)\$(TARGET).exe

# Run tests with verbose compiler output
run-verbose: $(BUILD_DIR)/$(TARGET).exe
	$(BUILD_DIR)\$(TARGET).exe -v

# Build only
build: $(BUILD_DIR)/$(TARGET).exe

# Clean build artifacts
clean:
	rm -f $(OBJS) $(BUILD_DIR)/$(TARGET).exe
	rm -f $(SRC_DIR)/*.o $(DFA_DIR)/*.o run_tests.o

# Clean everything including test outputs
distclean: clean
	rm -rf output

.PHONY: all run run-verbose build clean distclean
