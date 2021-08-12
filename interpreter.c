/* interpreter.c
 * Author: Khalid Hussain
 * --------------------
 * This program is able to interpret Scheme code that has been first been tokenized
 * by tokenize.c, parsed by parse.c, and has had the parse tree fed into
 * interpret() below. It should evaluate the Scheme program's expressions,
 * and print the correct result of executing these expressions.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "headers/linkedlist.h"
#include "headers/value.h"
#include "headers/talloc.h"
#include "headers/tokenizer.h"
#include "headers/parser.h"
#include "headers/interpreter.h"

Frame *globalframe = NULL; /* Bindings pointers to definitions of Scheme primitive & regular functions*/

/* Function: interpret
 * --------------------
 *   Core part of the program that interprets the parsed Scheme expressions and prints
 *   the result of each expression.
 *
 *   tree: The tree of parsed Scheme expressiona, stored as a linked list; each
 *   Scheme expression is nested based on the occurence of parenthesis.
 */

void interpret(Value *tree) {
  Value *cur = tree;
  Value *result;
  globalframe = talloc(sizeof(Frame));
  globalframe -> bindings = makeNull();
  globalframe -> parent = talloc(sizeof(Frame));
  globalframe -> parent = NULL;

  /* Bind pointers to the functions of each of the following Scheme primitive functions to global frame */
  bind("null?", primitiveNull, globalframe);
  bind("cons", primitiveCons, globalframe);
  bind("car", primitiveCar, globalframe);
  bind("cdr", primitiveCdr, globalframe);
  bind("+", primitiveAdd, globalframe);
  bind("-", primitiveMinus, globalframe);
  bind("<", primitiveLessThan, globalframe);
  bind(">", primitiveGreaterThan, globalframe);
  bind("=", primitiveEqual, globalframe);
  bind("*", primitiveMultiply, globalframe);
  bind("/", primitiveDivide, globalframe);
  bind("modulo", primitiveModulo, globalframe);

  while (cur -> type != NULL_TYPE) {
    Frame *frame = talloc(sizeof(Frame));
    frame -> bindings = makeNull();
    frame -> parent = NULL;
    result = eval(car(cur), frame);

    if (result -> type == INT_TYPE) {
      printf("%i \n", result -> i);
    }
    else if (result -> type == STR_TYPE) {
      printf("%s \n", result -> s);
    }
    else if (result -> type == DOUBLE_TYPE) {
      printf("%f \n", result -> d);
    }
    else if (result -> type == BOOL_TYPE) {
      printf("%s \n", result -> s);
    }
    else if (result -> type == CONS_TYPE) {
      printTree(result);
      printf("\n");
    }
    else if (result -> type == VOID_TYPE){
      printf("");
    }
    else if (result -> type == CLOSURE_TYPE) {
      printf("#<procedure> \n");
    }
    else if (result -> type == NULL_TYPE) {
      printf("() \n");
    }
    cur = cdr(cur);
  }
}

/* Function: bind
 * --------------------
 *   Stores a binding that contains a pointer to the Scheme function definitions
 *   to the global frame.
 *
 *   name: The name of some Scheme primitive function, parsed from a Scheme file.
 *   function: Pointer to the function that will replicate the functionality
 *   of the Scheme function stated in "name".
 *   frame: The global frame that will store these function pointers as bindings.
 */

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    Value *var = talloc(sizeof(Value));
    var -> s = talloc(sizeof(char) * (strlen(name) + 1));
    strcpy(var -> s, name);
    var -> type = SYMBOL_TYPE;

    Value *pair = talloc(sizeof(Value));
    pair -> type = CONS_TYPE;
    pair -> c.car = var;
    pair -> c.cdr = value;
    frame->bindings = cons(pair, frame -> bindings);
}

/* Function: primitiveAdd
 * --------------------
 *   This function mirrors the functionality of '+' in Scheme.
 *
 *   args: List of some number of integer or decimal arguments that would have been passed into '+'
 *   returns: If any of the arguments are decimals, return a Value struct that stores
 *   the answer as a decimal in result -> d; else return a Value struct that stores
 *   the answer as an integer in result -> i.
 */

Value *primitiveAdd(Value *args) {
   int realFlag = 0;
   Value *result = talloc(sizeof(Value));
   Value *cur = args;
   while (cur -> type != NULL_TYPE) {
     if(cur -> c.car -> type == DOUBLE_TYPE){
       realFlag += 1;
       cur = cdr(cur);
     } else if(cur -> c.car -> type == INT_TYPE){
       cur = cdr(cur);
     } else {
       printf("Evaluation error: Arguments must be a INT/DOUBLE type. \n");
       texit(0);
     }
   }

   cur = args;
   if(realFlag == 0){
     result -> i = 0;

     while (cur -> type != NULL_TYPE) {
       result -> i = result -> i + car(cur) -> i;
       cur = cdr(cur);
     }
     result -> type = INT_TYPE;
     return result;
   } else {
     result -> d = 0;

     while (cur -> type != NULL_TYPE) {
       if(car(cur) -> type == DOUBLE_TYPE){
         result -> d = result -> d + car(cur) -> d;
         cur = cdr(cur);
       } else {
         double currentArg = (double) car(cur) -> i;
         result -> d = result -> d + currentArg;
         cur = cdr(cur);
       }
     }
    result -> type = DOUBLE_TYPE;
    return result;
   }
}

/* Function: primitiveCons
 * --------------------
 *   This function mirrors the functionality of 'cons' in Scheme.
 *
 *   args: List of two arguments to 'cons' together, either of which could also
 *   be another list.
 *   returns: A pair, where the c.car is the first argument and the c.cdr is the second,
 *   which mimics the resulting list/pair when cons'ing these arguments together.
 */

Value *primitiveCons(Value *args) {
  if (args -> type == NULL_TYPE) {
    printf("Evaluation error: No args given to cons. \n");
    texit(0);
  }
  else if (length(args) != 2) {
    printf("Evaluation error: Wrong number of args to cons. \n");
    texit(0);
  }

  Value *pair = talloc(sizeof(Value));
  pair -> type = CONS_TYPE;

  pair -> c.car = args -> c.car;
  pair -> c.cdr = args -> c.cdr -> c.car;

  return pair;
}

