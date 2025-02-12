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

/* * * * * * * * * * * * * * * * * * * * * * * * */

// List of all possible keywords
char *keyword[] = {"const", "else", "fn", "if", "i32", "f64", "null", "pub", "return", "u8", "var", "void", "while"};

// List of all possible token names
const char *tokenName[] = {
    "TOKEN_EMPTY",
    "TOKEN_UNDERLINE",
    "TOKEN_IDENTIFIER",
    "TOKEN_IDENTIFIER_FUNC",
    "TOKEN_INT",
    "TOKEN_INT_EXP",
    "TOKEN_FLOAT",
    "TOKEN_FLOAT_EXP",
    "TOKEN_COMMENT",
    "TOKEN_STRING",
    "TOKEN_EOF",
    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_MULTIPLY",
    "TOKEN_DIVIDE",
    "TOKEN_CONCAT",
    "TOKEN_LEFT_BRACKET",
    "TOKEN_RIGHT_BRACKET",
    "TOKEN_LEFT_BRACE",
    "TOKEN_RIGHT_BRACE",
    "TOKEN_PIPE",
    "TOKEN_ARROW",
    "TOKEN_COMMA",
    "TOKEN_COLON",
    "TOKEN_SEMICOLON",
    "TOKEN_LESS_THAN",
    "TOKEN_LESS_EQUAL",
    "TOKEN_GREATER_THAN",
    "TOKEN_GREATER_EQUAL",
    "TOKEN_EQUAL",
    "TOKEN_AT",
    "TOKEN_ASSIGN",
    "TOKEN_NOT_EQUAL",
    "TOKEN_OPTIONAL_TYPE",
    "TOKEN_VAR",
    "TOKEN_ELSE",
    "TOKEN_FN",
    "TOKEN_IF",
    "TOKEN_I32",
    "TOKEN_I32_OPT",
    "TOKEN_F64",
    "TOKEN_F64_OPT",
    "TOKEN_U8",
    "TOKEN_U8_OPT",
    "TOKEN_PUB",
    "TOKEN_RETURN",
    "TOKEN_VOID",
    "TOKEN_WHILE",
    "TOKEN_CONST",
    "TOKEN_NULL",
    "TOKEN_END",
    "TOKEN_E"};

TokenType is_keyword(char *s) // func to check if Ident is Keyword
{
    for (int i = 0; i < 13; i++)
    {
        if (strcmp(s, keyword[i]) == 0)
        {
            switch (i)
            {
            case 0:
                return TOKEN_CONST;
            case 1:
                return TOKEN_ELSE;
            case 2:
                return TOKEN_FN;
            case 3:
                return TOKEN_IF;
            case 4:
                return TOKEN_I32;
            case 5:
                return TOKEN_F64;
            case 6:
                return TOKEN_NULL;
            case 7:
                return TOKEN_PUB;
            case 8:
                return TOKEN_RETURN;
            case 9:
                return TOKEN_U8;
            case 10:
                return TOKEN_VAR;
            case 11:
                return TOKEN_VOID;
            case 12:
                return TOKEN_WHILE;
            }
        }
    }
    return KEYWORD_CMP_ERR;
}

Token *init_token()
{
    Token *newToken = (Token *)malloc(sizeof(Token));
    if (!newToken)
    {
        printf("Error: malloc failed in init_token\n");
        error_exit(ERR_INTERNAL);
    }
    newToken->value = (char *)malloc(MAX_TOKEN_LENGHT * sizeof(char));
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

bool is_letter(char c) // check if char is letter
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    return false;
}

bool is_digit(char c) // check if char is number
{
    if (c >= '0' && c <= '9')
        return true;
    else
        return false;
}

int is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

char read_char(FILE *fp)
{
    char c = fgetc(fp);
    return c;
}

