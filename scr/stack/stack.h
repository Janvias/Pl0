/**
 * @file stack.h
 * @brief Generic Stack Implementation Header
 * 
 * This header file provides a generic stack implementation using void pointers.
 * The stack can store any data type and is designed to be used in various
 * compiler components such as:
 *   - Symbol table management during parsing
 *   - Expression evaluation
 *   - Jump address tracking for code generation
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

/**
 * @struct Stack
 * @brief Represents a generic stack data structure
 */
typedef struct {
    void** items;          /**< Array of pointers to stored items */
    int capacity;          /**< Maximum number of items the stack can hold */
    int top;               /**< Index of the top element (-1 means empty) */
    void (*free_func)(void*); /**< Function to free item memory (optional) */
} Stack;

/**
 * @brief Creates a new stack with specified capacity
 * 
 * @param capacity Maximum number of items the stack can hold
 * @param free_func Optional function to free item memory when popped
 * @return Pointer to the newly created stack, or NULL on failure
 */
Stack* stack_create(int capacity, void (*free_func)(void*));

/**
 * @brief Destroys a stack and frees all allocated memory
 * 
 * @param stack Pointer to the stack to destroy
 */
void stack_destroy(Stack* stack);

/**
 * @brief Checks if the stack is empty
 * 
 * @param stack Pointer to the stack
 * @return true if empty, false otherwise
 */
bool stack_is_empty(const Stack* stack);

/**
 * @brief Checks if the stack is full
 * 
 * @param stack Pointer to the stack
 * @return true if full, false otherwise
 */
bool stack_is_full(const Stack* stack);

/**
 * @brief Returns the number of items in the stack
 * 
 * @param stack Pointer to the stack
 * @return Number of items in the stack
 */
int stack_size(const Stack* stack);

/**
 * @brief Pushes an item onto the stack
 * 
 * @param stack Pointer to the stack
 * @param item Pointer to the item to push
 * @return true if successful, false if stack is full
 */
bool stack_push(Stack* stack, void* item);

/**
 * @brief Pops an item from the stack
 * 
 * @param stack Pointer to the stack
 * @return Pointer to the popped item, or NULL if stack is empty
 */
void* stack_pop(Stack* stack);

/**
 * @brief Returns the top item without removing it
 * 
 * @param stack Pointer to the stack
 * @return Pointer to the top item, or NULL if stack is empty
 */
void* stack_peek(const Stack* stack);

/**
 * @brief Clears all items from the stack
 * 
 * @param stack Pointer to the stack
 */
void stack_clear(Stack* stack);

/**
 * @brief Resizes the stack to a new capacity
 * 
 * @param stack Pointer to the stack
 * @param new_capacity New maximum capacity
 * @return true if successful, false on failure
 */
bool stack_resize(Stack* stack, int new_capacity);

#endif /* STACK_H */