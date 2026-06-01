# List Utility

This directory contains a generic doubly linked list implementation for the PL/0 compiler project.

## Files

- `list.h` - Header file with list data structure and function declarations
- `list.c` - Implementation of list operations

## Features

- Generic implementation using void pointers
- Doubly linked list for efficient forward/backward traversal
- Optional memory cleanup via free function
- Comprehensive list operations
- Thread-safe design (can be extended)

## Usage Example

```c
#include "list/list.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a list
    List* list = list_create(NULL);
    
    // Append items
    int* val1 = malloc(sizeof(int));
    *val1 = 10;
    list_append(list, val1);
    
    // Prepend items
    int* val2 = malloc(sizeof(int));
    *val2 = 5;
    list_prepend(list, val2);
    
    // Get item at index
    int* item = list_get(list, 1);
    printf("Item at index 1: %d\n", *item);
    
    // Iterate through list
    printf("List contents: ");
    list_foreach(list, print_item, NULL);
    
    // Destroy the list
    list_destroy(list);
    return 0;
}
```

## API Reference

### List* list_create(void (*free_func)(void*))
Creates a new empty list.

### void list_destroy(List* list)
Destroys a list and frees all allocated memory.

### bool list_is_empty(const List* list)
Checks if the list is empty.

### int list_size(const List* list)
Returns the number of items in the list.

### bool list_prepend(List* list, void* data)
Adds an item to the beginning of the list.

### bool list_append(List* list, void* data)
Adds an item to the end of the list.

### bool list_insert_at(List* list, void* data, int index)
Inserts an item at the specified position.

### void* list_remove_first(List* list)
Removes and returns the first item.

### void* list_remove_last(List* list)
Removes and returns the last item.

### void* list_remove_at(List* list, int index)
Removes and returns the item at the specified position.

### void* list_get(const List* list, int index)
Returns the item at the specified position.

### void* list_get_first(const List* list)
Returns the first item without removing it.

### void* list_get_last(const List* list)
Returns the last item without removing it.

### void list_clear(List* list)
Clears all items from the list.

### void list_reverse(List* list)
Reverses the list in place.

### void list_foreach(const List* list, bool (*callback)(void*, void*), void* user_data)
Iterates over the list and calls callback for each item.

## Integration with PL/0 Compiler

This list implementation can be used in:
1. **Symbol Table Management**: Store symbols in scope order
2. **Quadruple List**: Store generated intermediate code
3. **Token Stream**: Store tokens during lexical analysis
4. **Error Collection**: Collect error messages during compilation

## Compilation

To compile with your project:
```
gcc -c list/list.c -o list/list.o
```

Link with your main program:
```
gcc main.c list/list.o -o compiler
```