/* Function: primitiveNull
 * --------------------
 *   This function mirrors the functionality of 'null?' in Scheme.
 *
 *   args: List of one argument, which could be nested, to check if it is null.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *primitiveNull(Value *args) {
  if (length(args) != 1) {
    printf("Evaluation error: Wrong number of args to null?. \n");
    texit(0);
  }

  if (length(args) == 1 && args -> c.cdr -> type != NULL_TYPE) {
    printf("Evaluation error: Wrong number of args to null?. \n");
    texit(0);
  }

  Value *result = talloc(sizeof(Value));
  Value *cur = args;

  if (cur -> type == NULL_TYPE) {
    char *boolean = "#t";
    result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
    strcpy(result -> s,boolean);
    result -> type = BOOL_TYPE;
  }

  else {
      if (cur -> type == CONS_TYPE) {
        while (cur -> c.car -> type == CONS_TYPE) {
          if (cur -> c.cdr -> type == NULL_TYPE) {
            cur = cur -> c.car;
          }
          else {
            char *boolean = "#f";
            result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
            strcpy(result -> s,boolean);
            result -> type = BOOL_TYPE;
            break;
          }
        }
        if (cur -> c.car -> type == NULL_TYPE) {
          char *boolean = "#t";
          result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
          strcpy(result -> s,boolean);
          result -> type = BOOL_TYPE;
        }
        else {
          char *boolean = "#f";
          result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
          strcpy(result -> s,boolean);
          result -> type = BOOL_TYPE;
        }
      }
  }

  return result;
}

/* Function: primitiveCar
 * --------------------
 *   This function mirrors the functionality of 'car' in Scheme.
 *
 *   args: List of one argument, which could be nested, to grab the car of.
 *   returns: A Value struct whose type and result depends on what is found in the car.
 */

Value *primitiveCar(Value *args) {
  if (length(args) != 1) {
    printf("Evaluation error: Wrong number of args to car. \n");
    texit(0);
  }

  if (args -> type == CONS_TYPE) {
    if (args -> c.car -> type != CONS_TYPE) {
      printf("Evaluation error: Wrong number of args to car. \n");
      texit(0);
    }
  }

  Value *result = talloc(sizeof(Value));

  if (args -> c.car -> type == CONS_TYPE) {

    if (args -> c.car -> c.car -> type == CONS_TYPE) {
      result = args -> c.car -> c.car;
    }

    else if (args -> c.car -> c.car -> type == INT_TYPE) {
      result -> type = INT_TYPE;
      result -> i = args -> c.car -> c.car -> i;
    }

    else if (args -> c.car -> c.car -> type == DOUBLE_TYPE) {
      result -> type = DOUBLE_TYPE;
      result -> d = args -> c.car -> c.car -> d;
    }

    else if (args -> c.car -> c.car -> type == NULL_TYPE) {
      result = makeNull();
    }

    else {
      result -> type = STR_TYPE;
      result -> s = args -> c.car -> c.car -> s;
    }
  }

  else if (args -> c.car -> type == INT_TYPE) {
    result -> type = INT_TYPE;
    result -> i = args -> c.car -> i;
  }

  else if (args -> c.car -> type == DOUBLE_TYPE) {
    result -> type = DOUBLE_TYPE;
    result -> d = args -> c.car -> d;
  }

  else if (args -> c.car -> type == NULL_TYPE) {
    result = makeNull();
  }

  else {
    result -> type = STR_TYPE;
    result -> s = args -> c.car -> s;
  }
  return result;
}

/* Function: primitiveCdr
 * --------------------
 *   This function mirrors the functionality of 'cdr' in Scheme.
 *
 *   args: List of one argument, which could be nested, to grab the cdr of.
 *   returns: A Value struct whose type and result depends on what is found in the cdr.
 */

Value *primitiveCdr(Value *args) {
  if (length(args) != 1) {
    printf("Evaluation error: Wrong number of args to cdr. \n");
    texit(0);
  }

  Value *pair = talloc(sizeof(Value));

  if (args -> c.car -> type == CONS_TYPE) {

    if (args -> c.car -> c.cdr -> type == CONS_TYPE) {
      pair -> type = CONS_TYPE;
      pair -> c.car = talloc(sizeof(Value));
      pair -> c.car = args -> c.car -> c.cdr -> c.car;
      pair -> c.cdr = args -> c.car -> c.cdr -> c.cdr;
    }

    else if (args -> c.car -> c.cdr -> type == INT_TYPE) {
      pair -> type = INT_TYPE;
      pair -> i = args -> c.car -> c.cdr -> i;
    }

    else if (args -> c.car -> c.cdr -> type == DOUBLE_TYPE) {
      pair -> type = DOUBLE_TYPE;
      pair -> d = args -> c.car -> c.cdr -> d;
    }

    else if (args -> c.car -> c.cdr -> type == NULL_TYPE) {
      pair -> type = NULL_TYPE;
    }

    else{
      pair -> type = STR_TYPE;
      pair -> s = args -> c.car -> c.cdr -> s;
    }
  }

  else if (args -> c.car -> type == INT_TYPE) {
    pair -> type = INT_TYPE;
    pair -> i = args -> c.cdr -> i;
  }

  else if (args -> c.car -> type == DOUBLE_TYPE) {
    pair -> type = DOUBLE_TYPE;
    pair -> d = args -> c.cdr -> d;
  }

  else if (args -> c.car -> type == NULL_TYPE) {
    pair -> type = NULL_TYPE;
  }

  else {
    pair -> type = STR_TYPE;
    pair -> s = args -> c.cdr -> s;
  }

  return pair;
}

/* Function: primitiveMinus
 * --------------------
 *   This function mirrors the functionality of '-' in Scheme.
 *
 *   args: List of some number of integer or decimal arguments that would have been passed into '-'.
 *   returns: If any of the arguments are decimal, return a Value struct that stores
 *   the answer as a decimal in result -> d; else return a Value struct that stores
 *   the answer as an integer in result -> i.
 */

Value *primitiveMinus(Value *args) {
   if(length(args) > 2){
     printf("Evaluation error: '-' can only take in two arguments. \n");
     texit(0);
   }

   int realFlag = 0;
   Value *result = talloc(sizeof(Value));
   Value *cur = args;
   while (cur -> type != NULL_TYPE) {
     if(cur -> c.car -> type == DOUBLE_TYPE){
       realFlag += 1;
       cur = cdr(cur);
     } else if(cur -> c.car -> type == INT_TYPE){
       cur = cdr(cur);
     } else {
       printf("Evaluation error: Arguments must be a INT/DOUBLE type. \n");
       texit(0);
     }
   }

   cur = args;
   if(realFlag == 0){
     result -> i = (car(cur) -> i) - (car(cdr(cur)) -> i);

     result -> type = INT_TYPE;
     return result;
   }
   else {
     result -> d = 0;

     if(car(cur) -> type == DOUBLE_TYPE){
       if(car(cdr(cur)) -> type == DOUBLE_TYPE) {
         result -> d = (car(cur) -> d) - (car(cdr(cur)) -> d);
       } else{
         double currentArg = (double) car(cdr(cur)) -> i;
         result -> d = (car(cur) -> d) - currentArg;
       }
      } else {
        double currentArg = (double) car(cur) -> i;

        result -> d = currentArg - (car(cdr(cur)) -> d);
      }

    result -> type = DOUBLE_TYPE;
    return result;
   }
}

