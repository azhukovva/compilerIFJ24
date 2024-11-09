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