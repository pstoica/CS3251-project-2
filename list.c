/**
 * CS 2110 - Fall 2011 - Homework #11
 * Patrick Stoica
 * Edited by: Brandon Whitehead
 *
 * list.c: Complete the functions!
 **/

#include <stdlib.h>
#include <stdio.h>
#include "list.h"

/* The node struct.  Has a prev pointer, next pointer, and data. */
/* DO NOT DEFINE ANYTHING OTHER THAN WHAT'S GIVEN OR YOU WILL GET A ZERO*/
/* Design consideration only this file should know about nodes */
/* Only this file should be manipulating nodes */
typedef struct lnode {
  struct lnode* prev; /* Pointer to previous node */
  struct lnode* next; /* Pointer to next node */
  void* data; /* User data */
} node;


/* Do not create any global variables here. Your linked list library should obviously work for multiple linked lists */
// This function is declared as static since you should only be calling this inside this file.
static node* create_node(void* data);

/** create_list
  *
  * Creates a list by allocating memory for it on the heap.
  * Be sure to initialize size to zero and head to NULL.
  *
  * @return an empty linked list
  */
list* create_list(void) {
	list *newList = malloc(sizeof(list));
	if (newList == NULL) {
		printf("Malloc failed.");
		exit(0);
	} else {
		newList->size = 0;
		newList->head = NULL;
		return newList;
	}
}

/** create_node
  *
  * Helper function that creates a node by allocating memory for it on the heap.
  * Be sure to set its pointers to NULL.
  *
  * @param data a void pointer to data the user wants to store in the list
  * @return a node
  */
static node* create_node(void* data) {
	node *newNode = malloc(sizeof(node));
	if (newNode == NULL) {
		printf("Malloc failed.");
		exit(0);
	} else {
		newNode->prev = NULL;
		newNode->next = NULL;
		newNode->data = data;
		return newNode;
	}
}

/** push_front
  *
  * Adds the data to the front of the linked list
  *
  * @param llist a pointer to the list.
  * @param data pointer to data the user wants to store in the list.
  */
void push_front(list* llist, void* data) {
	node *newNode = create_node(data);

	if (!is_empty(llist)) {
		node *prev = llist->head->prev;
		node *next = llist->head;
		prev->next = newNode;
		next->prev = newNode;
		newNode->next = next;
		newNode->prev = prev;
		llist->head = newNode;
	} else {
		// if it's empty, add it as the head
		llist->head = newNode;
		llist->head->prev = llist->head;
		llist->head->next = llist->head;
	}

	llist->size++;
}

/** push_back
  *
  * Adds the data to the back/end of the linked list
  *
  * @param llist a pointer to the list.
  * @param data pointer to data the user wants to store in the list.
  */
void push_back(list* llist, void* data) {
	node *newNode = create_node(data);
	if (!is_empty(llist)) {
		llist->head->prev->next = newNode;
		newNode->prev = llist->head->prev;
		newNode->next = llist->head;
		llist->head->prev = newNode;
	} else {
		// if it's empty, add it as the head
		llist->head = newNode;
		llist->head->prev = llist->head;
		llist->head->next = llist->head;
	}

	llist->size++;
}

/** remove_front
  *
  * Removes the node at the front of the linked list
  *
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list.
  * @param free_func pointer to a function that is responsible for freeing the node's data.
  * @return -1 if the remove failed (which is only there are no elements) 0 if the remove succeeded.
  */
int remove_front(list* llist, list_op free_func) {
	if (is_empty(llist)) {
		return -1;
	} else if (llist->size == 1) {
		// clear if only one element
		free_func(llist->head->data);
		free(llist->head);
		llist->head = NULL;
		llist->size--;
		return 0;
	} else {
		node *prev;
		node *next;

		prev = llist->head->prev;
		next = llist->head->next;
		prev->next = next;
		next->prev = prev;

		free_func(llist->head->data);
		free(llist->head);
		llist->head = next;
		llist->size--;
		return 0;
	}
}

/** remove_index
  *
  * Removes the indexth node of the linked list
  *
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list.
  * @param index index of the node to remove.
  * @param free_func pointer to a function that is responsible for freeing the node's data.
  * @return -1 if the remove failed 0 if the remove succeeded.
  */
int remove_index(list* llist, int index, list_op free_func)
{
	if (is_empty(llist) || index >= llist->size || index < 0) {
		return -1;
	} else if (index == 0) {
		// remove front
		return remove_front(llist, free_func);
	} else if (index == llist->size - 1) {
		// remove back
		return remove_back(llist, free_func);
	} else {
		int i = 0;
		node *current = llist->head;
		while (i < index) {
			current = current->next;
			i++;
		}

		node *prev;
		node *next;

		prev = current->prev;
		next = current->next;
		prev->next = next;
		next->prev = prev;

		free_func(current->data);
		free(current);
		llist->size--;
		return 0;
	}
}

/** remove_back
  *
  * Removes the node at the back of the linked list
  *
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list.
  * @param free_func pointer to a function that is responsible for freeing the node's data.
  * @return -1 if the remove failed 0 if the remove succeeded.
  */
int remove_back(list* llist, list_op free_func)
{
	if (is_empty(llist)) {
		return -1;
	} else if (llist->size == 1) {
		return remove_front(llist, free_func);
	} else {
		node *removed = llist->head->prev;
		node *prev;
		node *next;

		prev = removed->prev;
		next = removed->next;
		prev->next = next;
		next->prev = prev;

		free_func(removed->data);
		free(removed);
		llist->size--;
		return 0;
	}
}

