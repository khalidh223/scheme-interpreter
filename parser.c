/* parser.c
 * Author: Khalid Hussain
 * --------------------
 * This program parse's an existing list of tokens from tokenizer.c, creating
 * the parse tree in accordance with the number of open/close parenthesis. It
 * can also display all Scheme expressions/atoms from a parse tree in printTree().
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "headers/linkedlist.h"
#include "headers/value.h"
#include "headers/talloc.h"
#include "headers/tokenizer.h"

/* Function: syntaxError
 * --------------------
 *   Prints "Syntax error" and texit's.
 */

void syntaxError(){
  printf("Syntax error\n");
  texit(1);
}

/* Function: parse
 * --------------------
 *   Takes a list of tokens representing a Scheme program, and creates a parse
 *   tree from these list of tokens.
 *
 *   tokens: The list of tokens to parse.
 *   returns: The parse tree from the list of tokens.
 */

Value *parse(Value *tokens){
  Value *tree = makeNull();
  Value *stack = makeNull();

  int depth = 0;

  Value *current = tokens;
  assert(current != NULL && "Error (parse): null pointer");

  while (current->type != NULL_TYPE) {
    if (car(current) -> type == OPEN_TYPE) {
      depth = depth + 1;
    }

    if (car(current) -> type == CLOSE_TYPE) {
      if(depth == 0) {
        syntaxError();
      }
      depth = depth - 1;
    }

    if (car(current) -> type != CLOSE_TYPE) {
      stack = cons(car(current), stack);
    }

    else {
      Value *tempList = makeNull();
      while (car(stack) -> type != OPEN_TYPE) {
        if (stack -> type == NULL_TYPE) {
          syntaxError();
        }
        tempList = cons(car(stack), tempList);
        stack = cdr(stack);
      }

      stack = cdr(stack);
      stack = cons(tempList, stack);
      tempList = makeNull();
    }

    if (depth == 0) {
      tree = cons(car(stack), tree);
    }
    current = cdr(current);
  }

  if (depth != 0) {
    if (depth < 0) {
      printf("Syntax error: too many close parentheses.\n");
    }
    if (depth > 0) {
      printf("Syntax error: not enough close parentheses.\n");
    }
  }

  return reverse(tree);
}

/* Function: printTree
 * --------------------
 *   Prints a parse tree to the console in a readable fashion, looking just
 *   like a Scheme expression & using parentheses to indicate subtrees.
 *
 *   tree: The parse tree to print.
 */

void printTree(Value *tree) {
  Value *cur = tree;

  if (cur -> type == CONS_TYPE) {
    printf("(");
    while (cur -> type != NULL_TYPE) {
      if(cur -> type == CONS_TYPE){
        if (car(cur) -> type == CONS_TYPE) {
          printTree(car(cur));
          cur = cdr(cur);
        }

        else if (car(cur) -> type == SYMBOL_TYPE) {
          printf("%s ", car(cur) -> s);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == INT_TYPE) {
          printf("%i ", car(cur) -> i);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == DOUBLE_TYPE) {
          printf("%f ", car(cur) -> d);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == STR_TYPE) {
          printf("%s ", car(cur) -> s);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == OPEN_TYPE) {
          printf("%s ", car(cur) -> s);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == CLOSE_TYPE) {
          printf("%s ", car(cur) -> s);
          cur = cdr(cur);
        }

        else if (car(cur) -> type == BOOL_TYPE) {
          printf("%s ", car(cur) -> s);
          cur = cdr(cur);
        }
        else if (car(cur) -> type == NULL_TYPE) {
          printf("() ");
          cur = cdr(cur);
        }

        if(cur -> type != CONS_TYPE && cur -> type != NULL_TYPE){
          printf(". ");
        }
      }
      else{
        if (cur -> type == CONS_TYPE) {
          printTree(car(cur));
          cur = cdr(cur);
        }

        else if (cur -> type == SYMBOL_TYPE) {
          printf("%s", cur -> s);
          break;
        }

        else if (cur -> type == INT_TYPE) {
          printf("%i", cur -> i);
          break;
        }

        else if (cur -> type == DOUBLE_TYPE) {
          printf("%f", cur -> d);
          break;
        }

        else if (cur -> type == STR_TYPE) {
          printf("%s", cur -> s);
          break;
        }

        else if (cur -> type == OPEN_TYPE) {
          printf("%s", cur -> s);
          break;
        }

        else if (cur -> type == CLOSE_TYPE) {
          printf("%s", cur -> s);
          break;
        }

        else if (cur -> type == BOOL_TYPE) {
          printf("%s", cur -> s);
          break;
        }
        else if (cur -> type == NULL_TYPE) {
          printf("()");
          break;
        }
      }
    }
  }

  printf(") ");
}