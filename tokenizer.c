/* tokenizer.c
 * Author: Khalid Hussain
 * --------------------
 * This program tokenizes given Scheme code into a list of numbers and symbols.
 * In the interest of simplicity, here is the syntax of numbers that the
 * tokenizer expects to handle:
 * <number>   -> <sign> <ureal> | <ureal>
 * <sign>     -> -
 * <ureal>    ->  <uinteger> | <udecimal>
 * <uinteger> ->  <digit>+
 * <udecimal> ->  . <digit>+ | <digit>+ . <digit>*
 * <digit>    ->  0 | 1 | ... | 9
 *
 * where * indicates zero or more repetitions, and + is one or more repetitions.
 * Here is the syntax of symbols/identifiers that the tokenizer expects:
 * <identifier> ->  <initial> <subsequent>* | + | -
 * <initial>    ->  <letter> | ! | $ | % | & | * | / | : | < | = | > | ? | ~ | _ | ^
 * <subsequent> ->  <initial> | <digit> | . | + | -
 * <letter>     ->  a | b | ... | z | A | B | ... | Z
 * <digit>      ->  0 | 1 | ... | 9
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "headers/linkedlist.h"
#include "headers/value.h"
#include "headers/talloc.h"

/* Function: concatenate
 * --------------------
 *   Function that concatenates a character to a string, and returns the
 *   concatenated string.
 *
 *   str: The string to concatenate the character to.
 *   ch: The character to concatenate to the string.
 *   returns: The concatenated string.
 */

static char *concatenate (char *str, char ch) {
  int last_position;
  char *newstring;
  if (str) {
    last_position = strlen(str);
    newstring = talloc(sizeof(char)*(strlen(str)) + 2);
    strncpy(newstring, str, strlen(str));
    str = newstring;
  }
  else {
    str = talloc(2);
    last_position = 0;
  }
  str[last_position] = ch;
  str[last_position + 1] = 0;
  return str;
}

/* Function: tokenize
 * --------------------
 *   Function that reads all of the input from stdin, and returens a linked List
 *   consisting of the tokens. Refer to the program notes above for the expected
 *   syntax.
 */

