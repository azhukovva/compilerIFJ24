#ifndef TOPDOWN_H
#define TOPDOWN_H
#include "../lexer/scanner.h"

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
void optional_q_mark_rule();
void base_type_rule();
void statements_rule();
void statement_rule();
bool is_statement_start(TokenType type);

#endif //TOPDOWN_H