/* Function: primitiveLessThan
 * --------------------
 *   This function mirrors the functionality of '<' in Scheme.
 *
 *   args: List of two integers to compare.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *primitiveLessThan(Value *args) {
   Value *cur = args;
   Value *result = talloc(sizeof(Value));
   if(length(args) > 2){
     printf("Evaluation error: '<' can only take in two arguments. \n");
     texit(0);
   }
   if (car(cur) -> i < cdr(cur) -> c.car -> i) {
     char *boolean = "#t";
     result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
     strcpy(result -> s,boolean);
     result -> type = BOOL_TYPE;
   }
   else {
     char *boolean = "#f";
     result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
     strcpy(result -> s,boolean);
     result -> type = BOOL_TYPE;
   }
   return result;
}

/* Function: primitiveGreaterThan
 * --------------------
 *   This function mirrors the functionality of '>' in Scheme.
 *
 *   args: List of two integers to compare.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *primitiveGreaterThan(Value *args) {
   Value *cur = args;
   Value *result = talloc(sizeof(Value));
   if(length(args) > 2){
     printf("Evaluation error: '>' can only take in two arguments. \n");
     texit(0);
   }

   if (car(cur) -> i > cdr(cur) -> c.car -> i) {
     char *boolean = "#t";
     result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
     strcpy(result -> s,boolean);
     result -> type = BOOL_TYPE;
   }
   else {
     char *boolean = "#f";
     result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
     strcpy(result -> s,boolean);
     result -> type = BOOL_TYPE;
   }
   return result;
}

/* Function: primitiveEqual
 * --------------------
 *   This function mirrors the functionality of '=' in Scheme.
 *
 *   args: List of two numbers, either decimals or integers, to compare.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *primitiveEqual(Value *args) {
  if (length(args) > 2) {
     printf("Evaluation error: '=' can only take in two arguments. \n");
     texit(0);
   }

   int realFlag = 0;
   Value *result = talloc(sizeof(Value));
   Value *cur = args;
   while (cur -> type != NULL_TYPE) {
     if (cur -> c.car -> type == DOUBLE_TYPE) {
       realFlag += 1;
       cur = cdr(cur);
     } else if (cur -> c.car -> type == INT_TYPE) {
       cur = cdr(cur);
     } else {
       printf("Evaluation error: Arguments must be a INT/DOUBLE type. \n");
       texit(0);
     }
   }

   cur = args;

   if (realFlag == 0) {
     int param1 = car(cur) -> i;
     int param2 = car(cdr(cur)) -> i;

     if (param1 == param2) {
       char *boolean = "#t";
       result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
       strcpy(result -> s,boolean);
       result -> type = BOOL_TYPE;
     } else {
       char *boolean = "#f";
       result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
       strcpy(result -> s,boolean);
       result -> type = BOOL_TYPE;
     }
   }

   else {
     double param1 = 0;
     double param2 = 0;

     if (car(cur) -> type == DOUBLE_TYPE) {
       param1 = car(cur) -> d;

       if(car(cdr(cur)) -> type == DOUBLE_TYPE) {
         param2 = car(cdr(cur)) -> d;
       } else{
         param2 = (double) car(cdr(cur)) -> i;
       }

      } else {
        param1 = (double) car(cur) -> i;
        param2 = car(cdr(cur)) -> d;
      }

      if(param1 == param2){
        char *boolean = "#t";
        result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
        strcpy(result -> s,boolean);
        result -> type = BOOL_TYPE;
      }
      else {
        char *boolean = "#f";
        result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
        strcpy(result -> s,boolean);
        result -> type = BOOL_TYPE;
      }
   }

   return result;
}

/* Function: primitiveMultiply
 * --------------------
 *   This function mirrors the functionality of '*' (the multiply operator) in Scheme.
 *
 *   args: List of some number of integer or decimal arguments that would have been passed into '*'.
 *   returns: If any of the arguments are decimals, return a Value struct that stores
 *   the answer as a decimal in result -> d; else return a Value struct that stores
 *   the answer as an integer in result -> i.
 */

//This method is a primitive method that functions as the 'multiply' method in scheme, where it multiples two ints/double that is passed as parameters to the 'multiply' fuction. If the parameters are neither int/double type, it will be an error.
Value *primitiveMultiply(Value *args) {
   int realFlag = 0;
   Value *result = talloc(sizeof(Value));
   Value *cur = args;
   while (cur -> type != NULL_TYPE) {
     if(cur -> c.car -> type == DOUBLE_TYPE){
       realFlag += 1;
       cur = cdr(cur);
     } else if(cur -> c.car -> type == INT_TYPE){
       cur = cdr(cur);
     } else {
       printf("Evaluation error: Arguments must be an INT/DOUBLE type. \n");
       texit(0);
     }
   }

   cur = args;

   if (realFlag == 0) {
     result -> i = 1;

     while (cur -> type != NULL_TYPE) {
       result -> i = result -> i * car(cur) -> i;
       cur = cdr(cur);
     }
     result -> type = INT_TYPE;
     return result;
   }

   else {
     result -> d = 1;

     while (cur -> type != NULL_TYPE) {
       if(car(cur) -> type == DOUBLE_TYPE) {
         result -> d = result -> d * car(cur) -> d;
         cur = cdr(cur);
       } else {
         double currentArg = (double) car(cur) -> i;
         result -> d = result -> d * currentArg;
         cur = cdr(cur);
       }
     }
    result -> type = DOUBLE_TYPE;
    return result;
   }
}

/* Function: primitiveDivide
 * --------------------
 *   This function mirrors the functionality of '/' in Scheme.
 *
 *   args: List of some number of integer or decimal arguments that would have been passed into '/'.
 *   returns: If any of the arguments are decimals, return a Value struct that stores
 *   the answer as a decimal in result -> d; else return a Value struct that stores
 *   the answer as an integer in result -> i.
 */

