#ifndef TOPDOWN_H
#define TOPDOWN_H
#include "../lexer/scanner.h"
#include "../stack/stack.h"
#include "../expression/expression.h"
#include "../symTable/symTable.h"
#include "bottomUp.h"
#include "../generator/generator.h"
#include <stdlib.h>

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
void statements_rule(bool from_main, bool in_while);
void statement_rule(bool from_main, bool in_while);
void var_rule(bool from_main, bool in_while);
void if_rule();
void while_rule();
void return_rule(bool from_main);
void assigment_rule(bool from_main);
void var_type_rule();
void var_mode_rule();
void conditionals_rule(bool from_main);
void optional_null_rule(TokenType id_w_null, bool from_main, bool in_while);
void optional_else_rule(bool from_main);
void else_body_rule(bool from_main);
void while_statement_rule(bool from_main, bool in_while);
void return_statement_rule(bool from_main);
void expression_opt_rule();
void function_call_rule(char *var_id,bool from_main);
void arguments_rule();
void arguments_tail_rule();
void argument_rule();
void literal_rule();
void expect(TokenType type);
void expect_id(char *id);
void expect_string(char *string);
void block_rule_fn(Node *fn, bool from_main);
void if_block(bool from_main, int what_else_case);
void else_block(bool from_main);
void while_block(bool from_main, int what_while_case, int what_while_end_case, Expression *expr, bool in_while, int pipe_index);

char* what_frame(bool from_main);
TokenType convert_from_lit(TokenType type);
bool is_lit(TokenType type);
TokenType type_compatibility(TokenType variable, TokenType expr_type, bool from_args);
TokenType get_token_type(const char *type_str);
bool is_statement_start(TokenType type);
void parse_args(Node *signature, int index, bool builtin);
void build_builtin(char *id, int curr_token, char *var_id, bool from_main);
void print_param_list(const char *label, Param *head);
#endif //TOPDOWN_H
