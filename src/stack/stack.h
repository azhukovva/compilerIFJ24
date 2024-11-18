#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"

typedef struct StackElement {
    Token *data;
    char *value;
    struct StackElement *nextElement;
} *StackElementPtr;

typedef struct {
    StackElementPtr top;
} Stack;


void init_stack(Stack *stack);
void push(Stack *stack, Token *token, char *value);
Token *pop(Stack *stack);
struct StackElement *top(Stack *stack);
void free_stack(Stack *stack);
void print_stack(Stack *stack);
bool stackIsEmpty(Stack *stack);

#endif //STACK_H

