/**
 * @file list.c
 * @brief Generic Doubly Linked List Implementation
 * 
 * This file contains the implementation of a generic doubly linked list
 * data structure that can be used throughout the PL/0 compiler project.
 * 
 * @author PL/0 Compiler Project
 * @date 2026-06-01
 * @version 1.0
 */

#include "list.h"
#include <stdlib.h>

/**
 * @brief Creates a new list node
 */
static ListNode* list_node_create(void* data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    if (!node) {
        return NULL;
    }
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/**
 * @brief Destroys a list node
 */
static void list_node_destroy(ListNode* node, void (*free_func)(void*)) {
    if (!node) {
        return;
    }
    if (free_func && node->data) {
        free_func(node->data);
    }
    free(node);
}

/**
 * @brief Creates a new empty list
 */
List* list_create(void (*free_func)(void*)) {
    List* list = (List*)malloc(sizeof(List));
    if (!list) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->free_func = free_func;
    return list;
}

/**
 * @brief Destroys a list and frees all allocated memory
 */
void list_destroy(List* list) {
    if (!list) {
        return;
    }
    list_clear(list);
    free(list);
}

/**
 * @brief Checks if the list is empty
 */
bool list_is_empty(const List* list) {
    return (list == NULL || list->head == NULL);
}

/**
 * @brief Returns the number of items in the list
 */
int list_size(const List* list) {
    if (!list) {
        return 0;
    }
    return list->size;
}

/**
 * @brief Adds an item to the beginning of the list
 */
bool list_prepend(List* list, void* data) {
    if (!list) {
        return false;
    }
    
    ListNode* node = list_node_create(data);
    if (!node) {
        return false;
    }
    
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    
    list->size++;
    return true;
}

/**
 * @brief Adds an item to the end of the list
 */
bool list_append(List* list, void* data) {
    if (!list) {
        return false;
    }
    
    ListNode* node = list_node_create(data);
    if (!node) {
        return false;
    }
    
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    
    list->size++;
    return true;
}

/**
 * @brief Inserts an item at the specified position
 */
bool list_insert_at(List* list, void* data, int index) {
    if (!list || index < 0 || index > list->size) {
        return false;
    }
    
    if (index == 0) {
        return list_prepend(list, data);
    }
    
    if (index == list->size) {
        return list_append(list, data);
    }
    
    ListNode* node = list_node_create(data);
    if (!node) {
        return false;
    }
    
    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    node->prev = current->prev;
    node->next = current;
    current->prev->next = node;
    current->prev = node;
    
    list->size++;
    return true;
}

/**
 * @brief Removes and returns the first item from the list
 */
void* list_remove_first(List* list) {
    if (!list || !list->head) {
        return NULL;
    }
    
    ListNode* node = list->head;
    void* data = node->data;
    
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        list->head->prev = NULL;
    }
    
    node->next = NULL;
    free(node);
    list->size--;
    
    return data;
}

/**
 * @brief Removes and returns the last item from the list
 */
void* list_remove_last(List* list) {
    if (!list || !list->tail) {
        return NULL;
    }
    
    ListNode* node = list->tail;
    void* data = node->data;
    
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        list->tail->next = NULL;
    }
    
    node->prev = NULL;
    free(node);
    list->size--;
    
    return data;
}

/**
 * @brief Removes and returns the item at the specified position
 */
void* list_remove_at(List* list, int index) {
    if (!list || index < 0 || index >= list->size) {
        return NULL;
    }
    
    if (index == 0) {
        return list_remove_first(list);
    }
    
    if (index == list->size - 1) {
        return list_remove_last(list);
    }
    
    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    void* data = current->data;
    
    current->prev->next = current->next;
    current->next->prev = current->prev;
    
    current->prev = NULL;
    current->next = NULL;
    free(current);
    list->size--;
    
    return data;
}

/**
 * @brief Returns the data at the specified position without removing it
 */
void* list_get(const List* list, int index) {
    if (!list || index < 0 || index >= list->size) {
        return NULL;
    }
    
    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

/**
 * @brief Returns the first item without removing it
 */
void* list_get_first(const List* list) {
    if (!list || !list->head) {
        return NULL;
    }
    return list->head->data;
}

/**
 * @brief Returns the last item without removing it
 */
void* list_get_last(const List* list) {
    if (!list || !list->tail) {
        return NULL;
    }
    return list->tail->data;
}

/**
 * @brief Clears all items from the list
 */
void list_clear(List* list) {
    if (!list) {
        return;
    }
    
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        list_node_destroy(current, list->free_func);
        current = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/**
 * @brief Reverses the list in place
 */
void list_reverse(List* list) {
    if (!list || !list->head) {
        return;
    }
    
    ListNode* current = list->head;
    ListNode* temp = NULL;
    
    while (current) {
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev;
    }
    
    temp = list->head;
    list->head = list->tail;
    list->tail = temp;
}

/**
 * @brief Iterates over the list and calls callback for each item
 */
void list_foreach(const List* list, bool (*callback)(void*, void*), void* user_data) {
    if (!list || !callback) {
        return;
    }
    
    ListNode* current = list->head;
    while (current) {
        if (!callback(current->data, user_data)) {
            break;
        }
        current = current->next;
    }
}