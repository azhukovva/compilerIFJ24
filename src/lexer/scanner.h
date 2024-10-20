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
#define MAX_TOKEN_LENGHT 100 // Random:)

typedef enum
{
    TOKEN_EMPTY,
    TOKEN_UNDERLINE,
    TOKEN_IDENTIFIER,
    TOKEN_IDENTIFIER_FUNC,
    TOKEN_INT,
    TOKEN_INT_EXP,
    TOKEN_FLOAT,
    TOKEN_FLOAT_EXP,
    TOKEN_COMMENT,
    TOKEN_STRING,
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
    TOKEN_ARROW,
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
    // KEYWORDS
    TOKEN_VAR,
    TOKEN_ELSE,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_I32,
    TOKEN_F64,
    TOKEN_PUB,
    TOKEN_RETURN,
    TOKEN_U8,
    TOKEN_VOID,
    TOKEN_WHILE,
    TOKEN_CONST,
    TOKEN_NULL,
} TokenType;

char *tokenName[] = {
    "tStringType",
    "tStringTypeNil",
    "tIntType",
    "tIntTypeNil",
    "tDoubleType",
    "tDoubleTypeNil",
    "tInt",
    "tFloat",
    "tExp",
    "tFloatExp",
    "tLiter",
    "tFn",
    "tIdent",
    "tUnderline",
    "tAdd",
    "tSub",
    "tMul",
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
    "tVar",
    "tLet",
    "tCommLine",
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

typedef enum
{
    sStart,
    sFn, // function
    // States for num
    sInt,

    sFloat,
    sFloat1, // state after dot, need to check if there is a digit after
    sFloatExp,
    sFloatExp1, // anything other than a digit or +/- after e/E -> error
    sFloatExp2, // anything other than a digit -> error
    sFloatExpFinal,

    sExp1,
    sExp2,
    sExp,
    // States for string
    sLiter,
    sLiterContent,
    sEsc,
    sHex,
    sHexContent,
    // States for comments
    sCommLine,
    // Other
    sUnderline,
    sIdent,
    sQuestion
} ScannerState;

typedef struct
{
    TokenType type;
    char *value;
} Token;

/**
 *@brief Function to initialize token
 *@return Pointer to the initialized token
 */
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
char escapeSequence(char *dec, Token *token);

#endif
