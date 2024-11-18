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
void add_frame(FrameStack *frameStack);
void add_item(FrameStack *frameStack, Node *node);
void add_item_fn(FrameStack *frameStack, Param *param);
Node *search(FrameStack *frameStack, const char *id);
Node *searchTop(FrameStack *frameStack, const char *id);
void printInOrder(Node *root);
void printNode(Node *node);
void printFrameStack(FrameStack *frameStack);
void removeFrame(FrameStack *frameStack);
void set_usage(FrameStack *frameStack, const char *id);
void check_used_flag(Node *root);
void printNode(Node *node);

#endif //SYMTABLE_H