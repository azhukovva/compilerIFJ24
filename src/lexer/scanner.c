/**
 * @file scanner.c
 * IFJ24 Compiler
 *
 * @brief Scanner implementation for IFJ24 language
 * Reads the input source code character by character.
 * Recognizes lexemes (the smallest meaningful units of code, e.g. identifiers, keywords, literals).
 * Creates tokens based on recognized lexemes.
 * Passes tokens to the next compilation stages (e.g., syntactic parser).
 *
 * @author Aryna Zhukava (xzhuka01)
 */

#ifndef SCANNER_C
#define SCANNER_C

#include "scanner.h"
#include "dynamicString.h"

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * HELPER FUNCTIONS for scanner
 *
 */
/**
 * @brief Function to read a character from a file
 * @param file The file to read from
 * @return The character read from the file
 */
char read_char(FILE *file)
{
    char c = fgetc(file);
    return c;
}

Token *init_token()
{
    Token *newToken = (Token *)malloc(sizeof(Token));
    if (!newToken)
    {
        printf("Error: malloc failed in init_token\n");
        error_exit(ERR_INTERNAL);
    }
    // new_token->type = NULL;
    newToken->value = (char *)malloc(MAX_TOKEN_LENGHT * sizeof(char));
    // new_token->value = NULL;
    return newToken;
}

void print_token(Token *token)
{
    if (token == NULL)
    {
        printf("Error: token is NULL in print_token\n");
        return;
    }
    printf("Token: %s, %s\n", tokenName[token->type], token->value);
}

void free_token(Token *token)
{
    if (token == NULL)
    {
        printf("Error: token is NULL in free_token\n");
        return;
    }
    free(token);
}

void error_handler(int error, Token *token)
{
    free_token(token);
    error_exit(error);
}
/* * * * * * * * * * * * * * * * * * * * * * * * */

// List of all possible keywords
char *keyword[] = {"const", "else", "fn", "if", "i32", "f64", "null", "pub", "return", "u8", "var", "void", "while"};

TokenType is_keyword(char *c)
{
    for (int i = 0; i < 13; i++)
    {
        if (strcmp(c, keyword[i]) == 0)
        {
            switch (i)
            {
            case 0:
                return KW_CONST;
            case 1:
                return KW_ELSE;
            case 2:
                return KW_FN;
            case 3:
                return KW_IF;
            case 4:
                return KW_I32;
            case 5:
                return KW_F64;
            case 6:
                return KW_NULL;
            case 7:
                return KW_PUB;
            case 8:
                return KW_RETURN;
            case 9:
                return KW_U8;
            case 10:
                return KW_VAR;
            case 11:
                return KW_VOID;
            case 12:
                return KW_WHILE;
            }
        }
    }
    // If c doesn't match any keyword
    return KEYWORD_CMP_ERR;
}

bool is_letter(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    else
        return false;
}

bool is_digit(char c)
{
    if (c >= '0' && c <= '9')
        return true;
    else
        return false;
}

char escapeSequence(char *dec, Token *token)
{
    if (strcmp(dec, "092") == 0)
        return '\\';
    if (strcmp(dec, "114") == 0)
        return 'r';
    if (strcmp(dec, "116") == 0)
        return 't';
    if (strcmp(dec, "110") == 0)
        return 'n';
    if (strcmp(dec, "034") == 0)
        return '"';
    error_handler(ERR_LEX, token);
    return 0;
}