Value *primitiveDivide(Value *args) {
   if (length(args) > 2) {
     printf("Evaluation error: '/' can only take in two arguments. \n");
     texit(0);
   }

   int realFlag = 0;
   Value *result = talloc(sizeof(Value));
   Value *cur = args;

   while (cur -> type != NULL_TYPE) {
     if (cur -> c.car -> type == DOUBLE_TYPE) {
       realFlag += 1;
       cur = cdr(cur);
     } else if (cur -> c.car -> type == INT_TYPE) {
       cur = cdr(cur);
     } else {
       printf("Evaluation error: Arguments must be a INT/DOUBLE type. \n");
       texit(0);
     }
   }

   cur = args;

   if (realFlag == 0) {
     int param1 = car(cur) -> i;
     int param2 = car(cdr(cur)) -> i;

      if (param1 % param2 == 0) {
        result -> i = param1 / param2;
        result -> type = INT_TYPE;
        return result;
      } else {
        result -> d = (double) param1 / (double) param2;
        result -> type = DOUBLE_TYPE;
        return result;
      }
   }

   else {
     double param1 = 0;
     double param2 = 0;

     if (car(cur) -> type == DOUBLE_TYPE) {
       param1 = car(cur) -> d;
       if (car(cdr(cur)) -> type == DOUBLE_TYPE) {
         param2 = car(cdr(cur)) -> d;
       }
       param2 = (double) car(cdr(cur)) -> i;
     }

     result -> d = param1 / param2;
     result -> type = DOUBLE_TYPE;
     return result;
   }
}

/* Function: primitiveModulo
 * --------------------
 *   This function mirrors the functionality of 'modulo' in Scheme.
 *
 *   args: List of two integers to take the modulo of.
 *   returns: A INT_TYPE Value struct that stores the integer result in result -> i.
 */

Value *primitiveModulo(Value *args) {
   if (length(args) > 2) {
     printf("Evaluation error: 'modulo' can only take in two arguments. \n");
     texit(0);
   }

   Value *result = talloc(sizeof(Value));
   Value *cur = args;

   while (cur -> type != NULL_TYPE) {
     if(cur -> c.car -> type != INT_TYPE){
       printf("Evaluation error: Arguments must be a INT type. \n");
       texit(0);
     }

     cur = cdr(cur);
   }

   cur = args;

   int param1 = car(cur) -> i;
   int param2 = car(cdr(cur)) -> i;

   result -> i = param1 % param2;
   result -> type = INT_TYPE;
   return result;
}

/* Function: evalIf
 * --------------------
 *   This function mirrors the functionality of 'if' in Scheme. It evaluates the
 *   arguments of the 'if' expression, and returns either '#t' or '#f'.
 *
 *   args: List of arguments to evaluate.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *evalIf(Value *args, Frame *frame) {
  if (length(args) < 3) {
    printf("Evaluation error: if has fewer than 3 arguments. \n");
    texit(0);
  }
  Value *result_test;
  Value *result_trueExpr = makeNull();
  Value *result_falseExpr= makeNull();
  result_test = eval(car(args), frame);
  if (!strcmp(result_test -> s, "#t")) {
    result_trueExpr = eval(cdr(args) -> c.car, frame);
  }
  else if (!strcmp(result_test -> s, "#f")) {
    result_falseExpr = eval(cdr(args) -> c.cdr -> c.car, frame);
  }
  if (result_trueExpr -> type != NULL_TYPE) {
    return result_trueExpr;
  }
  else {
    return result_falseExpr;
  }
}

/* Function: evalLet
 * --------------------
 *   This function mirrors the functionality of a "let" expression in Scheme. It
 *   does so by creating a new frame whose parent is the frame passed in, then
 *   stores each of the arguments of the statement as bindings within the new frame. They
 *   are bound to the their own corresponding, evaluated expressions. Lastly, it
 *   evaluates the body of the let expression in this new frame.
 *
 *   args: The Value struct arguments of the let expression
 *   frame: The current Frame struct of the interpreter
 *   returns: Result of evaluating the body of the let expression in the new frame.
 */

Value *evalLet(Value *args, Frame *frame) {
  if (car(args) -> type != NULL_TYPE && car(args) -> type != CONS_TYPE) {
    printf("Evaluation error: bad form in let \n");
    texit(0);
  }

  Value *cur = args;
  Frame *newframe = talloc(sizeof(Frame));
  newframe -> parent = frame;
  Value *bindings = makeNull();
  newframe -> bindings = bindings;
  Value *result;

  Value *expressions = car(cur);
  while (expressions -> type != NULL_TYPE) {
    Value *var = talloc(sizeof(Value));
    if (expressions -> c.car -> type == NULL_TYPE) {
      printf("Evaluation error: null binding in let. \n");
      texit(0);
    }

    else if (expressions -> c.car -> type != NULL_TYPE && expressions -> c.car -> type != CONS_TYPE) {
      printf("Evaluation error: bad form in let \n");
      texit(0);
    }

    else if (expressions -> c.car -> type ==  CONS_TYPE) {
      if (expressions -> c.car -> c.car -> type != SYMBOL_TYPE) {
        printf("Evaluation error: left side of a let pair doesn't have a variable. \n");
        texit(0);
      }
    }

    char *text = expressions -> c.car -> c.car -> s;

    if (newframe -> bindings -> type != NULL_TYPE) {
      Value *curbinding = newframe -> bindings;
      Value *checkduplicate = makeNull();
      while (curbinding -> type != NULL_TYPE) {
        if (!strcmp(curbinding->c.car->c.car -> s, expressions -> c.car -> c.car -> s)) {
          checkduplicate = curbinding->c.car->c.cdr;
          break;
        }
        else {
          curbinding = curbinding -> c.cdr;
        }
      }
      if (checkduplicate -> type != NULL_TYPE) {
        printf("Evaluation error: duplicate variable in let. \n");
        texit(0);
      }
    }

    var -> s = talloc(sizeof(char) * (strlen(text) + 1));
    strcpy(var -> s,text);
    var -> type = STR_TYPE;
    Value *val = talloc(sizeof(Value));
    val = eval(expressions -> c.car -> c.cdr -> c.car, frame);
    if (val -> type == CLOSURE_TYPE || val -> type == UNSPECIFIED_TYPE) {
      printf("Evaluation error: Unbound variable %s in let. \n", var -> s);
      texit(0);
    }

    Value *pair = talloc(sizeof(Value));
    pair -> type = CONS_TYPE;
    pair -> c.car = var;
    pair -> c.cdr = val;
    bindings = cons(pair, bindings);
    newframe -> bindings = bindings;

    expressions = cdr(expressions);
  }

  Value *curbody = cdr(args);
  while (curbody -> type != NULL_TYPE) {
    result = eval(car(curbody), newframe);
    curbody = cdr(curbody);
  }
  return result;
}

