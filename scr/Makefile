CC = gcc
CXX = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g
TARGET = compiler
OBJS = main.o lexer.o parser.o list/list.o symtable/symtable.o \
       dfa/regex_parser.o dfa/nfa.o dfa/dfa.o dfa/dfa_wrapper.o dfa/regex_access.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c compiler.h
	$(CC) $(CFLAGS) -c main.c -o main.o

lexer.o: lexer.c compiler.h dfa/dfa_wrapper.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o

parser.o: parser.c compiler.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

list/list.o: list/list.c list/list.h
	$(CC) $(CFLAGS) -c list/list.c -o list/list.o

symtable/symtable.o: symtable/symtable.c symtable/symtable.h
	$(CC) $(CFLAGS) -c symtable/symtable.c -o symtable/symtable.o

dfa/regex_parser.o: dfa/regex_parser.cpp dfa/regex_parser.h
	$(CXX) $(CXXFLAGS) -c dfa/regex_parser.cpp -o dfa/regex_parser.o

dfa/nfa.o: dfa/nfa.cpp dfa/nfa.h
	$(CXX) $(CXXFLAGS) -c dfa/nfa.cpp -o dfa/nfa.o

dfa/dfa.o: dfa/dfa.cpp dfa/dfa.h dfa/nfa.h
	$(CXX) $(CXXFLAGS) -c dfa/dfa.cpp -o dfa/dfa.o

dfa/dfa_wrapper.o: dfa/dfa_wrapper.cpp dfa/dfa_wrapper.h \
                    dfa/regex_parser.h dfa/nfa.h dfa/dfa.h
	$(CXX) $(CXXFLAGS) -c dfa/dfa_wrapper.cpp -o dfa/dfa_wrapper.o

dfa/regex_access.o: dfa/regex_access.cpp dfa/regex_access.h pl0_regex/pl0_regex.h
	$(CXX) $(CXXFLAGS) -c dfa/regex_access.cpp -o dfa/regex_access.o

clean:
	del /Q $(OBJS) $(TARGET).exe 2>nul || echo Clean done

run: $(TARGET)
	$(TARGET).exe

.PHONY: all clean run