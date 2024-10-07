/**
 * @file error.c
 * IFJ24 Compiler
 * 
 * @brief Implementation of error handling in the compiler
 * @author Aryna Zhukava (xzhuka01)
 */

#include <stdio.h>
#include "error.h"
#include <stdlib.h>


int error_exit(int error)
{
    fprintf(stderr, "ERROR: %s\n", errorText(error));
    exit(error);
}

char *errorText(int error)
{
    switch (error)
    {
    case ERR_LEX:
        return "Lexical error";
    case ERR_SYNTAX:
        return "Syntax error";
    case ERR_DEFINE:
        return "Undefined or redefined function";
    case ERR_WRONG_PARAM_RET:
        return "Wrong parameter or return value";
    case ERR_UNDEF_VAR:
        return "Use of undefined variable";
    case ERR_MISS_OVERFL_RETURN:
        return "Missing or excess return value";
    case ERR_EXPR_TYPE:
        return "Type error in expression";
    case ERR_EXPR_DERIV:
        return "Type derivation error";
    case ERR_VAR_OUT_OF_SCOPE:
        return "Variable out of scope";
    case ERR_SEM_OTHER:
        return "Other semantic error";
    case ERR_INTERNAL:
        return "Internal error";
    default:
        return "Unknown error";
    }
}