/* Function: evalLetStar
 * --------------------
 *   This function mirrors the functionality of a "let*" expression in Scheme. It
 *   does so by creating a new Frame struct for every variable within the expression, with
 *   their parent's linking to the previously created frame; the first frame that
 *   is created has the frame that was passed into the function become its parent.
 *   Each new frame created has only one binding, the variable it holds bound to
 *   its corresponding, evaluated expression. Lastly, it evaluates the body of
 *   the let* expression by starting from the last frame created, and bubbling up
 *   the chain of frames until it can find the correct variable to evaluate.
 *
 *   args: The Value struct arguments of the let* expression
 *   frame: The current Frame struct of the interpreter
 *   returns: Result of evaluating the body of the let expression.
 */

Value *evalLetStar(Value *args, Frame *frame) {
  if (car(args) -> type != NULL_TYPE && car(args) -> type != CONS_TYPE) {
    printf("Evaluation error: bad form in let \n");
    texit(0);
  }

  if (args -> type == NULL_TYPE) {
    printf("Evaluation error: no args following the bindings in let*. \n");
    texit(0);
  }

  Value *expressions = car(args);
  Frame *newframe;
  Value *result;
  Frame *prevframe = frame;

  while (expressions -> type != NULL_TYPE) {
    Value *var = talloc(sizeof(Value));
    if (expressions -> c.car -> type == NULL_TYPE) {
      printf("Evaluation error: null binding in let*. \n");
      texit(0);
    }

    else if (expressions -> c.car -> type != NULL_TYPE && expressions -> c.car -> type != CONS_TYPE) {
      printf("Evaluation error: bad form in let*. \n");
      texit(0);
    }

    else if (expressions -> c.car -> type ==  CONS_TYPE) {
      if (expressions -> c.car -> c.car -> type != SYMBOL_TYPE) {
        printf("Evaluation error: left side of a let* pair doesn't have a variable. \n");
        texit(0);
      }
    }

    newframe = talloc(sizeof(Frame));
    newframe -> bindings = makeNull();
    newframe -> parent = prevframe;
    prevframe = newframe;

    char *text = expressions -> c.car -> c.car -> s;

    if (newframe -> bindings -> type != NULL_TYPE) {
      Value *curbinding = newframe -> bindings;
      Value *checkduplicate = makeNull();
      while (curbinding -> type != NULL_TYPE) {
        if (!strcmp(curbinding->c.car->c.car -> s, expressions -> c.car -> c.car -> s)) {
          checkduplicate = curbinding->c.car->c.cdr;
          break;
        }
        else {
          curbinding = curbinding -> c.cdr;
        }
      }
      if (checkduplicate -> type != NULL_TYPE) {
        printf("Evaluation error: duplicate variable in let* \n");
        texit(0);
      }
    }

    var -> s = talloc(sizeof(char) * (strlen(text) + 1));
    strcpy(var -> s,text);
    var -> type = STR_TYPE;
    Value *val = talloc(sizeof(Value));
    val = eval(expressions -> c.car -> c.cdr -> c.car, newframe);
    if (val -> type == CLOSURE_TYPE || val -> type == UNSPECIFIED_TYPE) {
      printf("Evaluation error: Unbound variable %s in let*. \n", var -> s);
      texit(0);
    }

    Value *pair = talloc(sizeof(Value));
    pair -> type = CONS_TYPE;
    pair -> c.car = var;
    pair -> c.cdr = val;
    newframe -> bindings = cons(pair, newframe -> bindings);

    expressions = cdr(expressions);
  }
  Value *curbody = cdr(args);
  while (curbody -> type != NULL_TYPE) {
    result = eval(car(curbody), newframe);
    curbody = cdr(curbody);
  }
  return result;
}

/* Function: evalLetRec
 * --------------------
 *   This function mirrors the functionality of the "letrec" expression in Scheme. It
 *   does so by creating a new frame whose parent is the frame passed into the
 *   function, then creates a binding for each of the variables of the expression's argument.
 *   Each variable is bound to a Value struct of UNSPECIFIED_TYPE; after evaluating
 *   each value for each variable in this new frame, the evaluations themselves
 *   replace the original UNSPECIFIED_TYPE binding for each corresponding variable.
 *   Lastly, it evaluates the body of the let expression in this new frame.
 *
 *   args: The Value struct arguments of the letrec expression
 *   frame: The current Frame struct of the interpreter
 *   returns: Result of evaluating the body of the let expression.
 */

Value *evalLetRec(Value *args, Frame *frame) {
  Value *bindings = car(args);
  Frame *newframe = talloc(sizeof(Frame));
  newframe -> parent = frame;
  newframe -> bindings = makeNull();
  Value *result;

  while (bindings -> type != NULL_TYPE) {
    Value *pair = talloc(sizeof(Value));
    pair -> type = CONS_TYPE;
    pair -> c.car = bindings -> c.car -> c.car;
    pair -> c.cdr = talloc(sizeof(Value));
    pair -> c.cdr -> type = UNSPECIFIED_TYPE;
    newframe -> bindings = cons(pair, newframe -> bindings);
    bindings = cdr(bindings);
  }

  newframe -> bindings = reverse(newframe -> bindings);
  bindings = car(args);
  Value *evaluatedvalues = makeNull();

  // First evaluate each value of each variable in newframe -> bindings, within
  // this newframe of bindings with UNSPECIFIED_TYPE's
  while (bindings -> type != NULL_TYPE) {
    evaluatedvalues = cons(eval(bindings -> c.car -> c.cdr -> c.car, newframe), evaluatedvalues);
    if (eval(bindings -> c.car -> c.cdr -> c.car, newframe) -> type == UNSPECIFIED_TYPE) {
      printf("Evaluation error: Evaluated an UNSPECIFIED_TYPE in letrec. \n");
      texit(0);
    }
    bindings = cdr(bindings);
  }

  // NOW assign each evaluated value to its corresponding variable - don't forget
  // to reverse evaluatedvalues first to match the order
  evaluatedvalues = reverse(evaluatedvalues);
  Value *cur_evaluated_value = evaluatedvalues;
  Value *cur_newframe_binding = newframe -> bindings; //variables
  while (cur_newframe_binding -> type != NULL_TYPE) {
    car(cur_newframe_binding) -> c.cdr = cur_evaluated_value -> c.car; //set variable of car(cur_newframe_binding) -> c.car to corresponding evaluated value
    cur_newframe_binding = cdr(cur_newframe_binding);
    cur_evaluated_value = cdr(cur_evaluated_value);
  }

  Value *curbody = cdr(args);
  while (curbody -> type != NULL_TYPE) {
    result = eval(car(curbody), newframe);
    curbody = cdr(curbody);
  }
  return result;
}

