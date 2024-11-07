#include "bottomUp.h"

char precedence_table[14][14] = {
    // (   )    +    -    *    /   ==   !=    <    >   <=    >=   i    $
    {'<', '=', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', ' '}, // (
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>'}, // )
    {'<', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // +
    {'<', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // -
    {'<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // *
    {'<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>'}, // /
    {'<', '>', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '<', '>'}, // ==
    {'<', '>', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '<', '>'}, // !=
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
        case TOKEN_E:
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
void parse_expression(Expression *expression) {
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
            return;
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
            reduce(&stack);
        } else if (precedence == ' ') {
            printf("Error: Invalid expression.\n");
            //free_stack(&stack);  // Free the stack before exiting
            error_exit(ERR_SYNTAX);
            return;

        } else if (precedence == 'V') {
            // Handle syntax error if precedence is invalid
            printf("Expression is valid.\n");
            //free_stack(&stack);  // Free the stack before exiting
            return;
        }
    }

    // Final reduction after the last input token if needed
    Token *stack_top_token = top(&stack);  // Check top of the stack
    if (stack_top_token != NULL && stack_top_token->type == TOKEN_E) {
        // If the stack top is an expression (E), and the input is '$', perform the final reduction
        Token *input_end_token = malloc(sizeof(Token));
        input_end_token->type = TOKEN_END;  // Simulate the end of input

        char final_precedence = get_precedence(stack_top_token->type, input_end_token->type);
        if (final_precedence == '>') {
            // Final reduction to complete parsing
            reduce(&stack);
            printf("Parsing completed successfully: Expression reduced to E.\n");
        } else {
            printf("Syntax error: Could not reduce at the end.\n");
        }

        //free(input_end_token);
    } else {
        printf("Syntax error: No valid expression found.\n");
    }

    //free_stack(&stack);  // Free memory used by the stack
}

void stack_reduce(Stack *stack) {
    Token *non_terminal = malloc(sizeof(Token));
    non_terminal->type = TOKEN_E;
    pop(stack);
    pop(stack);
    pop(stack);
    push(stack, non_terminal);
}

// Function to reduce the top elements of the stack based on grammar rules
void reduce(Stack *stack) {
    Token *non_terminal = malloc(sizeof(Token));
    non_terminal->type = TOKEN_E;  // Non-terminal E for expression
    Token *top_token = top(stack);  // Get the top token in the stack
    if (top_token == NULL) {
        printf("Error: Stack is empty, cannot reduce.\n");
        return;
    }

    if (top_token->type == TOKEN_IDENTIFIER || top_token->type == TOKEN_INT || top_token->type == TOKEN_FLOAT) {
        // E -> i
        printf("Reducing: E -> i\n");
        pop(stack);
        push(stack, non_terminal);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_PLUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E + E
        printf("Reducing: E -> E + E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MINUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E - E
        printf("Reducing: E -> E - E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MULTIPLY && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E * E
        printf("Reducing: E -> E * E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_DIVIDE && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E / E
        printf("Reducing: E -> E / E\n");
        stack_reduce(stack);
        print_stack(stack);
    } else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E < E
        printf("Reducing: E -> E < E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E > E
        printf("Reducing: E -> E > E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E <= E
        printf("Reducing: E -> E <= E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E >= E
        printf("Reducing: E -> E >= E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E == E
        printf("Reducing: E -> E == E\n");
        stack_reduce(stack);
        print_stack(stack);
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_NOT_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E != E
        printf("Reducing: E -> E != E\n");
        stack_reduce(stack);
        print_stack(stack);
    } else if (top_token->type == TOKEN_RIGHT_BRACKET && stack->top->nextElement->data->type == TOKEN_E && stack->top->nextElement->nextElement->data->type == TOKEN_LEFT_BRACKET) {
        // E -> (E)
        printf("Reducing: E -> (E)\n");
        stack_reduce(stack);
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
