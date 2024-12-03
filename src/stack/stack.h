/**
* @file stack.h
 * IFJ24 Compiler
 *
 * @brief Header file for the stack data structure implementation. It contains type definitions
 *        and function declarations for initializing the stack, pushing and popping elements,
 *        checking the top element, freeing the stack, and printing the stack contents.
 *
 * @authors
 *          Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

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

// Function to push a token onto the stack
void push(Stack *stack, Token *token, char *value);

// Function to pop a token from the stack
Token *pop(Stack *stack);

// Function to get the top element of the stack
struct StackElement *top(Stack *stack);


void free_stack(Stack *stack);
void print_stack(Stack *stack);
bool stackIsEmpty(Stack *stack);

#endif //STACK_H

