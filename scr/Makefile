CC = gcc
CFLAGS = -Wall -g
TARGET = compiler
OBJS = main.o lexer.o parser.o list/list.o symtable/symtable.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c compiler.h
	$(CC) $(CFLAGS) -c main.c -o main.o

lexer.o: lexer.c compiler.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o

parser.o: parser.c compiler.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

list/list.o: list/list.c list/list.h
	$(CC) $(CFLAGS) -c list/list.c -o list/list.o

symtable/symtable.o: symtable/symtable.c symtable/symtable.h
	$(CC) $(CFLAGS) -c symtable/symtable.c -o symtable/symtable.o

clean:
	del /Q $(OBJS) $(TARGET).exe 2>nul || echo Clean done

run: $(TARGET)
	$(TARGET).exe

.PHONY: all clean run