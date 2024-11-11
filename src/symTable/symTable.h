#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include "../lexer/scanner.h"
#include "../stack/stack.h"

typedef enum {
    ITEM_VARIABLE,
    ITEM_FUNCTION
} item_type_t;

typedef struct {
    TokenType type;
    char *id;
    char *value;
} variable_data_t;

typedef struct function_data_t {
    TokenType return_type;
    char *id;
    variable_data_t *arguments;
} function_data_t;

typedef union {
    variable_data_t variable;
    struct function_data_t *function;
} item_data_t;

typedef struct bst_node {
    char *key;
    item_type_t type;
    item_data_t data;
    struct bst_node *left;
    struct bst_node *right;
    int height;
} bst_node_t;

typedef struct fStackElement {
    bst_node_t *root;
    struct fStackElement *nextElement;
} *fStackElementPtr;

typedef struct {
    fStackElementPtr top;
} fStack;


void frame_init(bst_node_t **tree);
void add_frame(fStack *top);
void remove_frame

#endif //SYMTABLE_H