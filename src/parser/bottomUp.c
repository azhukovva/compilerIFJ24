#include "bottomUp.h"

extern InstructionList *instructionList;
char precedence_table[14][14] = {
    // (   )    +    -    *    /   ==   !=    <    >   <=    >=   i    $
    {'<', '=', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', ' '}, // (
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>'}, // )
    {'<', '>', '>', '>', '<', '<', '>', '>', '>', '>', '>', '>', '<', '>'}, // +
    {'<', '>', '>', '>', '<', '<', '>', '>', '>', '>', '>', '>', '<', '>'}, // -
    {'<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // *
    {'<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // /
    {'<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>'}, // ==
    {'<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>'}, // !=
    {'<', '>', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>'}, // <
    {'<', '>', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>'}, // >
    {'<', '>', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>'}, // <=
    {'<', '>', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>'}, // >=
    {' ', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>'}, // i
    {'<', ' ', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', 'V'}  // $
};

char get_precedence(TokenType stack_top, TokenType input) {
    char result = precedence_table[get_index(stack_top)][get_index(input)];
    printf("Precedence between '%s' (stack top) and '%s' (input): %c\n", tokenName[stack_top], tokenName[input], result);
    return result;
}


int get_index(TokenType type){
    switch (type){
        case TOKEN_LEFT_BRACKET:
            return 0;
            break;
        case TOKEN_RIGHT_BRACKET:
            return 1;
            break;
        case TOKEN_PLUS:
            return 2;
            break;
        case TOKEN_MINUS:
            return 3;
            break;
        case TOKEN_MULTIPLY:
            return 4;
            break;
        case TOKEN_DIVIDE:
            return 5;
            break;
        case TOKEN_EQUAL:
            return 6;
            break;
        case TOKEN_NOT_EQUAL:
            return 7;
            break;
        case TOKEN_LESS_THAN:
            return 8;
            break;
        case TOKEN_GREATER_THAN:
            return 9;
            break;
        case TOKEN_LESS_EQUAL:
            return 10;
            break;
        case TOKEN_GREATER_EQUAL:
            return 11;
            break;
        case TOKEN_IDENTIFIER:
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_FLOAT_EXP:
        case TOKEN_STRING:
        case TOKEN_NULL:
            return 12;
            break;
        case TOKEN_END:
            return 13;
            break;
        default:
            return -1;
            break;
    }
}

// Function to parse an expression using shift-reduce parsing
TokenType parse_expression(Expression *expression, FrameStack *frameStack) {
    TokenType result = TOKEN_EMPTY;
    // Initialize the stack
    Stack stack;
    init_stack(&stack);

    // Push the end-of-input symbol ('$') onto the stack
    Token *end_of_input =  malloc(sizeof(Token));
    end_of_input->type = TOKEN_END;  // '$' represents the end of input
    end_of_input->value = "$";
    shift(&stack, end_of_input);  // Shift the end-of-input symbol onto the stack

    // Start processing the input
    ExpElementPtr current = expression->firstElement;  // Start from the first element
    shift(&stack, current->data);  // Shift the first token onto the stack
    current = current->nextElement;

    // Process the input tokens
    while (current != NULL) {
        // Get the first terminal token on the stack
        Token *first_terminal_token = NULL;
        StackElementPtr stack_element = stack.top;

        // Traverse the stack to find the first terminal token
        while (stack_element != NULL) {
            if (stack_element->data->type != TOKEN_E) {
                first_terminal_token = stack_element->data;
                break;
            }
            stack_element = stack_element->nextElement;
        }

        // If no terminal token is found, we can't proceed with precedence check
        if (first_terminal_token == NULL) {
            printf("Error: No terminal token found on the stack.\n");
            //free_stack(&stack);
            return TOKEN_EMPTY;
        }

        TokenType stack_top_type = first_terminal_token->type;  // Type of first terminal
        TokenType input_token_type = current->data->type;  // Type of input token

        // Get precedence of the first terminal element vs. current input token
        char precedence = get_precedence(stack_top_type, input_token_type);
        if (precedence == '<' || precedence == '=') {
            // Shift action: push the current token onto the stack
            shift(&stack, current->data);
            current = current->nextElement;  // Move to the next token
        } else if (precedence == '>') {
            // Reduce action: apply reduction
            reduce(&stack, frameStack);
        } else if (precedence == ' ') {
            printf("Error: Invalid expression.\n");
            //free_stack(&stack);  // Free the stack before exiting
            error_exit(ERR_SYNTAX);
            return TOKEN_EMPTY;

        } else if (precedence == 'V') {
            // Handle syntax error if precedence is invalid
            printf("Expression is valid.\n");
            //free_stack(&stack);  // Free the stack before exiting
            printf("Expression datatype: %s\n", stack.top->data->value);
            if (strstr(stack.top->data->value,"int") != NULL) {
                if(strcmp(stack.top->data->value,"int?") == 0){
                    return TOKEN_I32_OPT;
                } else {
                    return TOKEN_I32;
                }
            } else if (strstr(stack.top->data->value,"float") != NULL) {
                if(strcmp(stack.top->data->value,"float?") == 0){
                    return TOKEN_F64_OPT;
                } else {
                    return TOKEN_F64;
                }
            } else if (strstr(stack.top->data->value,"u8") != NULL) {
                if(strcmp(stack.top->data->value,"u8?") == 0){
                    return TOKEN_U8_OPT;
                } else {
                    return TOKEN_U8;
                }
            } else {
                return TOKEN_NULL;
            }
        }
    }
    // TODO: $[]u8$ :)
    // if (stack == "$[]u8$") { :) }
    return result;
    //free_stack(&stack);  // Free memory used by the stack
}

void stack_reduce(Stack *stack, char *result) {
    Token *non_terminal = malloc(sizeof(Token));
    non_terminal->type = TOKEN_E;
    non_terminal->value = result;
    pop(stack);
    pop(stack);
    pop(stack);
    push(stack, non_terminal);
}

char *assign_type_frame(Token *topToken, FrameStack *frameStack) {
    Node *node = search(frameStack, topToken->value);
    if (node == NULL) {
        printf("Error: Variable '%s' not found in the symbol table.\n", topToken->value);
        error_exit(ERR_UNDEF_VAR);
    }
    switch (node->type) {
        case TOKEN_I32:
            if (node->t_const) {

                return "intConst";
            } else {
                return "int";
            }
        case TOKEN_F64:
            if (node->t_const) {
                return "floatConst";
            } else {
                return "float";
            }
        case TOKEN_I32_OPT:
            return "int?";
        case TOKEN_F64_OPT:
            return "float?";
        case TOKEN_U8:
            return "u8";
        case TOKEN_U8_OPT:
            return "u8?";
        default:
            printf("Error: Invalid type for variable '%s'.\n", topToken->value);
            error_exit(ERR_SYNTAX);
            return "-1";
    }
}

char *assign_type(Token *topToken) {
    switch (topToken->type) {
        case TOKEN_INT:
            return "intConst";
        case TOKEN_FLOAT_EXP:
        case TOKEN_FLOAT:
            if (atof(topToken->value) == (int)atof(topToken->value)) {
                return "floatConstConv";
            } else {
                return "floatConst";
            }
        case TOKEN_NULL:
            return "null";
        case TOKEN_E:
            return topToken->value;
        case TOKEN_STRING:
            return "u8Const";
        default:
            printf("Error: Invalid type for token.\n");
            error_exit(ERR_SYNTAX);
            return "-1";
    }
}

char* arithmetic(Stack *stack) {
    Token *left = stack->top->nextElement->nextElement->data;
    Token *right = stack->top->data;
    char *result = NULL;
    if(strchr(right->value, '?') != NULL ||
       strchr(left->value, '?') != NULL ||
       strcmp(right->value, "null") == 0 ||
       strcmp(left->value, "null") == 0) error_exit(ERR_EXPR_TYPE);
    if(right->value != left->value){
        if (strstr(left->value, right->value) != NULL) {
            if(strstr(left->value, "float") != NULL){
                result = "float";
            } else {
                result = "int";
            }
        } else if (strstr(right->value, left->value) != NULL){
            if(strstr(right->value, "float") != NULL){
                result = "float";
            } else {
                result = "int";
            }
        } else {
            printf("mama sdohla tri\n");
            result = "float";
        }
        if (strstr(right->value, "Const") != NULL && strstr(left->value, "Const") != NULL) {
            char *temp_result = (char *)malloc(strlen(result) + strlen("Const") + 1);
            if (temp_result == NULL) {
                error_exit(ERR_INTERNAL);
            }
            strcpy(temp_result, result);
            strcat(temp_result, "Const");
            result = temp_result;
        }
        return result;
    } else {
        return assign_type(right);
    }
}

char* eq(Stack *stack) {
    Token *left = stack->top->nextElement->nextElement->data;
    Token *right = stack->top->data;
    if (((strcmp(right->value, "null") == 0) && (strchr(left->value, '?') != NULL)) ||
        ((strcmp(left->value, "null") == 0) && (strchr(right->value, '?') != NULL)) ||
        ((strcmp(left->value, "null") == 0) && (strcmp(right->value, "null") == 0))) {
        return "int";
    }
    if (strstr(left->value, "Const") != NULL || strstr(right->value, "Const") != NULL){
        printf("Right: %s, Left: %s\n", right->value, left->value);
        if (strstr(left->value, right->value) != NULL) {
            return "int";
        } else if (strstr(right->value, left->value) != NULL){
            return "int";
        } else if ((strcmp(right->value, "floatConstConv") == 0 && strstr(left->value, "int") != NULL) ||
                  (strcmp(left->value, "floatConstConv") == 0 && strstr(right->value, "int") != NULL)) {
            return "int";
        } else if ((strstr(right->value, "float") != NULL && strstr(left->value, "int") != NULL) ||
                  (strstr(left->value, "float") != NULL && strstr(right->value, "int") != NULL)) {
            return "int";
        } else {
            error_exit(ERR_EXPR_TYPE);
            return "null";
        }
    } else {
        error_exit(ERR_EXPR_TYPE);
        return "null";
    }
}

char* relational(Stack *stack) {
    Token *left = stack->top->nextElement->nextElement->data;
    Token *right = stack->top->data;
    if ((strcmp(right->value, "null") == 0) || (strchr(left->value, '?') != NULL) ||
        (strcmp(left->value, "null") == 0) || (strchr(right->value, '?') != NULL)) {
        error_exit(ERR_EXPR_TYPE);
        return "null";
    }
    if (strstr(left->value, "Const") != NULL || strstr(right->value, "Const") != NULL){
        printf("Right: %s, Left: %s\n", right->value, left->value);
        if (strstr(left->value, right->value) != NULL) {
            return "int";
        } else if (strstr(right->value, left->value) != NULL){
            return "int";
        } else if ((strcmp(right->value, "floatConstConv") == 0 && strstr(left->value, "int") != NULL) ||
                  (strcmp(left->value, "floatConstConv") == 0 && strstr(right->value, "int") != NULL)) {
            return "int";
        } else if ((strstr(right->value, "float") != NULL && strstr(left->value, "int") != NULL) ||
                  (strstr(left->value, "float") != NULL && strstr(right->value, "int") != NULL)) {
            return "int";
        } else {
            error_exit(ERR_EXPR_TYPE);
            return "null";
        }
    } else {
        error_exit(ERR_EXPR_TYPE);
        return "null";
    }
}

// Function to reduce the top elements of the stack based on grammar rules
void reduce(Stack *stack, FrameStack *frameStack) {
    char *result = NULL;
    Token *non_terminal = malloc(sizeof(Token));
    non_terminal->type = TOKEN_E;  // Non-terminal E for expression
    Token *top_token = top(stack);  // Get the top token in the stack
    if (top_token == NULL) {
        printf("Error: Stack is empty, cannot reduce.\n");
        return;
    }

    if (top_token->type == TOKEN_IDENTIFIER || top_token->type == TOKEN_INT ||
     top_token->type == TOKEN_FLOAT || top_token->type == TOKEN_NULL ||
      top_token->type == TOKEN_FLOAT_EXP || top_token->type == TOKEN_STRING) {
        // E -> i
        printf("Reducing: E -> i\n");
        if (top_token->type == TOKEN_IDENTIFIER) {
            non_terminal->value = assign_type_frame(top_token, frameStack);
        } else {
            non_terminal->value = assign_type(top_token);
        }
        pop(stack);
        push(stack, non_terminal);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_PLUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E + E
        printf("Reducing: E -> E + E\n");
        result = arithmetic(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MINUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E - E
        printf("Reducing: E -> E - E\n");
        result = arithmetic(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MULTIPLY && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E * E
        printf("Reducing: E -> E * E\n");
        result = arithmetic(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_DIVIDE && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E / E
        printf("Reducing: E -> E / E\n");
        result = arithmetic(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    } else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E < E
        printf("Reducing: E -> E < E\n");
        result = relational(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E > E
        printf("Reducing: E -> E > E\n");
        result = relational(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E <= E
        printf("Reducing: E -> E <= E\n");
        result = relational(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E >= E
        printf("Reducing: E -> E >= E\n");
        result = relational(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E == E
        printf("Reducing: E -> E == E\n");
        result = eq(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_NOT_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E != E
        printf("Reducing: E -> E != E\n");
        result = eq(stack);
        stack_reduce(stack, result);
        print_stack(stack);
    } else if (top_token->type == TOKEN_RIGHT_BRACKET && stack->top->nextElement->data->type == TOKEN_E && stack->top->nextElement->nextElement->data->type == TOKEN_LEFT_BRACKET) {
        // E -> (E)
        printf("Reducing: E -> (E)\n");
        result = stack->top->nextElement->data->value;
        stack_reduce(stack, result);
        print_stack(stack);
    } else {
        printf("Error: Invalid expression.\n");
        error_exit(ERR_SYNTAX);
    }
}

void shift(Stack *stack, Token *token) {
    printf("Shifting: %s\n", tokenName[token->type]);
    push(stack, token);  // Push the item onto the stack
    print_stack(stack);
}
