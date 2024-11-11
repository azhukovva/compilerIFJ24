#include "symTable.h"

Token *all_tokens[100000];
int token_index = 0;
FrameStack *frameStack = NULL;
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

void fill_sym_table_fn(FrameStack *frameStack, int token_index) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        error_exit(ERR_INTERNAL);
    }
    node->id = NULL;
    node->type = TOKEN_EMPTY; // Replace with appropriate TokenType
    node->fn = false;
    node->t_const = false;
    node->params = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;

    if (all_tokens[token_index]->type == TOKEN_FN) {
        token_index++; // Skip `fn`
        // Allocate memory for node->id
        node->id = (char *)malloc(strlen(all_tokens[token_index]->value) + 1);
        if (node->id == NULL) {
            error_exit(ERR_INTERNAL);
        }
        strcpy(node->id, all_tokens[token_index]->value);
        printf("ID: %s\n", node->id);

        node->fn = true;
        node->t_const = false;
        token_index += 2; // Skip id and (

        // Parse parameters
        Param *last_param = NULL;
        while (all_tokens[token_index]->type != TOKEN_RIGHT_BRACKET) {
            if (is_data_type()) {
                Param *new_param = (Param *)malloc(sizeof(Param));
                if (new_param == NULL) {
                    error_exit(ERR_INTERNAL);
                }
                new_param->type = all_tokens[token_index]->type;
                new_param->next = NULL;

                if (last_param == NULL) {
                    node->params = new_param;
                } else {
                    last_param->next = new_param;
                }
                last_param = new_param;
            }
            token_index++;
        }

        // Parse return type
        while (all_tokens[token_index]->type != TOKEN_LEFT_BRACE) {
            if (is_data_type()) {
                node->type = all_tokens[token_index]->type;
                break;
            }
            token_index++;
        }

        // Add node to the tree
        add_item(frameStack, node);
    }
}

int main() {
    FrameStack *frameStack = init_frameStack();
    add_frame(frameStack);

    Token *current_token = init_token();
    while (current_token->type != TOKEN_EOF) {
        get_token(current_token);
        save_token(current_token);
    }

    token_index = 0;
    while (all_tokens[token_index]->type != TOKEN_EOF) {
        fill_sym_table_fn(frameStack, token_index);
        token_index++;
    }

    // Print the tree in in-order traversal to verify the structure
    printFrameStack(frameStack);

    // Free the tree nodes and parameters after use to avoid memory leaks
    // This part would involve recursive deallocation functions for each node and param

    return 0;
}