bool match_string(const char *target, FILE *input)
{
    size_t len = strlen(target);
    char string[len + 1]; // Buffer to hold the input string
    if (fread(string, 1, len, input) == len)
    {
        string[len] = '\0';
        return strcmp(string, target) == 0; 
    }
    return false; // Return false if reading fails
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

char *hex_to_decimal(char *hex)
{
    int len = strlen(hex);
    int base = 1;
    int dec_val = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        if (hex[i] >= '0' && hex[i] <= '9')
        {
            dec_val += (hex[i] - 48) * base;
            base = base * 16;
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F')
        {
            dec_val += (hex[i] - 55) * base;
            base = base * 16;
        }
    }
    char *out = malloc(sizeof(char) * 10);
    sprintf(out, "%d", dec_val);
    return out;
}

void get_token(Token *token)
{
    char c;
    dynamicString str;
    init_string(&str, 0);
    ScannerState state = sStart;
    int isToken = 0;
    while (isToken == 0)
    { // isToken = 1 mean, that we've returned token
        c = read_char(stdin);
        char hex[8];
        int idx = 0;
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
                state = sLiter_Int;
                append_string(&str, c);
                break;
            }
            else
            {
                switch (c)
                {
                case ',':
                    token->type = TOKEN_COMMA;
                    token->value = ",";
                    isToken = 1;
                    break;
                case '_':
                    append_string(&str, c);
                    c = read_char(stdin);
                    if (is_letter(c) || is_digit(c))
                    {
                        state = sIdent;
                        append_string(&str, c);
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_UNDERLINE;
                    token->value = "_";
                    isToken = 1;
                    break;
                case '+':
                    token->type = TOKEN_PLUS;
                    token->value = "+";
                    isToken = 1;
                    break;
                case '-':
                    append_string(&str, c);
                    c = read_char(stdin);
                    if (c == '>')
                    {
                        append_string(&str, c);
                        token->type = TOKEN_ARROW;
                        token->value = str.str;
                        isToken = 1;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_MINUS;
                    token->value = "-";
                    isToken = 1;
                    break;
                case '(':
                    token->type = TOKEN_LEFT_BRACKET;
                    token->value = "(";
                    isToken = 1;
                    break;
                case ')':
                    token->type = TOKEN_RIGHT_BRACKET;
                    token->value = ")";
                    isToken = 1;
                    break;
                case '{':
                    token->type = TOKEN_LEFT_BRACE;
                    token->value = "{";
                    isToken = 1;
                    break;
                case '}':
                    token->type = TOKEN_RIGHT_BRACE;
                    token->value = "}";
                    isToken = 1;
                    break;
                case '>':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_GREATER_EQUAL;
                        token->value = ">=";
                        isToken = 1;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_GREATER_THAN;
                    token->value = ">";
                    isToken = 1;
                    break;
                case '<':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_LESS_EQUAL;
                        token->value = "<=";
                        isToken = 1;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_LESS_THAN;
                    token->value = "<";
                    isToken = 1;
                    break;
                case ';':
                    token->type = TOKEN_SEMICOLON;
                    token->value = ";";
                    isToken = 1;
                    break;
                case ':':
                    token->type = TOKEN_COLON;
                    token->value = ":";
                    isToken = 1;
                    break;
                case '"':
                    state = sLiter;
                    break;
                case '/':
                    c = read_char(stdin);
                    if (c == '/')
                    {
                        state = sCommLine;
                        break;
                    }
                    else
                    {
                        ungetc(c, stdin);
                        token->type = TOKEN_DIVIDE;
                        token->value = "/";
                        isToken = 1;
                        break;
                    }

                case '|':
                    token->type = TOKEN_PIPE;
                    token->value = "|";
                    isToken = 1;
                    break;

                case '*':
                    token->type = TOKEN_MULTIPLY;
                    token->value = "*";
                    isToken = 1;
                    break;
                case '=':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_EQUAL;
                        token->value = "==";
                        isToken = 1;
                        break;
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_ASSIGN;
                    token->value = "=";
                    isToken = 1;
                    break;
                case '@':
                    token->type = TOKEN_AT;
                    token->value = "@";
                    isToken = 1;
                    break;
                case '!':
                    c = read_char(stdin);
                    if (c == '=')
                    {
                        token->type = TOKEN_NOT_EQUAL;
                        token->value = "!=";
                        isToken = 1;
                        break;
                    }
                    ungetc(c, stdin); // Is a single exclamation mark -> get back
                    error_handler(ERR_LEX, token);
                    break;

                case '[':
                    if (match_string("]u8", stdin))
                    {
                        token->type = TOKEN_U8;
                        token->value = "[]u8";
                        isToken = true;
                        break;
                    }
                    else
                    {
                        error_handler(ERR_LEX, token);
                        break;
                    }
                case '?':
                    state = sQuestion;
                    c = read_char(stdin);
                    // TODO
                    if (c == 'i')
                    {

                        if (match_string("32", stdin))
                        {
                            token->type = TOKEN_I32_OPT;
                            token->value = "?i32";
                            isToken = true;
                            break;
                        }
                        else
                            error_handler(ERR_LEX, token);
                    }
                    else if (c == 'f')
                    {
                        if (match_string("64", stdin))
                        {
                            token->type = TOKEN_F64_OPT;
                            token->value = "?f64";
                            isToken = true;
                            break;
                        }
                        else
                            error_handler(ERR_LEX, token);
                    }
                    else if (c == '[')
                    {
                        if (match_string("]u8", stdin))
                        {

                            token->type = TOKEN_U8_OPT;
                            token->value = "?[]u8";
                            isToken = true;
                            break;
                        }
                        else
                            error_handler(ERR_LEX, token);
                    }
                    else
                        error_handler(ERR_LEX, token);
                    // REVIEW
                    //  ... ? can '?' be single token?
                    token->type = TOKEN_OPTIONAL_TYPE;
                    token->value = "?";
                    isToken = true;
                    break;

                case EOF:
                    token->type = TOKEN_EOF;
                    token->value = "EOF";
                    isToken = 1;
                    break;

                default:
                    if (c > 32)
                    {
                        error_handler(ERR_LEX, token);
                    }
                    break;
                }
                break;
            }
            break;

        case sCommLine:
            while (c >= 32 || (isspace(c) && c != '\n' && c != EOF))
            { // wait for EOF to complete the comment
                append_string(&str, c);
                c = read_char(stdin);
            }
            if (c == '\n' || c == EOF)
            { // got EOF
                token->type = TOKEN_COMMENT;
                token->value = str.str;
                isToken = 1;
                break;
            }
            break;

        case sLiter_Int:
            while (is_digit(c))
            {
                token->type = TOKEN_INT;
                append_string(&str, c);
                c = read_char(stdin);
            }
            if (c == '.')
            {
                token->type = TOKEN_FLOAT; // REVIEW - need here?
                state = sLiter_Float;
                append_string(&str, c);
                break;
            }
            if (c == 'e' || c == 'E')
            { // get 'e' so it can be number with exponential
                state = sExp;
                append_string(&str, c);
                c = read_char(stdin); // Read the next character after '.'
                break;
            }
            else
            {
                if (is_letter(c) || c == '_' || c == '?')
                    error_handler(ERR_LEX, token);
                token->type = TOKEN_INT;
                token->value = str.str;
                isToken = 1;
                ungetc(c, stdin);
                break;
            }
            break;

        case sExp:
            if (c == '+' || c == '-')
            { // get '+'|'-' so we need to check if the next char is number in sExp2 state
                state = sExp_Char;
                append_string(&str, c);
                break;
            }
            else if (is_digit(c))
            {
                state = sExp_Final;
                append_string(&str, c);
                break;
            }
            else
                error_handler(ERR_LEX, token);

            break;

        case sExp_Char:
            if (is_digit(c))
            {
                state = sExp_Final;
                append_string(&str, c);
                break;
            }
            else
                error_handler(ERR_LEX, token);
            break;

        case sExp_Final:
            while (is_digit(c))
            {
                append_string(&str, c);
                c = read_char(stdin);
            }
            token->type = TOKEN_INT_EXP; // get exponential number with integer main part
            token->value = str.str;
            isToken = 1;
            ungetc(c, stdin);
            break;

        case sIdent:
            while (is_letter(c) || is_digit(c) || c == '_')
            {
                append_string(&str, c);
                c = read_char(stdin);
            }
            int key = is_keyword(str.str); // identificator can be a keyword, we need to check it
            if (key != KEYWORD_CMP_ERR)
            { 
                if (c == '?' && (key == TOKEN_I32 || key == TOKEN_F64))
                {
                    append_string(&str, c);
                    token->type = key + 1;
                    token->value = str.str;
                    isToken = 1;
                    break;
                }
                else
                {
                    ungetc(c, stdin);
                    token->type = key;
                    token->value = str.str;
                    isToken = 1;
                    break;
                }
            }
            if (strncmp(str.str, "ifj", 3) == 0)
            {
                while (isspace(c))
                {
                    c = read_char(stdin);
                }

                if (c == '.')
                {
                    append_string(&str, c);
                    c = read_char(stdin);

                    while (isspace(c))
                    {
                        c = read_char(stdin);
                    }

                    while (is_letter(c) || is_digit(c) || c == '_')
                    {
                        append_string(&str, c);
                        c = read_char(stdin);
                    }
                    ungetc(c, stdin);
                    token->type = TOKEN_IDENTIFIER_FUNC;
                    token->value = str.str;
                    isToken = 1;
                    break;
                }
            }
            ungetc(c, stdin); // identificator isnt keyword
            token->type = TOKEN_IDENTIFIER;
            token->value = str.str;
            isToken = 1;
            break;

        case sLiter:
            if ((c >= 35 || c == 32 || c == 33) && c != '\\' && c != '"')
            {
                append_string(&str, c);
                state = sLiterContent;
                break;
            }
            if (c == '"')
            {
                ungetc(c, stdin);
                state = sLiterContent;
                break;
            }
            if (c == '\\')
            {
                append_string(&str, c);
                state = sEsc;
                break;
            }
            else
            {
                error_handler(ERR_LEX, token);
                break;
            }
        case sLiterContent:
            if (c == '"')
            {
                token->type = TOKEN_STRING;
                token->value = str.str; // This will be an empty string
                isToken = 1;
                break;
            }
            while ((c >= 35 || c == 32 || c == 33) && c != '\\')
            {
                append_string(&str, c);
                c = read_char(stdin);
            }
            if (c == '\\')
            {
                state = sEsc;
                append_string(&str, c);
                break;
            }
            if (c == '"')
            {
                state = sLiter;
                token->type = TOKEN_STRING;
                token->value = str.str;
                isToken = 1;
                break;
            }
                break;
        case sEsc:
            if (c == 'x')
            {
                state = sHex; // Switch to hex state to process \x escape
                idx = 0;      // Reset the index for hex digits
                break;
            }
            else if (c == '"' || c == 'n' || c == 'r' || c == 't' || c == '\\')
            {
                // Handle other escape sequences like \n, \t, etc.
                state = sLiterContent;
                append_string(&str, c); // Append the corresponding character
                break;
            }
            else
            {
                error_handler(ERR_LEX, token); // Invalid escape sequence
                break;
            }
            break;

        case sHex:
            while (is_hex_digit(c))
            {
                hex[idx++] = c;
                if (idx == 2)
                { 
                    hex[idx] = '\0';
                    char esc = (char)strtol(hex, NULL, 16); // Convert hex to char
                    append_string(&str, esc);
                    state = sLiterContent;
                    idx = 0;
                    break;
                }
                c = read_char(stdin); // Read the next character
            }
            if (idx < 2 && state == sHex) 
            {
                error_handler(ERR_LEX, token); // Invalid hex escape sequence
                state = sLiterContent;
            }
            break;

        case sLiter_Float:
            while (is_digit(c))
            {
                append_string(&str, c);
                c = read_char(stdin);
            }

            if (c == 'e' || c == 'E')
            { // move to case with Exponential
                append_string(&str, c);
                state = sFloat_Exp;
                break;
            }
            else
            {
                token->type = TOKEN_FLOAT; // get token of float number
                token->value = str.str;
                isToken = 1;
                ungetc(c, stdin);
                break;
            }

        case sFloat_Exp:
            if (c == '+' || c == '-')
            {
                append_string(&str, c);
                state = sFloat_Exp_Char;
                break;
            }
            else if (is_digit(c))
            {
                append_string(&str, c);
                state = sFloat_Exp_Final;
                break;
            }
            else
                error_handler(ERR_LEX, token);
            break;

        case sFloat_Exp_Char:
            if (is_digit(c))
            {
                append_string(&str, c);
                state = sFloat_Exp_Final;
                break;
            }
            else
                error_handler(ERR_LEX, token);
            break;

        case sFloat_Exp_Final:
            while (is_digit(c))
            {
                append_string(&str, c);
                c = read_char(stdin);
            }
            token->type = TOKEN_FLOAT_EXP; // get exponential number with float main part
            token->value = str.str;
            isToken = 1;
            ungetc(c, stdin);
            break;
            break;

        default:
            error_handler(ERR_LEX, token);
  
            break;
        }
    }
}

#endif