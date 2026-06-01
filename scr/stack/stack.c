/**
 * @file stack.c
 * @brief Generic Stack Implementation
 * 
 * This file contains the implementation of a generic stack data structure
 * that can be used throughout the PL/0 compiler project.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#include "stack.h"
#include <stdlib.h>

/**
 * @brief Creates a new stack with specified capacity
 */
Stack* stack_create(int capacity, void (*free_func)(void*)) {
    if (capacity <= 0) {
        return NULL;
    }
    
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if (!stack) {
        return NULL;
    }
    
    stack->items = (void**)malloc(capacity * sizeof(void*));
    if (!stack->items) {
        free(stack);
        return NULL;
    }
    
    stack->capacity = capacity;
    stack->top = -1;
    stack->free_func = free_func;
    
    return stack;
}

/**
 * @brief Destroys a stack and frees all allocated memory
 */
void stack_destroy(Stack* stack) {
    if (!stack) {
        return;
    }
    
    stack_clear(stack);
    free(stack->items);
    free(stack);
}

/**
 * @brief Checks if the stack is empty
 */
bool stack_is_empty(const Stack* stack) {
    return (stack == NULL || stack->top == -1);
}

/**
 * @brief Checks if the stack is full
 */
bool stack_is_full(const Stack* stack) {
    return (stack != NULL && stack->top >= stack->capacity - 1);
}

/**
 * @brief Returns the number of items in the stack
 */
int stack_size(const Stack* stack) {
    if (!stack) {
        return 0;
    }
    return stack->top + 1;
}

/**
 * @brief Pushes an item onto the stack
 */
bool stack_push(Stack* stack, void* item) {
    if (!stack || stack_is_full(stack)) {
        return false;
    }
    
    stack->items[++stack->top] = item;
    return true;
}

/**
 * @brief Pops an item from the stack
 */
void* stack_pop(Stack* stack) {
    if (!stack || stack_is_empty(stack)) {
        return NULL;
    }
    
    return stack->items[stack->top--];
}

/**
 * @brief Returns the top item without removing it
 */
void* stack_peek(const Stack* stack) {
    if (!stack || stack_is_empty(stack)) {
        return NULL;
    }
    
    return stack->items[stack->top];
}

/**
 * @brief Clears all items from the stack
 */
void stack_clear(Stack* stack) {
    if (!stack) {
        return;
    }
    
    if (stack->free_func) {
        for (int i = 0; i <= stack->top; i++) {
            stack->free_func(stack->items[i]);
        }
    }
    
    stack->top = -1;
}

/**
 * @brief Resizes the stack to a new capacity
 */
bool stack_resize(Stack* stack, int new_capacity) {
    if (!stack || new_capacity <= 0) {
        return false;
    }
    
    void** new_items = (void**)realloc(stack->items, new_capacity * sizeof(void*));
    if (!new_items) {
        return false;
    }
    
    stack->items = new_items;
    stack->capacity = new_capacity;
    
    if (stack->top >= stack->capacity) {
        stack->top = stack->capacity - 1;
    }
    
    return true;
}