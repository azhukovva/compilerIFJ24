/**
 * @file error.h
 * IFJ24 Compiler
 *
 * @brief Header file for error handling in the compiler
 * @author Aryna Zhukava (xzhuka01)
 */

#ifndef error_h
#define error_h

// Error in lexical analysis
// Incorrect structure of the actual lexeme
#define ERR_LEX 1

// Error in syntax analysis
#define ERR_SYNTAX 2

// Semantic error
// Undefined function or redefined variable
#define ERR_DEFINE 3

// Semantic error
// Wrong number/type of parameters in function call or wrong type of return value from function
#define ERR_WRONG_PARAM_RET 4

// Semantic error
// Use of undefined variable
#define ERR_UNDEF_VAR 5

// Semantic error
// Missing/excess expression in function return statement
#define ERR_MISS_OVERFL_RETURN 6

// Semantic type compatibility error in arithmetic and relational expressions
#define ERR_EXPR_TYPE 7

// Semantic type derivation error
#define ERR_EXPR_DERIV 8

// Semantic error
// Use of a variable outside its scope
#define ERR_VAR_OUT_OF_SCOPE 9

// Other semantic errors
#define ERR_SEM_OTHER 10

// Internal error
#define ERR_INTERNAL 99

int error_exit(int error);

char *errorText(int error);

#endif