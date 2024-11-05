#ifndef TOPDOWN_H
#define TOPDOWN_H
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"

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

bool is_statement_start(TokenType type);

#endif //TOPDOWN_H
