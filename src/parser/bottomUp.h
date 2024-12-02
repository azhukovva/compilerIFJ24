/**
* @file bottomUp.h
 * IFJ24 Compiler
 *
 * @brief Header file for the bottom-up parser implementation. It contains function declarations
 *        for parsing expressions using shift-reduce parsing, handling various token types, and
 *        managing the precedence rules. The parser supports arithmetic, relational, and equality
 *        operations, as well as handling parentheses and different token types.
 *
 * @details This file provides the interface for the bottom-up parser functions, including
 *          functions for shifting tokens onto the stack, reducing the stack based on grammar
 *          rules, and generating intermediate code for operations.
 *
 * @authors Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

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

// Function to parse an expression using shift-reduce parsing
TokenType parse_expression(Expression *expression, FrameStack *frameStack, bool from_main);

int get_index(TokenType type);

// Function to get the precedence between two tokens
char get_precedence(TokenType stack_top, TokenType input);

// Function to reduce the top elements of the stack based on grammar rules
void reduce(Stack *stack, FrameStack *frameStack, bool from_main);

// Function to shift a token onto the stack
void shift(Stack *stack, Token *token);

// Function to reduce the stack based on the grammar rules
void stack_reduce(Stack *stack, char *result, bool from_main);

// Function to check for operand types and return the result type for relational operations (<, >, <=, >=)
char *relational(Stack *stack, bool from_main);

//Function to check for operand types and return the result type for == and != operations
char *eq(Stack *stack, bool from_main);

// Function to check for operand types and return the result type for arithmetic operations (+, -, *, /)
char *arithmetic(Stack *stack, bool from_main);


void operation_instruction(Stack *stack, char *tempregister, bool from_main);
char *convert(char *value, bool from_main);
#endif //BOTTOMUP_H
