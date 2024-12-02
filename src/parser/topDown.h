/**
* @file topDown.h
 * IFJ24 Compiler
 *
 * @brief Header file for the top-down parser implementation. It contains function declarations
 *        for parsing the program structure, handling various token types, and generating
 *        intermediate code for operations. The parser supports function definitions, variable
 *        declarations, control flow statements, and expression evaluations.
 *
 * @details This file provides the interface for the top-down parser functions, including
 *          functions for parsing tokens in a top-down manner, ensuring correct syntax and
 *          semantic analysis, and and generates the IFJ24code instructions.
 *
 * @authors Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

#ifndef TOPDOWN_H
#define TOPDOWN_H
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "../symTable/symTable.h"
#include "bottomUp.h"
#include "../generator/generator.h"
#include <stdlib.h>

// <functions> ::= <function> <functions_tail>
void functions_rule();

// <main_func> ::= pub fn main () void <Block>
void main_func_rule();

// <Prologue> ::= const ifj = @import(“ifj24.zig”) ;
void prologue_rule();

// <function> ::= pub fn func_id <Param_list> <Return_type> <Block>
void function_rule();

// <functions_tail> ::= <function> <functions_tail> | eps
void functions_tail_rule();

// <Param_list> ::= ( <Params> )
void param_list_rule();

// <Return_type> ::= void | <Base_type>
void return_type_rule();

// <Block> ::= { <Statements> }
void block_rule();

// <Params> ::= <Param> <Param_tail> | eps
void params_rule();

// <Param> ::= param_id : <Param_type>
void param_rule();

// <Param_tail> ::= , <Param> <Param_tail> | eps
void param_tail_rule();

// <Param_type> ::= <Base_type>
void param_type_rule();

// <Base_type> ::= i32 | f64 | []u8 | ?i32 | ?f64 | ?[]u8
void base_type_rule();

// <Statements> ::= <Statement> <Statements> | eps
void statements_rule(bool from_main, bool in_while);

// <Statement> ::= <var_def> | <conditionals> | <while_statement> | <Return_statement> | <Assigment> | <Function_call> 
void statement_rule(bool from_main, bool in_while);

// <var_def> ::= <Var_mode> id <Var_type> = Expression ;
void var_rule(bool from_main, bool in_while);

// <Assigment> ::= <function_call> | id = Expression ;
void assigment_rule(bool from_main);

// <Var_mode> ::= const | var
void var_mode_rule();

// <conditionals> ::= if ( Expression ) <Optional_null> <Block> <Optional_else>
void conditionals_rule(bool from_main);

//<Optional_null> ::= |id| | eps
void optional_null_rule(TokenType id_w_null, bool from_main, bool in_while);

//<Optional_else> ::= else <else_body> | eps
void optional_else_rule(bool from_main);

//<else_body> ::= <Block> | <Conditionals>
void else_body_rule(bool from_main);

//<while_statement> ::= while ( Expression ) <Optional_null> <Block>
void while_statement_rule(bool from_main, bool in_while);

// <Return_statement> ::= return <Expression_opt> ;
void return_statement_rule(bool from_main);

//<function_call> ::= id ( <Arguments> ) ;
void function_call_rule(char *var_id,bool from_main);

//<Arguments> ::= <Argument> <Arguments_tail>
void arguments_rule();

//<Arguments_tail> ::= , <Argument> <Arguments_tail> | eps
void arguments_tail_rule();

//<Argument> ::= id | <Literal>
void argument_rule();

//<Literal> ::= int | float | string
void literal_rule();

//check if the token type is the same as expected
void expect(TokenType type);

//check if the token value is the same as expected
void expect_id(char *id);
void expect_string(char *string);

//block_rule version for functions
void block_rule_fn(Node *fn, bool from_main);

//block_rule version for if statements
void if_block(bool from_main, int what_else_case);

//block_rule version for else statements
void else_block(bool from_main);

//block_rule version for while statements
void while_block(bool from_main, int what_while_case, int what_while_end_case, Expression *expr, bool in_while, int pipe_index);

void syntax_error();

//function to get the current frame
char* what_frame(bool from_main);

//function to convert the token type to the type of the variable
TokenType convert_from_lit(TokenType type);

//function to check if the token type is a literal
bool is_lit(TokenType type);

//function to check if the variable type and the expression result type are compatible
TokenType type_compatibility(TokenType variable, TokenType expr_type, bool from_args);

//function to get a token type from a string
TokenType get_token_type(const char *type_str);

//function to check if the token type is a statement start
bool is_statement_start(TokenType type);

//function to parse the arguments of the called function 
void parse_args(Node *signature, int index, bool builtin, bool from_main);

//function to genereate the built-in function instructions
void build_builtin(char *id, int curr_token, char *var_id, bool from_main);

#endif //TOPDOWN_H
