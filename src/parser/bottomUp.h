#ifndef BOTTOMUP_H
#define BOTTOMUP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "topDown.h"

TokenType parse_expression(Expression *expression, FrameStack *frameStack);
int get_index(TokenType type);
char get_precedence(TokenType stack_top, TokenType input);
void reduce(Stack *stack, FrameStack *frameStack);
void shift(Stack *stack, Token *token);
void stack_reduce(Stack *stack, char *result);
#endif //BOTTOMUP_H
