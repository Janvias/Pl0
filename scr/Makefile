CC = gcc
CFLAGS = -Wall -g
TARGET = compiler
OBJS = main.o lexer.o parser.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c compiler.h
	$(CC) $(CFLAGS) -c main.c

lexer.o: lexer.c compiler.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c compiler.h
	$(CC) $(CFLAGS) -c parser.c

clean:
	del /Q $(OBJS) $(TARGET).exe 2>nul || echo Clean done

run: $(TARGET)
	$(TARGET).exe

.PHONY: all clean run