/**
* @file expression.h
 * IFJ24 Compiler
 *
 * @brief Header file for the expression handling implementation. It contains type definitions and
 *        function declarations for initializing expressions, adding elements to expressions, freeing
 *        expressions, and printing expressions for debugging purposes. The `Expression` datatype is
 *        implemented as a linked list.
 *
 * @authors
 *          Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"


typedef struct ExpElement {
    Token *data;
    struct ExpElement *nextElement;
} *ExpElementPtr;

typedef struct {
    ExpElementPtr firstElement;
    ExpElementPtr lastElement;
    int currentLength;
} Expression;

void init_expression(Expression *expression);
void add_element(Expression *expression, Token *token);
void free_expression(Expression *expression);
void print_expression(Expression *expression);

#endif //EXPRESSION_H
