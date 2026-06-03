CC = gcc
CXX = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g
TARGET = compiler
OBJS = main.o lexer.o parser.o list/list.o symtable/symtable.o \
       pl0RegexDfa/regex_parser.o pl0RegexDfa/nfa.o pl0RegexDfa/dfa.o pl0RegexDfa/dfa_wrapper.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c compiler.h
	$(CC) $(CFLAGS) -c main.c -o main.o

lexer.o: lexer.c compiler.h pl0RegexDfa/dfa_wrapper.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o

parser.o: parser.c compiler.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

list/list.o: list/list.c list/list.h
	$(CC) $(CFLAGS) -c list/list.c -o list/list.o

symtable/symtable.o: symtable/symtable.c symtable/symtable.h
	$(CC) $(CFLAGS) -c symtable/symtable.c -o symtable/symtable.o

pl0RegexDfa/regex_parser.o: pl0RegexDfa/regex_parser.cpp pl0RegexDfa/regex_parser.h
	$(CXX) $(CXXFLAGS) -c pl0RegexDfa/regex_parser.cpp -o pl0RegexDfa/regex_parser.o

pl0RegexDfa/nfa.o: pl0RegexDfa/nfa.cpp pl0RegexDfa/nfa.h
	$(CXX) $(CXXFLAGS) -c pl0RegexDfa/nfa.cpp -o pl0RegexDfa/nfa.o

pl0RegexDfa/dfa.o: pl0RegexDfa/dfa.cpp pl0RegexDfa/dfa.h pl0RegexDfa/nfa.h
	$(CXX) $(CXXFLAGS) -c pl0RegexDfa/dfa.cpp -o pl0RegexDfa/dfa.o

pl0RegexDfa/dfa_wrapper.o: pl0RegexDfa/dfa_wrapper.cpp pl0RegexDfa/dfa_wrapper.h \
                          pl0RegexDfa/regex_parser.h pl0RegexDfa/nfa.h pl0RegexDfa/dfa.h
	$(CXX) $(CXXFLAGS) -c pl0RegexDfa/dfa_wrapper.cpp -o pl0RegexDfa/dfa_wrapper.o

clean:
	del /Q $(OBJS) $(TARGET).exe 2>nul || echo Clean done

run: $(TARGET)
	$(TARGET).exe

.PHONY: all clean run
