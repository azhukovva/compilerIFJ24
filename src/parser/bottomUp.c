/**
 * @file bottomUp.c
 * IFJ24 Compiler
 *
 * @brief This file contains the implementation of the bottom-up parser. It contains function declarations
 *        for parsing expressions using shift-reduce parsing, handling various token types, and
 *        managing the precedence rules. The parser supports arithmetic, relational, and equality
 *        operations, as well as handling parentheses and different token types.
 *
 * @details The parser uses a precedence table to determine the actions (shift or reduce) based
 *          on the current token and the top of the stack.
 *
 * @authors Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

#include "bottomUp.h"

extern InstructionList *instructionList;

int intResCounter = 1;
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

// Function to get the precedence between two tokens
char get_precedence(TokenType stack_top, TokenType input) {
    char result = precedence_table[get_index(stack_top)][get_index(input)];
    //printf("Precedence between '%s' (stack top) and '%s' (input): %c\n", tokenName[stack_top], tokenName[input], result); // db purposes only
    return result;
}

// Function to get the index of a token in the precedence table
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
TokenType parse_expression(Expression *expression, FrameStack *frameStack, bool from_main) {
    TokenType result = TOKEN_EMPTY;
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
            //printf("Error: No terminal token found on the stack.\n"); // db purposes only
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
            reduce(&stack, frameStack, from_main);
        } else if (precedence == ' ') {
            //printf("Error: Invalid expression.\n"); // db purposes only
            error_exit(ERR_SYNTAX);
            return TOKEN_EMPTY;

        } else if (precedence == 'V') {
            //printf("Expression is valid.\n"); // db purposes only
            //printf("Expression datatype: %s\n", stack.top->data->value); // db purposes only

	        // If the expression is valid, return the type of the top element of the stack
	        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), "supa_giga_expr_res"), convert(top(&stack)->value, from_main), NULL);
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
    return result;
}


// Function to process a reduce action
void stack_reduce(Stack *stack, char *result, bool from_main) {
    // Non-terminal token to replace the reduced tokens
    Token *non_terminal = malloc(sizeof(Token));
	char *tempregister;
    non_terminal->type = TOKEN_E;
    non_terminal->value = result;

    // Handle the defining of temporary registers
	if (strstr(top(stack)->value, "$tmp") == NULL && strstr(top(stack)->nextElement->nextElement->data->value, "$tmp") == NULL) {
			tempregister = _strcat("$tmp", itoa(intResCounter));
			intResCounter++;
			build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
		} else if (strstr(top(stack)->value, "$tmp") != NULL) {
        	tempregister = top(stack)->value;
		} else if (strstr(top(stack)->nextElement->nextElement->data->value, "$tmp") != NULL) {
			tempregister = top(stack)->nextElement->nextElement->data->value;
		}

	// Prepare the operation instruction for output
	operation_instruction(stack, tempregister, from_main);

    // Pop the reduced tokens from the stack and push the non-terminal token
    pop(stack);
    pop(stack);
    pop(stack);
    push(stack, non_terminal, tempregister);
}


// Function to prepare the operation instruction for output
void operation_instruction(Stack *stack, char *tempregister, bool from_main) {
	TokenType operation = top(stack)->nextElement->data->type;
	char *left = top(stack)->nextElement->nextElement->value;
	char *right = top(stack)->value;
	switch(operation){
		case TOKEN_PLUS:
            build_instruction(instructionList, "ADD", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
		case TOKEN_MINUS:
			build_instruction(instructionList, "SUB", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
		case TOKEN_MULTIPLY:
			build_instruction(instructionList, "MUL", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
		case TOKEN_DIVIDE:
			build_instruction(instructionList, "DIV", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
	    case TOKEN_EQUAL:
		    build_instruction(instructionList, "EQ", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
	    case TOKEN_NOT_EQUAL:
		    build_instruction(instructionList, "EQ", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
			build_instruction(instructionList, "NOT", convert(tempregister, from_main), convert(tempregister, from_main), NULL);
            break;
        case TOKEN_LESS_THAN:
            build_instruction(instructionList, "LT", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
        case TOKEN_GREATER_THAN:
            build_instruction(instructionList, "GT", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
            break;
        case TOKEN_LESS_EQUAL:
            build_instruction(instructionList, "GT", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
		    build_instruction(instructionList, "NOT", convert(tempregister, from_main), convert(tempregister, from_main), NULL);
			break;
        case TOKEN_GREATER_EQUAL:
            build_instruction(instructionList, "LT", convert(tempregister, from_main), convert(left, from_main), convert(right, from_main));
			build_instruction(instructionList, "NOT", convert(tempregister, from_main), convert(tempregister, from_main), NULL);
            break;
		default:
            break;
	}
}

// Function to convert a token value to a specific format
char *convert(char *value, bool from_main) {
    char *result;
    if (strchr(value, '.') != NULL) {
        // If there's a '.' in value, add "float@"
		float tmp = atof(value);
		sprintf(value, "%a", tmp);
        result = _strcat("float@", value);
    } else if (isdigit(value[0])) {
        // If it's just a number, add "int@"
        result = _strcat("int@", value);
    } else if (strcmp(value, "null") == 0) {
        // If it's "null", add "nil@"
        result = "nil@nil";
    } else{
        // Otherwise, it's an identifier, add "LF@"
        result = _strcat(what_frame(from_main), value);
    }
    return result;
}

// Function to assign the proper type for a token based on its tokentype on frame
char *assign_type_frame(Token *topToken, FrameStack *frameStack) {
    Node *node = search(frameStack, topToken->value);
    if (node == NULL) {
        //printf("Error: Variable '%s' not found in the symbol table.\n", topToken->value); // db purposes only
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
            //printf("Error: Invalid type for variable '%s'.\n", topToken->value); // db purposes only
            error_exit(ERR_SYNTAX);
            return "-1";
    }
}

// Function to get the proper type for a token based on its token type
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
            //printf("Error: Invalid type for token.\n"); // db purposes only
            error_exit(ERR_SYNTAX);
            return "-1";
    }
}

// Function to check for operand types and return the result type for arithmetic operations (+, -, *, /)
char* arithmetic(Stack *stack, bool from_main) {
	char *tempregister;
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
        } else if (strstr(right->value, "float") != NULL && strstr(left->value, "int") != NULL) {
			tempregister = _strcat("$tmp", itoa(intResCounter));
			intResCounter++;
			build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->nextElement->nextElement->value, from_main), NULL);
			stack->top->nextElement->nextElement->value = tempregister;
			stack->top->nextElement->nextElement->data->value = "float";
            result = "float";
        } else if (strstr(left->value, "float") != NULL && strstr(right->value, "int") != NULL) {
			tempregister = _strcat("$tmp", itoa(intResCounter));
			intResCounter++;
			build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->value, from_main), NULL);
			stack->top->value = tempregister;
			stack->top->data->value = "float";
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

//Function to check for operand types and return the result type for == and != operations
char* eq(Stack *stack, bool from_main) {
	char *tempregister;
    Token *left = stack->top->nextElement->nextElement->data;
    Token *right = stack->top->data;
    if (((strcmp(right->value, "null") == 0) && (strchr(left->value, '?') != NULL)) ||
        ((strcmp(left->value, "null") == 0) && (strchr(right->value, '?') != NULL)) ||
        ((strcmp(left->value, "null") == 0) && (strcmp(right->value, "null") == 0))) {
        return "int";
    }
    if (strstr(left->value, "Const") != NULL || strstr(right->value, "Const") != NULL){
        //printf("Right: %s, Left: %s\n", right->value, left->value); // db purposes only
        if (strstr(left->value, right->value) != NULL) {
            return "int";
        } else if (strstr(right->value, left->value) != NULL){
            return "int";
        } else if ((strcmp(right->value, "floatConstConv") == 0 && strstr(left->value, "int") != NULL) ||
                  (strcmp(left->value, "floatConstConv") == 0 && strstr(right->value, "int") != NULL)) {
			if (strcmp(right->value, "floatConstConv") == 0) {
	            tempregister = _strcat("$tmp", itoa(intResCounter));
				intResCounter++;
				build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
				build_instruction(instructionList, "FLOAT2INT", convert(tempregister, from_main), convert(stack->top->value, from_main), NULL);
				stack->top->value = tempregister;
				stack->top->data->value = "intConst";
            } else {
	            tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "FLOAT2INT", convert(tempregister, from_main), convert(stack->top->nextElement->nextElement->value, from_main), NULL);
			    stack->top->nextElement->nextElement->value = tempregister;
			    stack->top->nextElement->nextElement->data->value = "intConst";
			}
			return "int";
        } else if ((strstr(right->value, "float") != NULL && strstr(left->value, "int") != NULL) ||
                  (strstr(left->value, "float") != NULL && strstr(right->value, "int") != NULL)) {
			if (strstr(right->value, "int") != NULL) {
                tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->value, from_main), NULL);
			    stack->top->value = tempregister;
			    stack->top->data->value = "float";
			} else {
                tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->nextElement->nextElement->value, from_main), NULL);
			    stack->top->nextElement->nextElement->value = tempregister;
			    stack->top->nextElement->nextElement->data->value = "float";
			}
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


// Function to check for operand types and return the result type for relational operations (<, >, <=, >=)
char* relational(Stack *stack, bool from_main) {
	char *tempregister;
    Token *left = stack->top->nextElement->nextElement->data;
    Token *right = stack->top->data;
    if ((strcmp(right->value, "null") == 0) || (strchr(left->value, '?') != NULL) ||
        (strcmp(left->value, "null") == 0) || (strchr(right->value, '?') != NULL)) {
        error_exit(ERR_EXPR_TYPE);
        return "null";
    }
    if (strstr(left->value, "Const") != NULL || strstr(right->value, "Const") != NULL){
        //printf("Right: %s, Left: %s\n", right->value, left->value); // db purposes only
        if (strstr(left->value, right->value) != NULL) {
            return "int";
        } else if (strstr(right->value, left->value) != NULL){
            return "int";
        } else if ((strcmp(right->value, "floatConstConv") == 0 && strstr(left->value, "int") != NULL) ||
                  (strcmp(left->value, "floatConstConv") == 0 && strstr(right->value, "int") != NULL)) {
			if (strcmp(right->value, "floatConstConv") == 0) {
	            tempregister = _strcat("$tmp", itoa(intResCounter));
				intResCounter++;
				build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
				build_instruction(instructionList, "FLOAT2INT", convert(tempregister, from_main), convert(stack->top->value, from_main), NULL);
				stack->top->value = tempregister;
				stack->top->data->value = "intConst";
            } else {
	            tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "FLOAT2INT", convert(tempregister, from_main), convert(stack->top->nextElement->nextElement->value, from_main), NULL);
			    stack->top->nextElement->nextElement->value = tempregister;
			    stack->top->nextElement->nextElement->data->value = "intConst";
			}
			return "int";
        } else if ((strstr(right->value, "float") != NULL && strstr(left->value, "int") != NULL) ||
                  (strstr(left->value, "float") != NULL && strstr(right->value, "int") != NULL)) {
			if (strstr(right->value, "int") != NULL) {
                tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->value, from_main), NULL);
			    stack->top->value = tempregister;
			    stack->top->data->value = "float";
			} else {
                tempregister = _strcat("$tmp", itoa(intResCounter));
			    intResCounter++;
			    build_instruction(instructionList, "DEFVAR", convert(tempregister, from_main), NULL, NULL);
			    build_instruction(instructionList, "INT2FLOAT", convert(tempregister, from_main), convert(stack->top->nextElement->nextElement->value, from_main), NULL);
			    stack->top->nextElement->nextElement->value = tempregister;
			    stack->top->nextElement->nextElement->data->value = "float";
			}
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
void reduce(Stack *stack, FrameStack *frameStack, bool from_main) {
    char *result = NULL;
    Token *non_terminal = malloc(sizeof(Token));
    non_terminal->type = TOKEN_E;  // Non-terminal E for expression
    Token *top_token = top(stack)->data;  // Get the top token in the stack
    if (top_token == NULL) {
        //printf("Error: Stack is empty, cannot reduce.\n"); // db purposes only
        return;
    }

    if (top_token->type == TOKEN_IDENTIFIER || top_token->type == TOKEN_INT ||
     top_token->type == TOKEN_FLOAT || top_token->type == TOKEN_NULL ||
      top_token->type == TOKEN_FLOAT_EXP || top_token->type == TOKEN_STRING) {
        // E -> i
        //printf("Reducing: E -> i\n"); // db purposes only
        if (top_token->type == TOKEN_IDENTIFIER) {
            non_terminal->value = assign_type_frame(top_token, frameStack);
        } else {
            non_terminal->value = assign_type(top_token);
        }
		char *tokenval = top_token->value;
        pop(stack);
        push(stack, non_terminal, tokenval);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_PLUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E + E
        //printf("Reducing: E -> E + E\n"); // db purposes only
        result = arithmetic(stack, from_main);
		stack_reduce(stack, result, from_main);
		//print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MINUS && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E - E
        //printf("Reducing: E -> E - E\n"); // db purposes only
        result = arithmetic(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_MULTIPLY && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E * E
        //printf("Reducing: E -> E * E\n"); // db purposes only
        result = arithmetic(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_DIVIDE && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E / E
        //printf("Reducing: E -> E / E\n"); // db purposes only
        result = arithmetic(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
	else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E < E
        //printf("Reducing: E -> E < E\n"); // db purposes only
        result = relational(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_THAN && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E > E
        //printf("Reducing: E -> E > E\n"); // db purposes only
        result = relational(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_LESS_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E <= E
        //printf("Reducing: E -> E <= E\n"); // db purposes only
        result = relational(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_GREATER_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E >= E
        //printf("Reducing: E -> E >= E\n"); // db purposes only
        result = relational(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E == E
        //printf("Reducing: E -> E == E\n"); // db purposes only
        result = eq(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
    else if (top_token->type == TOKEN_E && stack->top->nextElement->data->type == TOKEN_NOT_EQUAL && stack->top->nextElement->nextElement->data->type == TOKEN_E) {
        // E -> E != E
        //printf("Reducing: E -> E != E\n"); // db purposes only
        result = eq(stack, from_main);
        stack_reduce(stack, result, from_main);
        //print_stack(stack); // db purposes only
    }
	else if (top_token->type == TOKEN_RIGHT_BRACKET && stack->top->nextElement->data->type == TOKEN_E && stack->top->nextElement->nextElement->data->type == TOKEN_LEFT_BRACKET) {
        // E -> (E)
        //printf("Reducing: E -> (E)\n"); // db purposes only
		Token *non_terminal = malloc(sizeof(Token));
    	non_terminal->type = TOKEN_E;
    	non_terminal->value = stack->top->nextElement->data->value;
		char *val = top(stack)->nextElement->value;
	    pop(stack);
        pop(stack);
        pop(stack);
        push(stack, non_terminal, val);
        //print_stack(stack); // db purposes only
    } else {
        //printf("Error: Invalid expression.\n"); // db purposes only
        error_exit(ERR_SYNTAX);
    }
}

// Function to shift a token onto the stack
void shift(Stack *stack, Token *token) {
    //printf("Shifting: %s\n", tokenName[token->type]); // db purposes only
    push(stack, token, NULL);  // Push the item onto the stack
    //print_stack(stack); // db purposes only
}
