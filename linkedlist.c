/* linkedlist.c
 * Author: Khalid Hussain
 * --------------------
 * This program is able to create a linked list in a similar fashion to the
 * "cons" function in Scheme using the Value struct, as well as perform a variety
 * of tasks on this linked list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/value.h"
#include "headers/linkedlist.h"
#include "headers/talloc.h"

/* Function: makeNull
 * --------------------
 *   Creates a new NULL_TYPE Value struct.
 *
 *   returns: This new NULL_TYPE Value struct.
 */

Value *makeNull() {
 Value *val = talloc(sizeof(Value));
 val -> type = NULL_TYPE;
 return val;
}

/* Function: cons
 * --------------------
 *   Create a new CONS_TYPE Value struct that, similar to a "cons" cell in Scheme,
 *   contains a value within the "car" of the struct, and either a linked list
 *   or a value within the "cdr".
 *
 *   newCar: The Value struct that will be the "car" of the CONS_TYPE Value struct.
 *   newCdr: The Value struct that will be the "cdr" of the CONS_TYPE Value struct.
 *   returns: The new CONS_TYPE Value struct.
 */

Value *cons(Value *newCar, Value *newCdr) {
  Value *val = talloc(sizeof(Value));
  val -> type = CONS_TYPE;
  val -> c.car = newCar;
  val -> c.cdr = newCdr;
  return val;
}

/* Function: reverse
 * --------------------
 *   Return a new linked list that is the reverse of the one that is passed in.
 *   No memory is shared between the original list and the new one, because all
 *   content within the original list is duplicated.
 *
 *   list: The linked list that will be reversed.
 *   returns: The reverse of the linked list passed in.
 */

Value *reverse(Value *list) {
  Value *prev = makeNull();
  Value *next = makeNull();

  while (true) {
    if (list -> type == NULL_TYPE) {
      return list;
    }

    else if (next -> type == NULL_TYPE) {
      next = list -> c.cdr;
      list -> c.cdr = prev;
      if (next -> type != NULL_TYPE) {
        prev = next -> c.car;
      }
      else {
        break;
      }
    }

    else {
      next = next -> c.cdr;
      if (prev -> type == NULL_TYPE) {
        break;
      }
      else {
        list = cons(prev, list);
        if (next -> type != NULL_TYPE) {
          prev = next -> c.car;
        }
        else {
          break;
        }
      }
    }
  }

  return list;
}

/* Function: car
 * --------------------
 *   Function to obtain the "car" value of the linked list, for less typing.
 *
 *   list: The linked list to grab the "car" of.
 *   returns: The "car" of the passed in linked list.
 */

Value *car(Value *list) {
  Value *val = list -> c.car;
  return val;
}

/* Function: cdr
 * --------------------
 *   Function to obtain the "cdr" value of the linked list, for less typing.
 *
 *   list: The linked list to grab the "cdr" of.
 *   returns: The "cdr" of the passed in linked list.
 */

Value *cdr(Value *list){
  Value *val = list -> c.cdr;
  return val;
}

/* Function: isNull
 * --------------------
 *   Function to check if the Value struct passed has type NULL_TYPE.
 *
 *   value: The Value struct to check if it is NULL_TYPE.
 *   returns: Either true, if it is of type NULL_TYPE, or false if it is not.
 */

bool isNull(Value *value) {
  Value *val = value;
  if (val -> type == NULL_TYPE) {
    return true;
  }
  else {
    return false;
  }
}

/* Function: length
 * --------------------
 *   Function to find the length of the linked list.
 *
 *   value: The Value struct to check the length of.
 *   returns: The length of the given Value struct.
 */

int length(Value *value){
  int len = 0;
  Value *cur = value;
  while (isNull(cur) != true) {
    len = len + 1;
    cur = cdr(cur);
  }
  return len;
}