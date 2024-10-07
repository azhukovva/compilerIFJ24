/**
 * @file scanner.h
 * IFJ24 Compiler
 * 
 * @author Aryna Zhukava (xzhuka01)
 * @brief Header file for scanner.
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "error.h"

#define KEYWORD_CMP_ERR 99

typedef enum
{
    TOKEN_EMPTY,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER_FUNC,
    TOKEN_IDENTIFIER_VAR,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_NULL,
    TOKEN_EOF,
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_CONCAT,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_LESS_THAN,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_THAN,
    TOKEN_GREATER_EQUAL,
    TOKEN_EQUAL,
    TOKEN_ASSIGN,
    TOKEN_NOT_EQUAL,
    TOKEN_OPTIONAL_TYPE,
} TokenType;

char * tokenName[] = {
    "tStringType", "tStringTypeNil", 
    "tIntType", "tIntTypeNil",
    "tDoubleType", "tDoubleTypeNil",
    "tInt", 
    "tFloat", 
    "tExp", 
    "tFloatExp",
    "tLiter", 
    "tFunc", 
    "tRetFunc",
    "tIdent", 
    "tUnderline",
    "tAdd", 
    "tSub", "tMul", 
    "tDiv",
    "tPlus",
    "tMinus", 
    "tAssign", 
    "tEqual",
    "tBrLeft", 
    "tBrRight", 
    "tMore", 
    "tNil",
    "tVoid",
    "tMoreOrEqual",
    "tLess", 
    "tLessOrEqual", 
    "tNotEqual",
    "tExclam", 
    "tDoubleQuestion",
    "tSemicolon", 
    "tComma",
    "tColon", 
    "tReturn", 
    "tIf", 
    "tElse",
    "tWhile", 
    "tVar", "tLet", 
    "tRemainder",
    "tCommBlock", 
    "tCommLine", 
    "tCommInside",
    "tLCurl", 
    "tRCurl",
    "tEOF",
    "tArrow",
};

typedef enum
{
    KW_CONST,
    KW_ELSE,
    KW_FN,
    KW_IF,
    KW_I32,
    KW_F64,
    KW_NULL,
    KW_PUB,
    KW_RETURN,
    KW_U8,
    KW_VAR,
    KW_VOID,
    KW_WHILE,
} KeywordType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

Token *init_token();

/**
 * @brief Function to read next token
 * @param token The token to read
 */
void get_token(Token *token);

/**
 * @brief Function to check if indetifier is a keyword
 * @param s The identifier to check
 * @return The token type of the keyword, or IDENTIFIER if not a keyword
 */
TokenType is_keyword(char *s);

/**
 * @brief Function to check if a character is a letter
 * @param c The character to check
 * @return True if the character is a letter, false otherwise
 */
bool is_letter(char c);

/**
 * @brief Function to check if a character is a digit
 * @param c The character to check
 * @return True if the character is a digit, false otherwise
 */
bool is_digit(char c);

/**
 * @brief Handles errors by freeing the token and exiting the program with the given error code
 * @param error The error code to exit the program with
 * @param token The token to free
 */
void error_handler(int error, Token *token);

/**
 * @brief Convert escape sequences in a string to their corresponding characters
 * @param dec The escape sequence to convert
 * @param token The token to handle errors with
 * @return The character corresponding to the escape sequence 
 */
char escapeSequence(char* dec, Token *token);

#endif