void get_token(Token *token)
{
    char c;
    dynamicString str;
    init_string(&str, MAX_TOKEN_LENGHT); // String of size 100
    bool isToken = false;                // Will indicate if we returned a token
    ScannerState state = sStart;         // Initial
    while (isToken == false)
    {
        c = read_char(stdin);
        switch (state)
        {
        case sStart:
            if (is_letter(c))
            {
                state = sIdent;
                append_string(&str, c);
                break;
            }
            if (is_digit(c))
            {
                state = sInt;
                append_string(&str, c);
                break;
            }
            else
            {
                switch (c)
                {
                case '_':
                    append_string(&str, c);
                    c = read_char(stdin); // Check next character
                    if (is_letter(c) || is_digit(c))
                    {
                        state = sIdent;
                        string_Append(&str, c);
                        break;
                    }
                    ungetc(c, stdin); // Not a letter or a digit -> get back, is not identifier and should be processed separately
                    token->type = TOKEN_UNDERLINE;
                    token->value = "_";
                    isToken = true;
                    break;
                case '+':
                    token->type = TOKEN_PLUS;
                    token->value = "+";
                    isToken = true;
                    break;
                case '-':
                    string_Append(&str, c);
                    c = read_char(stdin);
                    if (c == '>')
                    {
                        string_Append(&str, c);
                        token->type = TOKEN_ARROW;
                        token->value = str.str;
                        isToken = true;
                        break;
                    }
                    ungetc(c, stdin); // Is a single character, so get back
                    token->type = TOKEN_MINUS;
                    token->value = "-";
                    isToken = true;
                    break;
                case '*':
                    token->type = TOKEN_MULTIPLY;
                    token->value = "*";
                    isToken = true;
                    break;
                case '/':
                    c = read_char(stdin);
                    if (c == '/')
                    {
                        state = sCommLine;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_DIVIDE;
                    token->value = "/";
                    isToken = true;
                    break;
                // Brackets
                case '(':
                    token->type = TOKEN_LEFT_BRACKET;
                    token->value = "(";
                    isToken = true;
                    break;
                case ')':
                    token->type = TOKEN_RIGHT_BRACKET;
                    token->value = ")";
                    isToken = true;
                    break;
                case '{':
                    token->type = TOKEN_LEFT_BRACE;
                    token->value = "{";
                    isToken = true;
                    break;
                case '}':
                    token->type = TOKEN_RIGHT_BRACE;
                    token->value = "}";
                    isToken = true;
                    break;

                case ',':
                    token->type = TOKEN_COMMA;
                    token->value = ",";
                    isToken = true;
                    break;
                case ':':
                    token->type = TOKEN_COLON;
                    token->value = ":";
                    isToken = true;
                    break;
                case ';':
                    token->type = TOKEN_SEMICOLON;
                    token->value = ";";
                    isToken = true;
                    break;
                case '<':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_LESS_EQUAL;
                        token->value = "<=";
                        isToken = true;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_LESS_THAN;
                    token->value = "<";
                    isToken = true;
                    break;
                case '>':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_GREATER_EQUAL;
                        token->value = ">=";
                        isToken = true;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_GREATER_THAN;
                    token->value = ">";
                    isToken = true;
                    break;
                case '=':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_EQUAL;
                        token->value = "==";
                        isToken = true;
                        break;
                    }
                    ungetc(c, stdin); // Is a single equal sign -> get back
                    token->type = TOKEN_ASSIGN;
                    token->value = "=";
                    isToken = true;
                    break;
                case '!':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_NOT_EQUAL;
                        token->value = "!=";
                        isToken = true;
                        break;
                    }
                    ungetc(c, stdin); // Is a single exclamation mark -> get back
                    error_handler(ERR_LEX, token);
                    break;

                case '"':
                    state = sLiter;
                    break;
                case '?':
                    state = sQuestion;
                    c = read_char(stdin);
                    // ... ? can '?' be single token?
                    token->type = TOKEN_OPTIONAL_TYPE;
                    token->value = "?";
                    isToken = true;
                    break;
                case EOF:
                    token->type = TOKEN_EOF;
                    token->value = "EOF";
                    isToken = true;
                    break;

                default:
                    // Is not whitespace, letter, digit, control chars or any of the above -> error
                    if (c > 32)
                        error_handler(ERR_LEX, token);
                    break;
                }
                break;
            }
            break;
        }
    }
}

int main()
{
    Token *token = init_token();
    char c;
    while ((c = read_char(stdin)) != EOF)
    {
        ungetc(c, stdin);
        get_token(token);
        print_token(token);
    }

    return 0;
}

#endif // SCANNER_C
