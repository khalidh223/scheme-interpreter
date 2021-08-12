/* main.c
 * Author: Khalid Hussain
 * --------------------
 * This program is the main driver for the intepretation of Scheme code.
 * First, it tokenizes the Scheme code into a list of tokens with tokenize.c,
 * then it parses the Scheme code with parser.c by creating nested linked list
 * in accordance with the number of open/close parenthesis, and finally it
 * interprets the resulting tree with interpreter.c
 */

#include <stdio.h>
#include "headers/tokenizer.h"
#include "headers/value.h"
#include "headers/linkedlist.h"
#include "headers/parser.h"
#include "headers/talloc.h"
#include "headers/interpreter.h"

int main() {

    Value *list = tokenize();
    Value *tree = parse(list);
    interpret(tree);

    tfree();
    return 0;
}