/* Function: evalSet
 * --------------------
 *   This function mirrors the functionality of the "set!" expression in Scheme. It
 *   does so by looking up each of the variables of the expression's argument. It
 *   first searches the bindings of the frame passed into the function, and if
 *   the variable is not found, bubbles up to each of the linked parent frames.
 *   Lastly, it either changes the value bound to the variable that is being
 *   searched for to the value provided in args, or prints an error if the variable
 *   is not found in any frame.
 *
 *   args: The Value struct arguments of the set! expression
 *   frame: The current Frame struct of the interpreter
 *   returns: A VOID_TYPE Value struct.
 */

Value *evalSet(Value *args, Frame *frame) {
  Frame *cur = frame;
  int globalframeflag = 0;
  Value *value = makeNull();
  Value *curbinding = makeNull();

  while (cur != NULL) {
    curbinding = cur -> bindings;

    if (curbinding -> type == NULL_TYPE && globalframeflag == 1) {
      printf("Evaluation error: symbol '%s' not found when trying to set. \n", args -> c.car -> s);
      texit(0);
    }

    while (curbinding -> type != NULL_TYPE) {
      if (!strcmp(curbinding->c.car->c.car -> s, args -> c.car -> s)) {
        curbinding -> c.car -> c.cdr = eval(args -> c.cdr -> c.car, frame);
        value = curbinding->c.car->c.cdr;
        break;
      }
      else {
        curbinding = curbinding -> c.cdr;
      }
    }

    if (value -> type == NULL_TYPE) {
      if (curbinding -> type != NULL_TYPE) {
        if (curbinding -> c.car -> c.cdr -> type == NULL_TYPE) {
          break;
        }
      }

      cur = cur -> parent;
      if (cur == NULL && globalframeflag == 0) {
        cur = globalframe;
        globalframeflag = 1;
      }

      else if (cur == NULL && globalframeflag == 1) {
        break;
      }
    }

    else {
      break;
    }
  }

  if (value -> type == NULL_TYPE && curbinding -> type == NULL_TYPE) {
    printf("Evaluation error: symbol '%s' not found. \n", args -> c.car -> s);
    texit(0);
  }
  Value *result = talloc(sizeof(Value));
  result -> type = VOID_TYPE;
  return result;
}

/* Function: evalBegin
 * --------------------
 *   This function mirrors the functionality of the "begin" expression in Scheme. It
 *   does so by evaluating each of the arguments values within the current frame,
 *   and only returns the last evaluated value.
 *
 *   args: The list of arguments for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: Either the last evaluated result of the function, or if args is
 *   NULL_TYPE, returns a VOID_TYPE.
 */

Value *evalBegin(Value *args, Frame *frame) {
  Value *result;
  Value *cur = args;
  if (cur -> type == NULL_TYPE) {
    Value *result = talloc(sizeof(Value));
    result -> type = VOID_TYPE;
  }
  else {
    while (cur -> type != NULL_TYPE) {
      result = eval(car(cur), frame);
      cur = cdr(cur);
    }
  }
  return result;
}

/* Function: evalAnd
 * --------------------
 *   This function mirrors the functionality of the "and" expression in Scheme. It
 *   does so by first evaluating every expression within the argument, and returns
 *   '#f' if there any of them evaluates to '#f'. If not, the function
 *   will return '#t'.
 *
 *   args: The list of expressions within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *evalAnd (Value *args, Frame *frame) {
  Value *cur = args;
  Value *result = makeNull();

  if (length(args) == 0) {
    char *boolean = "#t";
    result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
    strcpy(result -> s,boolean);
    result -> type = BOOL_TYPE;
    return result;
  }

  while (cur -> type != NULL_TYPE) {

    Value *curExpr = eval(car(cur), frame);

    if (curExpr -> type == INT_TYPE) {
      result -> i = curExpr -> i;
      result -> type = INT_TYPE;
    }

    else if (curExpr -> type == DOUBLE_TYPE) {
      result -> d = curExpr -> d;
      result -> type = DOUBLE_TYPE;
    }

    else {
      if (!strcmp(curExpr -> s, "#f")) {
        char *boolean = "#f";
        result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
        strcpy(result -> s,boolean);
        result -> type = BOOL_TYPE;
        return result;
      }

      char *boolean = "#t";
      result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
      strcpy(result -> s,boolean);
      result -> type = BOOL_TYPE;
    }

    cur = cdr(cur);
  }

  return result;
}

/* Function: evalOr
 * --------------------
 *   This function mirrors the functionality of the "or" expression in Scheme. It
 *   does so by first evaluating every expression within the argument, and returns
 *   '#t' if there any of them evaluates to '#t'. If not, the function
 *   will return '#f'.
 *
 *   args: The list of expressions within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A BOOL_TYPE Value struct that stores either "#t" or "#f" in result -> s.
 */

Value *evalOr(Value *args, Frame *frame) {
  Value *cur = args;
  Value *result = makeNull();

  if(length(args) == 0){
    char *boolean = "#f";
    result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
    strcpy(result -> s,boolean);
    result -> type = BOOL_TYPE;
    return result;
  }

  while(cur -> type != NULL_TYPE){

    Value *curExpr = eval(car(cur), frame);

    if(curExpr -> type == INT_TYPE){
      result -> i = curExpr -> i;
      result -> type = INT_TYPE;
    }

    else if(curExpr -> type == DOUBLE_TYPE){
      result -> d = curExpr -> d;
      result -> type = DOUBLE_TYPE;
    }

    else{
      if(!strcmp(curExpr -> s, "#t")){
        char *boolean = "#t";
        result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
        strcpy(result -> s,boolean);
        result -> type = BOOL_TYPE;

        return result;
      }

      char *boolean = "#f";
      result -> s = talloc(sizeof(char) * (strlen(boolean) + 1));
      strcpy(result -> s,boolean);
      result -> type = BOOL_TYPE;
    }

    cur = cdr(cur);
  }

  return result;
}

