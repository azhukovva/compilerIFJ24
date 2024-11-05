#include "expression.h"
#include "../parser/precAnalyza.h"
#include "../lexer/scanner.h"

void init_expression(Expression *expression) {
    expression->firstElement = NULL;
    expression->lastElement = NULL;
    expression->currentLength = 0;
}

void add_element(Expression *expression, Token *token) {
    ExpElementPtr newElement = (ExpElementPtr)malloc(sizeof(struct ExpElement));
    if (newElement == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    newElement->data = token;
    newElement->nextElement = NULL;

    if (expression->firstElement == NULL) {
        expression->firstElement = newElement;
    } else {
        expression->lastElement->nextElement = newElement;
    }
    expression->lastElement = newElement;
    expression->currentLength++;
}

void free_expression(Expression *expression) {
    ExpElementPtr current = expression->firstElement;
    while (current != NULL) {
        ExpElementPtr next = current->nextElement;
        free(current->data);
        free(current);
        current = next;
    }
    expression->firstElement = NULL;
    expression->lastElement = NULL;
    expression->currentLength = 0;
}

void print_expression(Expression *expression) {
    ExpElementPtr current = expression->firstElement;
    while (current != NULL) {
        printf("type: %d val: %s\n", current->data->type, current->data->value);
        current = current->nextElement;
    }
}