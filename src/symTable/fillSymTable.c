#include "../lexer/scanner.h"



Token *all_tokens[100000];
int token_index = 0;
typedef struct Param{
    TokenType type;
    struct Param *next;
} Param;
typedef struct Node{
    char *id;
    TokenType type;
    bool fn;
    bool t_const;
    Param *params;
} Node;
Node *node = NULL;


void save_token(Token *current_token){
    Token *token_copy = (Token *)malloc(sizeof(Token));
    if (token_copy == NULL) {
        error_exit(ERR_INTERNAL);
    }
    *token_copy = *current_token;
    all_tokens[token_index] = token_copy;
    token_index++;
}

bool is_data_type(){
    return all_tokens[token_index]->type == TOKEN_I32 || all_tokens[token_index]->type == TOKEN_F64 || all_tokens[token_index]->type == TOKEN_U8 ||
           all_tokens[token_index]->type == TOKEN_I32_OPT || all_tokens[token_index]->type == TOKEN_F64_OPT || all_tokens[token_index]->type == TOKEN_U8_OPT; 
}

void fill_sym_table_fn(){
    if(all_tokens[token_index]->type == TOKEN_FN){
        token_index++; //skip fn
       node->id = all_tokens[token_index]->value;
       node->fn = true;
       node->t_const = false;
       token_index = token_index + 2; //skip id and (

       Param *last_param = NULL;
       while(all_tokens[token_index]->type != TOKEN_RIGHT_BRACKET){
            if(is_data_type()){
            Param *new_param = (Param *)malloc(sizeof(Param));
            new_param->type = all_tokens[token_index]->type;
            new_param->next = NULL;
            if(last_param == NULL){
                node->params = new_param;;
            } else {
                last_param->next = new_param;
            }
            last_param = new_param;
            }
            token_index++;
        }
        while(all_tokens[token_index]->type != TOKEN_LEFT_BRACE){
            if(is_data_type()){
                node->type = all_tokens[token_index]->type;
                break;
            }
            token_index++;
        }
    }
}

int main(){
    Token *current_token;
    current_token = init_token();
    while(current_token->type != TOKEN_EOF){
        get_token(current_token);
        save_token(current_token);
        //kys();
    }
    token_index = 0;
    node = (Node *)malloc(sizeof(Node));
    node->params = NULL; // Initialize params to NULL
    while(all_tokens[token_index]->type != TOKEN_EOF){
        fill_sym_table_fn();
        token_index++;
    }
    for(int i = 0; i < token_index; i++){
        printf("Token: %s, %s\n", tokenName[all_tokens[i]->type], all_tokens[i]->value);
    }
    printf("ID:%s\n TYPE:%s\n FN:%d\n CONST:%d\n", node->id, tokenName[node->type], node->fn, node->t_const );
    while(node->params != NULL){
        printf("PARAM:%s\n", tokenName[node->params->type]);
        Param *temp = node->params;
        node->params = node->params->next;
        free(temp); // Free the parameter memory
    }
    free(node); // Free the node memory
    return 0;
}