/* Function: lookUpSymbol
 * --------------------
 *   This function looks up unknown symbols within the bindings of the current
 *   frame that is passed in. If it is not found, the search goes through the
 *   bindings of every frame that is the parent of every other frame, until it
 *   reaches the global frame. Once it is found, the value that the symbol is
 *   bound to is returned; it if is not found in any frame, the function returns an error.
 *
 *   expr: The symbol to search for.
 *   frame: The current Frame struct of the interpreter.
 *   returns: The value that the expression was bound to, or an error if it is not found.
 */

Value *lookUpSymbol(Value *expr, Frame *frame) {
  Frame *cur = frame;
  int globalframeflag = 0;
  Value *value = makeNull();
  Value *curbinding = makeNull();
  while (cur != NULL) {
    curbinding = cur -> bindings;
    if (curbinding -> type == NULL_TYPE && globalframeflag == 1) {
      printf("Evaluation error: symbol '%s' not found. \n", expr -> s);
      texit(0);
    }

    while (curbinding -> type != NULL_TYPE) {
      if (!strcmp(curbinding->c.car->c.car -> s, expr -> s)) {
        value = curbinding->c.car->c.cdr;
        break;
      }
      else {
        curbinding = curbinding -> c.cdr;
      }
    }

    if (value -> type == NULL_TYPE) {
      if (curbinding -> type != NULL_TYPE) {
        if (curbinding -> c.car -> c.cdr -> type == NULL_TYPE) {
          break;
        }
      }
      cur = cur -> parent;
      if (cur == NULL && globalframeflag == 0) {
        cur = globalframe;
        globalframeflag = 1;
      }
      else if (cur == NULL && globalframeflag == 1) {
        break;
      }
    }

    else {
      break;
    }
  }

  if (value -> type == NULL_TYPE && curbinding -> type == NULL_TYPE) {
    printf("Evaluation error: symbol '%s' not found. \n", expr -> s);
    texit(0);
  }

  return value;
}

/* Function: evalCond
 * --------------------
 *   This function mirrors the functionality of the "cond" expression in Scheme. It
 *   does so by evaluating each test case within the argument. It returns the evaluated
 *   value associated with the first expression that evaluates to true. If no expressions
 *   contain a value that returns true, it returns the evaluated expression that
 *   is tied to "else" in the argument.
 *
 *   args: The list of expressions within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A BOOL_TYPE Value struct that stores "#t", or the evaluated expression
 *   tied to "else".
 */

Value *evalCond(Value *args, Frame *frame) {
  Value *cur = args;
  Value *result;
  while (cur -> type != NULL_TYPE) {
    if (cur -> c.car -> c.car -> type == SYMBOL_TYPE) {
      if (!strcmp(cur -> c.car -> c.car -> s, "else")) {
        result = eval(cur -> c.car -> c.cdr -> c.car, frame);
        break;
      }
    }
    else {
      Value *boolean = eval(cur -> c.car -> c.car, frame);
      if (!strcmp(boolean -> s, "#t")) {
        result = eval(cur -> c.car -> c.cdr -> c.car, frame);
        break;
      }
      else if (!strcmp(boolean -> s, "#f")) {
        result = eval(cur -> c.car -> c.cdr -> c.car, frame);
        cur = cdr(cur);
      }
    }
  }

  return result;
}

/* Function: evalQuote
 * --------------------
 *   This function mirrors the functionality of the "quote" expression in Scheme. It
 *   does so by evaluating the arguments of the expression and simply returning
 *   the "car" of the list of arguments.
 *
 *   args: The list of expressions within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: The "car" of the list of arguments
 */

Value *evalQuote(Value *args, Frame *frame) {
  if (length(args) > 1) {
    printf("Evaluation error: multiple arguments to quote \n");
    texit(0);
  }
  else if (args -> type == NULL_TYPE) {
    printf("Evaluation error \n");
    texit(0);
  }
  return car(args);
}

/* Function: evalDefine
 * --------------------
 *   This function mirrors the functionality of the "define" expression in Scheme. It
 *   does so by storing the variables of the argument within the bindings of the
 *   global frame, binding it to the evaluated values corresponding to these variables.
 *
 *   args: The list of expressions within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A VOID_TYPE Value struct.
 */

Value *evalDefine(Value *args, Frame *frame) {
  if (args -> type == NULL_TYPE) {
    printf("Evaluation error: no args following define. \n");
    texit(0);
  }
  Value *var = talloc(sizeof(Value));
  if (args -> c.car -> type != SYMBOL_TYPE) {
    printf("Evaluation error: define must bind to a symbol. \n");
    texit(0);
  }
  char *text = args -> c.car -> s;
  var -> s = talloc(sizeof(char) * (strlen(text) + 1));
  strcpy(var -> s,text);
  var -> type = STR_TYPE;
  Value *val = talloc(sizeof(Value));
  if (args -> c.cdr -> type == NULL_TYPE) {
    printf("Evaluation error: no value following the symbol in define. \n");
    texit(0);
  }
  val = eval(args -> c.cdr -> c.car, frame);

  Value *pair = talloc(sizeof(Value));
  pair -> type = CONS_TYPE;
  pair -> c.car = var;
  pair -> c.cdr = val;
  frame -> bindings = cons(pair, frame -> bindings);

  Value *result = talloc(sizeof(Value));
  result -> type = VOID_TYPE;
  return result;
}

/* Function: evalLambda
 * --------------------
 *   This function mirrors the functionality of the "lambda" expression in Scheme. It
 *   does so by creating a CLOSURE_TYPE Value struct to store the symbols and body
 *   within it.
 *
 *   args: The list of symbols and function body within the argument for the function.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A CLOSURE_TYPE Value struct.
 */

