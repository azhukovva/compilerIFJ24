#ifndef TOPDOWN_H
#define TOPDOWN_H
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "../symTable/symTable.h"
#include "bottomUp.h"

void functions_rule();
void main_func_rule();
void prologue_rule();
void function_rule();
void functions_tail_rule();
void param_list_rule();
void return_type_rule();
void block_rule();
void params_rule();
void param_rule();
void param_tail_rule();
void param_type_rule();
void base_type_rule();
void statements_rule();
void statement_rule();
void var_rule();
void if_rule();
void while_rule();
void return_rule();
void assigment_rule();
void var_type_rule();
void var_mode_rule();
void conditionals_rule();
void optional_null_rule();
void optional_else_rule();
void else_body_rule();
void while_statement_rule();
void return_statement_rule();
void expression_opt_rule();
void function_call_rule();
void arguments_rule();
void arguments_tail_rule();
void argument_rule();
void literal_rule();
void expect(TokenType type);
void expect_id(char *id);
void expect_string(char *string);
void block_rule_fn(Node *fn);

void parse_args(Node *signature, int index);
TokenType convert_from_lit(TokenType type);
bool is_lit(TokenType type);
TokenType type_compatibility(TokenType variable, TokenType expr_type, bool from_args);
TokenType get_token_type(const char *type_str);
bool is_statement_start(TokenType type);
void parse_args(Node *signature, int index);

#endif //TOPDOWN_H