Value *tokenize() {
  char charRead;
  Value *list = makeNull();
  char numbers[] = "0123456789";
  char identifiersigns[] = "+-";
  char initialsym[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?~_^";
  char subseqsym[] = "0123456789.+-";
  charRead = (char)fgetc(stdin);
  char *conc = NULL;
  Value *error_list = makeNull();

  while (charRead != EOF) {
    conc = NULL;
    if (charRead == '(') {
      conc = concatenate(conc, charRead);
      Value *val = talloc(sizeof(Value));
      val -> type = OPEN_TYPE;
      char *text = conc;
      val -> s = talloc(sizeof(char) * (strlen(text) + 1));
      strcpy(val -> s,text);
      list = cons(val,list);
    }

    else if (charRead == ')') {
      conc = concatenate(conc, charRead);
      Value *val = talloc(sizeof(Value));
      val -> type = CLOSE_TYPE;
      char *text = conc;
      val -> s = talloc(sizeof(char) * (strlen(text) + 1));
      strcpy(val -> s,text);
      list = cons(val,list);
    }

    else if (strchr(identifiersigns, charRead) != NULL) { //read a <sign>
      char nextchar;
      nextchar = (char)fgetc(stdin);
      if (nextchar == ' ' || nextchar == EOF || nextchar == '\n' || nextchar == ')' || nextchar == '(') { // only read <sign>
        conc = concatenate(conc, charRead);
        Value *val = talloc(sizeof(Value));
        char *text = conc;
        val -> type = SYMBOL_TYPE;
        val -> s = talloc(sizeof(char) * (strlen(text) + 1));
        strcpy(val -> s, text);
        list = cons(val, list);
        ungetc(nextchar, stdin);
      }

      else if (strchr(initialsym, nextchar) != NULL) {
        printf("Syntax error \n");
        texit(0);
      }

      else if (charRead == '-' && (nextchar == '.' || strchr(numbers, nextchar) != NULL)) { // reads number or decimal after <sign>
        conc = concatenate(conc, charRead);
        if (nextchar == '.') { // reads decimal after sign
          conc = concatenate(conc, nextchar);
          char charafterdecimal = (char)fgetc(stdin);

          if (strchr(numbers, charafterdecimal) == NULL) {
            printf("Syntax error \n");
            texit(0);
          }

          while (charafterdecimal != EOF || charafterdecimal != ' ' || charafterdecimal != '\n') { // <sign> then <udecimal> -> . <digit>+
            if (strchr(numbers, charafterdecimal) != NULL) {
              conc = concatenate(conc, charafterdecimal);
              charafterdecimal = (char)fgetc(stdin);
            }
            else {
              break;
            }
          }

          ungetc(charafterdecimal, stdin);
          double number;
          char *ptr;
          Value *val = talloc(sizeof(Value));
          number = strtod(conc, &ptr);
          val -> type = DOUBLE_TYPE;
          val -> d = number;
          list = cons(val, list);
          conc = NULL;
        }

        else if (strchr(numbers, nextchar) != NULL) { // reads number after sign
          int decimalFlag = 0;
          while (nextchar != EOF || nextchar != ' ') { // <uinteger> -> <digit>+ or <udecimal> -> <digit>+ . <digit>*
            if (strchr(numbers, nextchar) != NULL || nextchar == '.') {
              if (nextchar == '.') {
                decimalFlag = 1;
              }
              conc = concatenate(conc, nextchar);
              nextchar = (char)fgetc(stdin);
            }
            else {
              break;
            }
          }

          ungetc(nextchar, stdin);
          if (decimalFlag == 0) {
            long number;
            char *ptr;
            Value *val = talloc(sizeof(Value));
            number = strtol(conc, &ptr, 10);
            val -> type = INT_TYPE;
            val -> i = number;
            list = cons(val, list);
            conc = NULL;
          }
          else if (decimalFlag == 1) {
            double number;
            char *ptr;
            Value *val = talloc(sizeof(Value));
            number = strtod(conc, &ptr);
            val -> type = DOUBLE_TYPE;
            val -> d = number;
            list = cons(val, list);
            conc = NULL;
          }
        }
      }

      else if (charRead == '+' && (nextchar != EOF || nextchar != ' ' || nextchar != '\n')) {
        printf("Syntax error \n");
        texit(0);
      }
    }

    else if (strchr(initialsym, charRead) != NULL) { // read <initial>
      char nextchar;
      nextchar = (char)fgetc(stdin);
      if (nextchar == EOF || nextchar == ' ' || nextchar == '\n' || nextchar == ')' || nextchar == '(') { // just <initial>
        conc = concatenate(conc, charRead);
        Value *val = talloc(sizeof(Value));
        char *text = conc;
        val -> type = SYMBOL_TYPE;
        val -> s = talloc(sizeof(char) * (strlen(text) +1));
        strcpy(val->s,text);
        list = cons(val, list);
        ungetc(nextchar, stdin);
      }
      else { // read <initial> <subsequent>+
        conc = concatenate(conc, charRead);
        if (strchr(initialsym, nextchar) != NULL || strchr(subseqsym, nextchar) != NULL) {
          conc = concatenate(conc, nextchar);
        }
        else {
          printf("Syntax error \n");
          texit(0);
        }
        while (charRead != EOF || charRead != ' ') {
          charRead = (char)fgetc(stdin);
          if (strchr(initialsym, charRead) != NULL || strchr(subseqsym, charRead) != NULL) {
            conc = concatenate(conc, charRead);
          }
          else {
            break;
          }
        }
        ungetc(charRead, stdin);
        Value *val = talloc(sizeof(Value));
        char *text = conc;
        val -> type = SYMBOL_TYPE;
        val -> s = talloc(sizeof(char)*(strlen(text) + 1));
        strcpy(val->s,text);
        list = cons(val, list);
      }
    }

    else if (charRead == '.') { // reads decimal
      conc = concatenate(conc, charRead);
      char charafterdecimal = (char)fgetc(stdin);
      if (strchr(numbers, charafterdecimal) == NULL) {
        printf("Syntax error \n");
        texit(0);
      }
      while (charafterdecimal != EOF || charafterdecimal != ' ') { // <udecimal> ->  . <digit>+
        if (strchr(numbers, charafterdecimal) != NULL) {
          conc = concatenate(conc, charafterdecimal);
          charafterdecimal = (char)fgetc(stdin);
        }
        else {
          break;
        }
      }
      ungetc(charafterdecimal, stdin);
      double number;
      char *ptr;
      Value *val = talloc(sizeof(Value));
      number = strtod(conc, &ptr);
      val -> type = DOUBLE_TYPE;
      val -> d = number;
      list = cons(val, list);
      conc = NULL;
    }

    else if (strchr(numbers, charRead) != NULL) { // <digit>
      int decimalFlag = 0;
      while (charRead != EOF || charRead != ' ') {
        if (strchr(numbers, charRead) != NULL || charRead == '.') { // <digit>+ or <digit>+ . <digit>*
          if (charRead == '.') {
            decimalFlag = 1;
          }
          conc = concatenate(conc, charRead);
          charRead = (char)fgetc(stdin);
        }
        else {
          break;
        }
      }
      ungetc(charRead, stdin);
      if (decimalFlag == 0) {
        long number;
        char *ptr;
        Value *val = talloc(sizeof(Value));
        number = strtol(conc, &ptr, 10);
        val -> type = INT_TYPE;
        val -> i = number;
        list = cons(val, list);
        conc = NULL;
      }
      else if (decimalFlag == 1) {
        double number;
        char *ptr;
        Value *val = talloc(sizeof(Value));
        number = strtod(conc, &ptr);
        val -> type = DOUBLE_TYPE;
        val -> d = number;
        list = cons(val, list);
        conc = NULL;
      }
    }

    else if (charRead == ' ' || charRead == '\n') {
      charRead = ' ';
    }

    else if (charRead == '#') {
      char nextchar;
      nextchar = (char)fgetc(stdin);
      if (nextchar == 'f' || nextchar == 't') {
        conc = concatenate(conc, charRead);
        conc = concatenate(conc, nextchar);
      }
      else {
        printf("Syntax error \n");
        texit(0);
      }
      Value *val = talloc(sizeof(Value));
      val -> type = BOOL_TYPE;
      char *text = conc;
      val -> s = talloc(sizeof(char)*(strlen(text) + 1));
      strcpy(val -> s,text);
      list = cons(val,list);
    }

    else if (charRead == '"') {
      conc = concatenate(conc, charRead);
      charRead = (char)fgetc(stdin);
      while (charRead != EOF) {
        conc = concatenate(conc, charRead);
        if (charRead == '"') {
          Value *val = talloc(sizeof(Value));
          char *text = conc;
          val -> type  = STR_TYPE;
          val -> s = talloc(sizeof(char)*(strlen(text) + 1));
          strcpy(val->s,text);
          list = cons(val, list);
          break;
        }
        charRead = (char)fgetc(stdin);
      }
      if (charRead == EOF) {
        printf("Syntax error \n");
        texit(0);
      }
    }

    else if (charRead == ';') { // if it is the beginning of a comment
      while (charRead != EOF) {
        if (charRead == '\n') {
          break;
        }
        charRead = (char)fgetc(stdin);
      }
    }

    else { // if it is not recognizable syntax, per program comments
      printf("Syntax error \n");
      texit(0);
    }

    charRead = (char)fgetc(stdin);
  }

  Value *revList = reverse(list);
  return revList;
}

/* Function: displayTokens
 * --------------------
 *   Displays the contents of the linked list as tokens, with type information.
 *
 *   list: List of tokens
 */

void displayTokens(Value *list) {
  Value *cur = list;
  while (cur != NULL) {
    switch (cur -> type) {
      case INT_TYPE:
        printf("%i:integer\n", car(cur) -> i);
        cur = cdr(cur);
        break;
      case DOUBLE_TYPE:
        printf("%f:double\n", car(cur) -> d);
        cur = cdr(cur);
        break;
      case STR_TYPE:
        printf("%s:string\n", car(cur) -> s);
        cur = cdr(cur);
        break;
      case CONS_TYPE:
        if (car(cur) -> type == INT_TYPE) {
          printf("%i:integer\n", car(cur) -> i);
        }
        else if (car(cur) -> type == DOUBLE_TYPE) {
          printf("%f:double\n", car(cur) -> d);
        }
        else if (car(cur) -> type == STR_TYPE) {
          printf("%s:string\n", car(cur) -> s);
        }
        else if (car(cur) -> type == PTR_TYPE) {
          printf("Address = %p \n", car(cur) -> p);
        }
        else if (car(cur) -> type == OPEN_TYPE) {
          printf("%s:open\n", car(cur) -> s);
        }
        else if (car(cur) -> type == CLOSE_TYPE) {
          printf("%s:close\n", car(cur) -> s);
        }
        else if (car(cur) -> type == BOOL_TYPE) {
          printf("%s:boolean\n", car(cur) -> s);
        }
        else if (car(cur) -> type == SYMBOL_TYPE) {
          printf("%s:symbol\n", car(cur) -> s);
        }
        cur = cdr(cur);
        break;
      case NULL_TYPE:
        cur = cdr(cur);
        goto exit_loop;
      case PTR_TYPE:
        printf("Address = %p", car(cur) -> p);
        break;
      case OPEN_TYPE:
        printf("%s:open\n", car(cur) -> s);
        cur = cdr(cur);
        break;
      case CLOSE_TYPE:
        printf("%s:close\n", car(cur) -> s);
        cur = cdr(cur);
        break;
      case BOOL_TYPE:
        printf("%s:boolean\n", car(cur) -> s);
        cur = cdr(cur);
        break;
      case SYMBOL_TYPE:
        printf("%s:symbol\n", car(cur) -> s);
        cur = cdr(cur);
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        break;
      case PRIMITIVE_TYPE:
        break;
      case UNSPECIFIED_TYPE:
        break;
    }
  }
  exit_loop: ;
}