/** remove_data
  *
  * Removes ALL nodes whose data is EQUAL to the data you passed in or rather when the comparison function returns true (!0)
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list
  * @param data data to compare to.
  * @param compare_func a pointer to a function that when it returns true it will remove the element from the list and do nothing otherwise @see equal_op.
  * @param free_func a pointer to a function that is responsible for freeing the node's data
  * @return the number of nodes that were removed.
  */
int remove_data(list* llist, const void* data, equal_op compare_func, list_op free_func)
{
	if (!is_empty(llist)) {
		node *upNext = llist->head;
		node *current;
		int i = 0;
		int count = 0;
		while (i < llist->size) {
			current = upNext;
			upNext = current->next;
			if (compare_func(current->data, data) && i > 0) {
				node *prev;
				node *next;

				prev = current->prev;
				next = current->next;
				prev->next = next;
				next->prev = prev;

				free_func(current->data);
				free(current);
				llist->size--;
				i--;
				count++;
			} else if (compare_func(current->data, data) && i == 0) {
				remove_front(llist, free_func);
				i--;
				count++;
			}
			i++;
		}
		return count;
	} else {
		return -1;
	}
}

/** remove_if
  *
  * Removes all nodes whose data when passed into the predicate function returns true
  *
  * @param llist a pointer to the list
  * @param pred_func a pointer to a function that when it returns true it will remove the element from the list and do nothing otherwise @see list_pred.
  * @param free_func a pointer to a function that is responsible for freeing the node's data
  * @return the number of nodes that were removed.
  */
int remove_if(list* llist, list_pred pred_func, list_op free_func) {
	if (!is_empty(llist)) {
		node *upNext = llist->head;
		node *current;
		int i = 0;
		int count = 0;
		while (i < llist->size) {
			current = upNext;
			upNext = current->next;
			if (pred_func(current->data) && i > 0) {
				node *prev;
				node *next;

				prev = current->prev;
				next = current->next;
				prev->next = next;
				next->prev = prev;

				free_func(current->data);
				free(current);
				llist->size--;
				i--;
				count++;
			} else if (pred_func(current->data) && i == 0) {
				remove_front(llist, free_func);
				i--;
				count++;
			}
			i++;
		}
		return count;
	} else {
		return -1;
	}
}

/** front
  *
  * Gets the data at the front of the linked list
  * If the list is empty return NULL.
  *
  * @param llist a pointer to the list
  * @return The data at the first node in the linked list or NULL.
  */
void* front(list* llist) {
    /// @note you are returning the HEAD's DATA not the head node. Remember the user should never deal with the linked list nodes.
	if (is_empty(llist)) {
		return NULL;
	} else {
		return llist->head->data;
	}
}

/** get_index
  *
  * Gets the data at the indexth node of the linked list
  * If the list is empty or if the index is invalid return NULL.
  *
  * @param llist a pointer to the list
  * @return The data at the indexth node in the linked list or NULL.
  */
void* get_index(list* llist, int index) {
	if (is_empty(llist) || index >= llist->size || index < 0) {
		return NULL;
	} else if (index == 0) {
		return front(llist);
	} else if (index == llist->size - 1) {
		return back(llist);
	} else {
		int i = 0;
		node *current = llist->head;
		while (i < index && current->next != llist->head) {
			current = current->next;
			i++;
		}

		return current->data;
	}
}

/** back
  *
  * Gets the data at the "end" of the linked list
  * If the list is empty return NULL.
  *
  * @param llist a pointer to the list
  * @return The data at the last node in the linked list or NULL.
  */
void* back(list* llist) {
	if (is_empty(llist)) {
		return NULL;
	} else {
		return llist->head->prev->data;
	}
}

/** is_empty
  *
  * Checks to see if the list is empty.
  *
  * @param llist a pointer to the list
  * @return 1 if the list is indeed empty 0 otherwise.
  */
int is_empty(list* llist) {
	if (llist->head == NULL && llist->size == 0) {
		return 1;
	} else {
		return 0;
	}
}

/** size
  *
  * Gets the size of the linked list
  *
  * @param llist a pointer to the list
  * @return The size of the linked list
  */
int size(list* llist) {
    return llist->size;
}

/** find_occurence
  *
  * Tests if the search data passed in is in the linked list.
  *
  * @param llist a pointer to a linked list.
  * @param search data to search for the occurence.
  * @param compare_func a pointer to a function that returns true if two data items are equal @see equal_op.
  * @return 1 if the data is indeed in the linked list 0 otherwise.
  */
int find_occurrence(list* llist, const void* search, equal_op compare_func) {
	if (is_empty(llist)) {
		return 0;
	} else {
		node *current = llist->head;
		int i = 0;
		int size = llist->size;
		while (i < size) {
			if (compare_func(current->data, search)) {
				return 1;
			}
			current = current->next;
			i++;
		} 
		return 0;
	}
}

/** empty_list
  *
  * Empties the list after this is called the list should be empty.
  *
  * @param llist a pointer to a linked list.
  * @param free_func function used to free the node's data.
  *
  */
void empty_list(list* llist, list_op free_func) {
	while (!is_empty(llist)) {
		remove_front(llist, free_func);
	}
}

/** traverse
  *
  * Traverses the linked list calling a function on each node's data.
  *
  * @param llist a pointer to a linked list.
  * @param do_func a function that does something to each node's data.
  */
void traverse(list* llist, list_op do_func) {
	if (!is_empty(llist)) {
		node *next = llist->head;
		node *current;
		int i = 0;
		int size = llist->size; // just in case do_func does something bad
		while (i < size) {
			current = next;
			next = current->next;
			do_func(current->data);
			i++;
		}
	}
}