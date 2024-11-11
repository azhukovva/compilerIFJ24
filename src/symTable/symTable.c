#include "symTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void frame_init(bst_node_t **tree) {
    *tree = NULL;
}

void symTable_init(symtable_t *table) {
    table->global_frame = (bst_t *)malloc(sizeof(bst_t));
    frame_init(&(table->global_frame->root));
    table->global_frame->lower_frame = NULL;
    table->current_frame = table->global_frame;
}

void add_frame(symtable_t *table) {
    bst_t *new_frame = (bst_t *)malloc(sizeof(bst_t));
    frame_init(&(new_frame->root));
    new_frame->lower_frame = table->current_frame;
    table->current_frame = new_frame;
}


int height(bst_node_t *node) {
    if (node == NULL)
        return 0;
    return node->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

bst_node_t *right_rotate(bst_node_t *y) {
    bst_node_t *x = y->left;
    bst_node_t *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

bst_node_t *left_rotate(bst_node_t *x) {
    bst_node_t *y = x->right;
    bst_node_t *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

int get_balance(bst_node_t *node) {
    if (node == NULL)
        return 0;
    return height(node->left) - height(node->right);
}

bst_node_t *create_node(const char *key, item_type_t type, item_data_t data) {
    bst_node_t *node = (bst_node_t *)malloc(sizeof(bst_node_t));
    node->key = "\0";
    strcpy(node->key, key);
    node->type = type;
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

bst_node_t *insert_symbol(symtable_t *table, const char *key, item_type_t type, item_data_t data) {
    bst_node_t **tree = &(table->current_frame->root);
    bst_node_t *node = *tree;

    if (node == NULL) {
        *tree = create_node(key, type, data);
        return *tree;
    }

    int cmp = strcmp(key, node->key);
    if (cmp < 0) {
        node->left = insert_symbol(table, key, type, data);
    } else if (cmp > 0) {
        node->right = insert_symbol(table, key, type, data);
    } else {
        node->type = type;
        node->data = data;
        return node;
    }

    node->height = 1 + max(height(node->left), height(node->right));

    int balance = get_balance(node);

    if (balance > 1 && strcmp(key, node->left->key) < 0)
        return right_rotate(node);

    if (balance < -1 && strcmp(key, node->right->key) > 0)
        return left_rotate(node);

    if (balance > 1 && strcmp(key, node->left->key) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && strcmp(key, node->right->key) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

bst_node_t *find_symbol(symtable_t *table, const char *key) {
    bst_t *frame = table->current_frame;
    while (frame != NULL) {
        bst_node_t *node = frame->root;
        while (node != NULL) {
            int cmp = strcmp(key, node->key);
            if (cmp < 0) {
                node = node->left;
            } else if (cmp > 0) {
                node = node->right;
            } else {
                return node;
            }
        }
        frame = frame->lower_frame;
    }
    return NULL;
}

// void free_avl(bst_node_t *node) {
//     if (node != NULL) {
//         free_avl(node->left);
//         free_avl(node->right);
//         free(node->key);
//         if (node->type == ITEM_FUNCTION) {
//             free(node->data.function.arguments);
//         }
//         free(node);
//     }
// }

// void free_symTable(symtable_t *table) {
//     while (table->current_frame != NULL) {
//         remove_frame(table);
//     }
//     free(table->global_frame);
// }