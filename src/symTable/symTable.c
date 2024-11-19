#include "symTable.h"

FrameStack *init_frameStack(){
    FrameStack *new_frameStack = (FrameStack *)malloc(sizeof(FrameStack));
    if (new_frameStack == NULL) {
        error_exit(ERR_INTERNAL);
    }
    new_frameStack->top = NULL;
    return new_frameStack;
}

void add_frame(FrameStack *frameStack){
    //printf("Adding frame\n");   
    framePtr new_frame = (framePtr)malloc(sizeof(struct Frame));
    if (new_frame == NULL) {
        error_exit(ERR_INTERNAL);
    }
    new_frame->root = NULL;
    new_frame->nextFrame = frameStack->top;
    frameStack->top = new_frame;
}

// Utility function to get the height of the tree
int getHeight(Node *node) {
    return node ? node->height : 0;
}

// Update height of a node based on its children
void updateHeight(Node *node) {
    if (node) {
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);
        node->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    }
}

// Calculate balance factor
int getBalance(Node *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

// Perform a right rotation
Node *rotateRight(Node *y) {
    Node *x = y->left;
    Node *T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

// Perform a left rotation
Node *rotateLeft(Node *x) {
    Node *y = x->right;
    Node *T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

// Rebalance the tree
Node *rebalance(Node *node) {
    updateHeight(node);
    int balance = getBalance(node);

    // Left heavy
    if (balance > 1) {
        if (getBalance(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }
    // Right heavy
    if (balance < -1) {
        if (getBalance(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

// Insert a node and ensure the tree remains balanced
Node *insertNode(Node *node, Node *new_node) {
    if (node == NULL) {
        return new_node;
    }
    if (strcmp(new_node->id, node->id) < 0) {
        node->left = insertNode(node->left, new_node);
    } else if (strcmp(new_node->id, node->id) > 0) {
        node->right = insertNode(node->right, new_node);
    } else {
        error_exit(ERR_DEFINE); // Duplicate id
    }

    return rebalance(node);
}

void add_item(FrameStack *frameStack, Node *node) {
    if(searchTop(frameStack, node->id) != NULL){
        error_exit(ERR_UNDEF_VAR);
    }
    frameStack->top->root = insertNode(frameStack->top->root, node);
}

void add_item_fn(FrameStack *frameStack, Param *param) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        error_exit(ERR_INTERNAL);
    }
    node->id = param->id;
    node->type = param->type;
    node->fn = false;
    node->t_const = false;
    node->used = true;
    node->params = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    frameStack->top->root = insertNode(frameStack->top->root, node);
}

// Search for a node by id in the AVL tree
Node *searchNode(Node *root, const char *id) {
    
    if (root == NULL) {
        
        return NULL; // Not found
    }
    
    int cmp = strcmp(id, root->id);
    
    if (cmp == 0) {
        return root; // Found the node
    } else if (cmp < 0) {
        return searchNode(root->left, id); // Search in the left subtree
    } else {
        return searchNode(root->right, id); // Search in the right subtree
    }
}

Node *search(FrameStack *frameStack, const char *id) {
   //printf("Searching for %s\n", id);  
    if (frameStack->top == NULL) {
        return NULL; // Empty stack or tree
    }
    Node *result = searchNode(frameStack->top->root, id);
   
    if(result == NULL){
        framePtr current = frameStack->top->nextFrame;
        while (current != NULL) {
           
            result = searchNode(current->root, id);
            if(result != NULL){
                return result;
            }
            current = current->nextFrame;
        }
    }
    return result;
}
Node *searchTop(FrameStack *frameStack, const char *id) {
    if (frameStack->top == NULL) {
        return NULL; // Empty stack or tree
    }
    return searchNode(frameStack->top->root, id);
}

void printParams(Param *params) {
    Param *current = params;
    while (current != NULL) {
        printf("Param: TYPE:%s, ID:%s\n ", tokenName[current->type], current->id);
        current = current->next;
    }
}

void printNode(Node *node) {
    if (node != NULL) {
        printf("ID: %s, Type: %s, Const: %s, Function: %s, Used: %s, Height: %d\n",
               node->id,
               tokenName[node->type],
               node->t_const ? "True" : "False",
               node->fn ? "True" : "False",
               node->used ? "True" : "False",
               node->height);
        printParams(node->params);
    }
}

// In-order traversal (left-root-right)
void printInOrder(Node *root) {
    if (root != NULL) {
        printInOrder(root->left);
        printNode(root);
        printInOrder(root->right);
    }
}

void printFrameStack(FrameStack *frameStack) {
    framePtr current = frameStack->top;
    while (current != NULL) {
        printf("------Frame:-----\n");
        printInOrder(current->root);
        printf("-----------------\n");
        current = current->nextFrame;
    }
}

void removeFrame(FrameStack *frameStack) {
    if (frameStack->top == NULL) {
        return;
    }
   // framePtr temp = frameStack->top;
   check_used_flag(frameStack->top->root);
    frameStack->top = frameStack->top->nextFrame;
   // free(temp)
}
void set_usage(FrameStack *frameStack, const char *id){
    Node *node = search(frameStack, id);
    if(node == NULL){
        error_exit(ERR_UNDEF_VAR);
    }
    node->used = true;
}
void check_used_flag(Node *root) {
    if (root != NULL) {
        check_used_flag(root->left);
        if (!(root->used)) error_exit(ERR_VAR_OUT_OF_SCOPE);
        check_used_flag(root->right);
    }
}