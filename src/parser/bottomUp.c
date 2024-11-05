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
    return precedence_table[stack_top][input];
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

void parse_expression(Expression *expression)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    init_stack(stack);
    Token *endToken = init_token();
    endToken->type = TOKEN_END;
    endToken->value = "$";
    push(stack, endToken);
    ExpElementPtr current = expression->firstElement;
    while(current != NULL){
        Token *token = current->data;
        if(token->type == TOKEN_IDENTIFIER || token->type == TOKEN_INT || token->type == TOKEN_FLOAT){
            push(stack, token);
            current = current->nextElement;
        }
    }
    reduce(stack);
    print_stack(stack);
}

void reduce(Stack *s) {
    // E -> i
    Token *identifier = pop(s);
    if (identifier->type != TOKEN_IDENTIFIER && identifier->type != TOKEN_INT && identifier->type != TOKEN_FLOAT) {
        error_exit(ERR_SYNTAX);
    }

    // Create a new token for the non-terminal E
    Token *nonTerminal = (Token *)malloc(sizeof(Token));
    if (nonTerminal == NULL) {
        error_exit(ERR_INTERNAL);
    }
    nonTerminal->type = TOKEN_E;
    nonTerminal->value = "E";

    push(s, nonTerminal);
}