Value *evalLambda(Value *args, Frame *frame) {
  if (args -> type == NULL_TYPE) {
    printf("Evaluation error: no args following lambda. \n");
    texit(0);
  }

  Value *params = car(args);
  Value *closure = talloc(sizeof(Value));
  closure -> cl.paramNames = makeNull();
  closure -> type = CLOSURE_TYPE;
  while (params -> type != NULL_TYPE) {
    if (car(params) -> type != SYMBOL_TYPE) {
      printf("Evaluation error: formal parameters for lambda must be symbols. \n");
      texit(0);
    }
    closure -> cl.paramNames = cons(car(params), closure -> cl.paramNames);
    params = cdr(params);
  }

  closure -> cl.paramNames = reverse(closure -> cl.paramNames);
  Value *current = closure -> cl.paramNames;

  while (current -> type != NULL_TYPE) {
    Value *var_to_compare = car(current);
    Value *next_val = cdr(current);
    while (next_val -> type != NULL_TYPE) {
      if (!strcmp(var_to_compare -> s, car(next_val) -> s)) {
        printf("Evaluation error: duplicate identifier in lambda. \n");
        texit(0);
      }
      else {
        next_val = cdr(next_val);
      }
    }
    current = cdr(current);
  }

  Value *body = cdr(args);
  if (body -> type == NULL_TYPE) {
    printf("Evaluation error: no code in lambda following parameters. \n");
    texit(0);
  }

  closure -> cl.functionCode = body;
  Frame *env = talloc(sizeof(Frame));
  env = frame;
  closure -> cl.frame = env;
  return closure;
}

/* Function: evalEach
 * --------------------
 *   This function evaluates each of the arguments being passed into it, returning
 *   a list of the evaluated arguments.
 *
 *   args: The list of arguments to evaluate.
 *   frame: The current Frame struct of the interpreter.
 *   returns: A Value struct list of evaluated arguments.
 */

Value *evalEach(Value *args, Frame *frame) {
  Value *cur = args;
  Value *result = makeNull();
  while (cur -> type != NULL_TYPE) {
    result = cons(eval(car(cur), frame), result);
    cur = cdr(cur);
  }
  if (result -> type == CONS_TYPE) {
    if (result -> c.car -> type == NULL_TYPE && result -> c.cdr -> type == CONS_TYPE) {
      if (result -> c.cdr -> c.car -> type == NULL_TYPE) {
        return result;
      }
    }
  }
  Value *reversed = reverse(result);
  return reversed;
}

/* Function: apply
 * --------------------
 *   This function applies the arguments passed into this function to the Scheme
 *   function that is also passed into it. The Scheme function could be a pointer
 *   to one of the previously binded functions in the global frame, or it can be
 *   a lambda closure (returned by evalLambda).
 *
 *   function: Pointer to a Scheme function or to a lambda closure.
 *   args: The arguments to be applied to function passed in.
 *   returns: The result of applying the arguments to the function passed in.
 */

Value *apply(Value *function, Value *args) {
  Frame *applyframe = talloc(sizeof(Frame));
  Value *result;

  if (function -> type == CLOSURE_TYPE) {
    applyframe -> bindings = talloc(sizeof(Value));
    applyframe -> bindings -> type = NULL_TYPE;
    Value *funcvalue = function; // get closure
    applyframe -> parent = funcvalue -> cl.frame;
    Value *cur = funcvalue -> cl.paramNames;

    while (cur -> type != NULL_TYPE) {
      Value *var = talloc(sizeof(Value));
      char *text = car(cur) -> s;
      var -> s = talloc(sizeof(char) * (strlen(text) + 1));
      strcpy(var -> s,text);
      var -> type = STR_TYPE;

      Value *val = talloc(sizeof(Value));
      if (length(args) == 0) {
        val = makeNull();
      }
      else {
        val = car(args);
      }

      Value *pair = talloc(sizeof(Value));
      pair -> type = CONS_TYPE;
      pair -> c.car = var;
      pair -> c.cdr = val;

      args = cdr(args);
      cur = cdr(cur);

      applyframe -> bindings = cons(pair, applyframe -> bindings);
    }
    result = eval(funcvalue -> cl.functionCode -> c.car, applyframe);
  }

  else { // apply Scheme function
    result = function -> pf(args);
  }
  return result;
}

/* Function: eval
 * --------------------
 *   This function evaluates each type of expression passed into it from other
 *   functions in this code, and returns the corresponding result.
 *
 *   expr: The expression to evaluate.
 *   frame: The current Frame struct of the interpreter.
 *   returns: The result of evaluating the expression.
 */

Value *eval(Value *expr, Frame *frame) {
  Value *result;
  switch (expr->type)  {
    case INT_TYPE: {
      result = expr;
      break;
    }
    case DOUBLE_TYPE: {
      result = expr;
      break;
    }
    case SYMBOL_TYPE: {
      return lookUpSymbol(expr, frame);
      break;
    }
    case CONS_TYPE: {
      Value *first = car(expr);
      Value *args = cdr(expr);

      if (!strcmp(first->s,"if")) {
        result = evalIf(args,frame);
      }

      else if (!strcmp(first->s,"let")) {
        result = evalLet(args,frame);
      }

      else if (!strcmp(first->s,"let*")) {
        result = evalLetStar(args,frame);
      }

      else if (!strcmp(first->s,"letrec")) {
        result = evalLetRec(args,frame);
      }

      else if (!strcmp(first->s,"quote")) {
        result = evalQuote(args,frame);
      }

      else if (!strcmp(first->s,"define")) {
        result = evalDefine(args, globalframe);
      }
      else if (!strcmp(first->s,"lambda")) {
        result = evalLambda(args, frame);
      }
      else if (!strcmp(first->s,"set!")) {
        result = evalSet(args, frame);
      }
      else if (!strcmp(first->s,"begin")) {
        result = evalBegin(args, frame);
      }

      else if (!strcmp(first->s,"and")) {
        result = evalAnd(args, frame);
      }

      else if (!strcmp(first->s,"or")) {
        result = evalOr(args, frame);
      }
      else if (!strcmp(first->s,"cond")) {
        result = evalCond(args, frame);
      }

      else {
        // If not a special form, evaluate the first, evaluate the args, then
        // apply the first to the args.
       Value *evaledOperator = eval(first, frame);
       Value *evaledArgs = evalEach(args, frame);
       return apply(evaledOperator,evaledArgs);
      }
      break;
    }
    case STR_TYPE: {
      result = expr;
      break;
    }
    case OPEN_TYPE: {
      result = expr;
      break;
    }
    case CLOSE_TYPE: {
      result = expr;
      break;
    }
    case BOOL_TYPE: {
      result = expr;
      break;
    }
    case PTR_TYPE: {
      result = expr;
      break;
    }
    case NULL_TYPE: {
      break;
    }
    case VOID_TYPE: {
      break;
    }
    case CLOSURE_TYPE: {
      break;
    }
    case PRIMITIVE_TYPE: {
      break;
    }
    case UNSPECIFIED_TYPE: {
      break;
    }
  }

  return result;
}