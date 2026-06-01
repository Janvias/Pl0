/**
 * @file list.h
 * @brief Generic Doubly Linked List Implementation Header
 * 
 * This header file provides a generic doubly linked list implementation
 * using void pointers. The list can store any data type and is designed
 * to be used in various compiler components such as:
 *   - Symbol table management
 *   - Quadruple list management
 *   - Token stream management
 *   - Error message collection
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

/**
 * @struct ListNode
 * @brief Represents a node in the linked list
 */
typedef struct ListNode {
    void* data;              /**< Pointer to the data stored in this node */
    struct ListNode* prev;   /**< Pointer to the previous node */
    struct ListNode* next;   /**< Pointer to the next node */
} ListNode;

/**
 * @struct List
 * @brief Represents a doubly linked list
 */
typedef struct {
    ListNode* head;          /**< Pointer to the first node */
    ListNode* tail;          /**< Pointer to the last node */
    int size;                /**< Number of nodes in the list */
    void (*free_func)(void*);/**< Function to free data memory (optional) */
} List;

/**
 * @brief Creates a new empty list
 * 
 * @param free_func Optional function to free data memory when node is removed
 * @return Pointer to the newly created list, or NULL on failure
 */
List* list_create(void (*free_func)(void*));

/**
 * @brief Destroys a list and frees all allocated memory
 * 
 * @param list Pointer to the list to destroy
 */
void list_destroy(List* list);

/**
 * @brief Checks if the list is empty
 * 
 * @param list Pointer to the list
 * @return true if empty, false otherwise
 */
bool list_is_empty(const List* list);

/**
 * @brief Returns the number of items in the list
 * 
 * @param list Pointer to the list
 * @return Number of items in the list
 */
int list_size(const List* list);

/**
 * @brief Adds an item to the beginning of the list
 * 
 * @param list Pointer to the list
 * @param data Pointer to the data to add
 * @return true if successful, false otherwise
 */
bool list_prepend(List* list, void* data);

/**
 * @brief Adds an item to the end of the list
 * 
 * @param list Pointer to the list
 * @param data Pointer to the data to add
 * @return true if successful, false otherwise
 */
bool list_append(List* list, void* data);

/**
 * @brief Inserts an item at the specified position
 * 
 * @param list Pointer to the list
 * @param data Pointer to the data to insert
 * @param index Position to insert at (0-based)
 * @return true if successful, false otherwise
 */
bool list_insert_at(List* list, void* data, int index);

/**
 * @brief Removes and returns the first item from the list
 * 
 * @param list Pointer to the list
 * @return Pointer to the removed data, or NULL if list is empty
 */
void* list_remove_first(List* list);

/**
 * @brief Removes and returns the last item from the list
 * 
 * @param list Pointer to the list
 * @return Pointer to the removed data, or NULL if list is empty
 */
void* list_remove_last(List* list);

/**
 * @brief Removes and returns the item at the specified position
 * 
 * @param list Pointer to the list
 * @param index Position to remove from (0-based)
 * @return Pointer to the removed data, or NULL if index is invalid
 */
void* list_remove_at(List* list, int index);

/**
 * @brief Returns the data at the specified position without removing it
 * 
 * @param list Pointer to the list
 * @param index Position to get (0-based)
 * @return Pointer to the data, or NULL if index is invalid
 */
void* list_get(const List* list, int index);

/**
 * @brief Returns the first item without removing it
 * 
 * @param list Pointer to the list
 * @return Pointer to the first item, or NULL if list is empty
 */
void* list_get_first(const List* list);

/**
 * @brief Returns the last item without removing it
 * 
 * @param list Pointer to the list
 * @return Pointer to the last item, or NULL if list is empty
 */
void* list_get_last(const List* list);

/**
 * @brief Clears all items from the list
 * 
 * @param list Pointer to the list
 */
void list_clear(List* list);

/**
 * @brief Reverses the list in place
 * 
 * @param list Pointer to the list
 */
void list_reverse(List* list);

/**
 * @brief Iterates over the list and calls callback for each item
 * 
 * @param list Pointer to the list
 * @param callback Function to call for each item (returns true to continue)
 * @param user_data Optional user data to pass to callback
 */
void list_foreach(const List* list, bool (*callback)(void*, void*), void* user_data);

#endif /* LIST_H */