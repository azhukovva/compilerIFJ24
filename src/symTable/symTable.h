#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "../lexer/scanner.h"

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
void add_frame(FrameStack *frameStack);
void add_item(FrameStack *frameStack, Node *node);
Node *search(FrameStack *frameStack, const char *id);
void printInOrder(Node *root);
void printNode(Node *node);
void printFrameStack(FrameStack *frameStack);

#endif //SYMTABLE_H