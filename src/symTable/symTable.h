/**
* @file symTable.h
 * IFJ24 Compiler
 *
 * @brief Header file for the symbol table implementation. It contains type definitions and function
 *        declarations for managing the symbol table, including operations for adding, searching,
 *        and removing symbols, as well as handling the frame stack for different scopes. The symbol
 *        table is implemented as an AVL tree to ensure balanced and efficient lookups.
 *
 * @authors
 *          Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */


#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "../lexer/scanner.h"

typedef struct Param{
    TokenType type;
    char *id;
    struct Param *next;
} Param;

typedef struct Node{
    char *id;
    char *value;
    TokenType type;
    bool fn;
    bool t_const;
    bool used;
    Param *params;
	struct Node *left;
	struct Node *right;
	int height;
} Node;

typedef struct Frame{
    Node *root;
    struct Frame *nextFrame;
} *framePtr;

typedef struct {
    framePtr top;
} FrameStack;

FrameStack *init_frameStack();

// Function to add a new frame to the frame stack
void add_frame(FrameStack *frameStack);

// Function to add a new item to the frame stack
void add_item(FrameStack *frameStack, Node *node);

// Function to convert function parameter to a tree node
void add_item_fn(FrameStack *frameStack, Param *param);

// Function to search for a node by id in the AVL tree
Node *search(FrameStack *frameStack, const char *id);

// Function to search for a node by id in the AVL tree at the top of the frame stack
Node *searchTop(FrameStack *frameStack, const char *id);


void printInOrder(Node *root);
void printNode(Node *node);
void printFrameStack(FrameStack *frameStack);
void removeFrame(FrameStack *frameStack);

// Funtction to set the used flag of a node to true
void set_usage(FrameStack *frameStack, const char *id);

// Function to check if a node is used
void check_used_flag(Node *root);
void printNode(Node *node);

#endif //SYMTABLE_H