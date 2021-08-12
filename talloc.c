/* talloc.c
 * Author: Khalid Hussain
 * --------------------
 * This program is a replacement for malloc that stores the pointers allocated
 * in a linked list.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "headers/linkedlist.h"
#include "headers/value.h"
#include "headers/talloc.h"

struct node *globallist = NULL; // global linked list of pointers

/* Function: talloc
 * --------------------
 *   Function that performs the talloc by storing a pointer, malloc'd for a
 *   particular size, in a linked list.
 *
 *   size: The size of the pointer that needs to be malloc'd.
 *   returns: The pointer that was malloc'd
 */

void *talloc(size_t size){
  struct node *link = (struct node*) malloc(sizeof(struct node));
  void *pointer = malloc(size);

  link->data = pointer;
  link->next = globallist;
  globallist = link;

  return pointer;
}

/* Function: tfree
 * --------------------
 *   Function that free's all pointers allocated by talloc, and the memory
 *   allocated for the global list to hold these pointers.
 */

void tfree() {
  struct node* current = globallist;
  while (current != NULL) {
    struct node* temp = current -> next;
    free(current -> data);
    free(current);
    current = temp;
  }
  globallist = NULL;
}

/* Function: texit
 * --------------------
 *   Function that is a replacement for the "exit". It calls tfree() before
 *   exit, and is useful for exiting the program abruptly after encountering an
 *   error, while cleaning up all used memory.
 *
 *   status: The exit status for the program where texit() is called.
 */

void texit(int status){
  tfree();
  exit(status);
}