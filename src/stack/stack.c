#include "stack.h"

void init_stack(Stack *stack) {
    stack->top = NULL;
}

void push(Stack *stack, Token *token) {
    StackElementPtr newElement = (StackElementPtr)malloc(sizeof(struct StackElement));
    if (newElement == NULL) {
        error_exit(ERR_INTERNAL);
    }
    newElement->data = token;
    newElement->nextElement = stack->top;
    stack->top = newElement;
}

Token *pop(Stack *stack) {
    if (stack->top == NULL) {
        return NULL;
    }
    StackElementPtr topElement = stack->top;
    Token *token = topElement->data;
    stack->top = topElement->nextElement;
    free(topElement);
    return token;
}

Token *top(Stack *stack) {
	if (stack->top == NULL) {
        return NULL;
    }
    return stack->top->data;
}

void free_stack(Stack *stack) {
    while (stack->top != NULL) {
        StackElementPtr topElement = stack->top;
        stack->top = topElement->nextElement;
        free(topElement->data);
        free(topElement);
    }
}

void print_stack(Stack *stack) {
    StackElementPtr current = stack->top;
    printf("---STACK---\n");
    while (current != NULL) {
        printf("type: %s val: %s\n", tokenName[current->data->type], current->data->value);
        current = current->nextElement;
    }
    printf("-----------\n");
}

bool stackIsEmpty(Stack *stack) {
    return stack->top == NULL;
}