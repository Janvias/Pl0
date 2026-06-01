# Stack Utility

This directory contains a generic stack implementation for the PL/0 compiler project.

## Files

- `stack.h` - Header file with stack data structure and function declarations
- `stack.c` - Implementation of stack operations

## Features

- Generic implementation using void pointers
- Dynamic resizing support
- Optional memory cleanup via free function
- Thread-safe design (can be extended)
- Comprehensive error handling

## Usage Example

```c
#include "stack/stack.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a stack with capacity 10
    Stack* stack = stack_create(10, NULL);
    
    // Push items onto the stack
    int* val1 = malloc(sizeof(int));
    *val1 = 10;
    stack_push(stack, val1);
    
    // Pop items from the stack
    int* popped = stack_pop(stack);
    printf("Popped: %d\n", *popped);
    free(popped);
    
    // Destroy the stack
    stack_destroy(stack);
    return 0;
}
```

## API Reference

### Stack* stack_create(int capacity, void (*free_func)(void*))
Creates a new stack with specified capacity.

### void stack_destroy(Stack* stack)
Destroys a stack and frees all allocated memory.

### bool stack_is_empty(const Stack* stack)
Checks if the stack is empty.

### bool stack_is_full(const Stack* stack)
Checks if the stack is full.

### int stack_size(const Stack* stack)
Returns the number of items in the stack.

### bool stack_push(Stack* stack, void* item)
Pushes an item onto the stack.

### void* stack_pop(Stack* stack)
Pops an item from the stack.

### void* stack_peek(const Stack* stack)
Returns the top item without removing it.

### void stack_clear(Stack* stack)
Clears all items from the stack.

### bool stack_resize(Stack* stack, int new_capacity)
Resizes the stack to a new capacity.

## Integration with PL/0 Compiler

This stack implementation can be used in:
1. **Symbol Table Management**: Track symbol scopes during parsing
2. **Expression Evaluation**: Operand stack for expression parsing
3. **Jump Address Tracking**: Track loop and conditional jump addresses
4. **Procedure Call Stack**: Manage procedure activation records

## Compilation

To compile with your project:
```
gcc -c stack/stack.c -o stack/stack.o
```

Link with your main program:
```
gcc main.c stack/stack.o -o compiler
```