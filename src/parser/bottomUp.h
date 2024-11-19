#ifndef BOTTOMUP_H
#define BOTTOMUP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "topDown.h"
#include "../generator/generator.h"

TokenType parse_expression(Expression *expression, FrameStack *frameStack, bool from_main);
int get_index(TokenType type);
char get_precedence(TokenType stack_top, TokenType input);
void reduce(Stack *stack, FrameStack *frameStack, bool from_main);
void shift(Stack *stack, Token *token);
void stack_reduce(Stack *stack, char *result, bool from_main);
char *relational(Stack *stack, bool from_main);
char *eq(Stack *stack, bool from_main);
char *arithmetic(Stack *stack, bool from_main);
void operation_instruction(Stack *stack, char *tempregister, bool from_main);
char *convert(char *value, bool from_main);
#endif //BOTTOMUP_H
