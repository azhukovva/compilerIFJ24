#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"


/** Prvek jednosměrně vázaného seznamu. */
typedef struct ExpElement {
    Token *data;
    struct ExpElement *nextElement;
} *ExpElementPtr;

/** Jednosměrně vázaný seznam. */
typedef struct {
    ExpElementPtr firstElement;
    ExpElementPtr lastElement;
    int currentLength;
} Expression;

void init_expression(Expression *expression);
void add_element(Expression *expression, Token *token);
void free_expression(Expression *expression);

#endif //EXPRESSION_H
