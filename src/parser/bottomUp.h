#ifndef BOTTOMUP_H
#define BOTTOMUP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "topDown.h"

void parse_expression(Expression *expression);
int get_index(TokenType type);
char get_precedence(TokenType stack_top, TokenType input);
void reduce(Stack *stack);
#endif //BOTTOMUP_H
