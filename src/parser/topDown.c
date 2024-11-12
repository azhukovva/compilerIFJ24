#include "topDown.h"



// extern const char *tokenName[];
 
//Current token pointer
Token *all_tokens[100000];
FrameStack *frameStack;
int token_index = 0;
bool encoutered_main = false;

//Get the next token
void next_token(){
    token_index++;
}
void save_token(Token *current_token){
    Token *token_copy = (Token *)malloc(sizeof(Token));
    if (token_copy == NULL) {
        error_exit(ERR_INTERNAL);
    }
    *token_copy = *current_token;
    all_tokens[token_index] = token_copy;
    token_index++;
}

bool is_data_type(TokenType type){
    return type == TOKEN_I32 || type == TOKEN_F64 || type == TOKEN_U8 ||
           type == TOKEN_I32_OPT || type == TOKEN_F64_OPT || type == TOKEN_U8_OPT || type == TOKEN_VOID; 
}

void fill_sym_table_fn(FrameStack *frameStack, int token_index) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        error_exit(ERR_INTERNAL);
    }
    node->id = NULL;
    node->type = TOKEN_EMPTY; // Replace with appropriate TokenType
    node->fn = false;
    node->t_const = false;
    node->params = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;

    if (all_tokens[token_index]->type == TOKEN_FN) {
        token_index++; // Skip `fn`
        // Allocate memory for node->id
        node->id = (char *)malloc(strlen(all_tokens[token_index]->value) + 1);
        if (node->id == NULL) {
            error_exit(ERR_INTERNAL);
        }
        strcpy(node->id, all_tokens[token_index]->value);
        printf("ID: %s\n", node->id);

        node->fn = true;
        node->t_const = false;
        token_index += 2; // Skip id and (

        // Parse parameters
        Param *last_param = NULL;
        while (all_tokens[token_index]->type != TOKEN_RIGHT_BRACKET) {
            if (all_tokens[token_index]->type == TOKEN_IDENTIFIER) {
                Param *new_param = (Param *)malloc(sizeof(Param));
                if (new_param == NULL) {
                    error_exit(ERR_INTERNAL);
                }
                new_param->id = (char *)malloc(strlen(all_tokens[token_index]->value) + 1);
                new_param->id = all_tokens[token_index]->value;
                new_param->type = all_tokens[token_index+2]->type;
                new_param->next = NULL;

                if (last_param == NULL) {
                    node->params = new_param;
                } else {
                    last_param->next = new_param;
                }
                last_param = new_param;
            }
            token_index++;
        }

        // Parse return type
        while (all_tokens[token_index]->type != TOKEN_LEFT_BRACE) {
            if (is_data_type(all_tokens[token_index]->type)) {
                node->type = all_tokens[token_index]->type;
                break;
            }
            token_index++;
        }

        // Add node to the tree
        add_item(frameStack, node);
    }
}


//Error handling 
void syntax_error(){
    error_exit(ERR_SYNTAX);
}

//i think we'll be handling comments in the expect function
//Compare current token type and expected token type. Get the next token if they match.
void skip_comments(){
   
    while(all_tokens[token_index]->type == TOKEN_COMMENT){
        printf("Skipping comment\n");
        next_token();
    }
}
void expect(TokenType type){
    //printf("Checking: type: %s val: %s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value);
    //if we get a comment we skip it and get the next token
    skip_comments();
    //printf("Checking: type: %s val: %s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value);
    if(all_tokens[token_index]->type == type){
        next_token();
    } else {
        printf("Expected: %s\n", tokenName[type]);
        syntax_error();
    }
}

//Check if the id of token value matches expected one
void expect_id(char *id_to_check){
    //printf("type: %s val: %s id_to_check:%s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value, id_to_check);
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER && !strcmp(all_tokens[token_index]->value, id_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the string in token matches expected one
void expect_string(char *string_to_check){
    //printf("type: %s val: %s id_to_check:%s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value, string_to_check);
    if(all_tokens[token_index]->type == TOKEN_STRING && !strcmp(all_tokens[token_index]->value, string_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

// <Program> ::= <Prologue> <functions> <main_func> eof
void program_rule(){
    prologue_rule();
    functions_rule();
    //main_func_rule();
    expect(TOKEN_EOF);
}
// <Prologue> ::= const ifj = @import(“ifj24.zig”) ;
void prologue_rule(){
    expect(TOKEN_CONST); 
    expect_id("ifj"); //ifj
    expect(TOKEN_ASSIGN);
    expect(TOKEN_AT);
    expect_id("import"); //import
    expect(TOKEN_LEFT_BRACKET);
    expect_string("ifj24.zig"); //ifj24.zig
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
}
// <functions> ::= <function> <functions_tail>
void functions_rule(){
    function_rule();
    functions_tail_rule();
}

// <functions_tail> ::= <function> <functions_tail> | eps
void functions_tail_rule(){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_PUB){
        function_rule();
        functions_tail_rule();
    }
}

// <function> ::= pub fn func_id <Param_list> <Return_type> <Block>
void function_rule(){
    expect(TOKEN_PUB);
    expect(TOKEN_FN);
    if(!strcmp(all_tokens[token_index]->value, "main")){
        main_func_rule();
    } else {
        Node *fn = search(frameStack, all_tokens[token_index]->value);
        expect(TOKEN_IDENTIFIER);
        param_list_rule();
        return_type_rule();
        block_rule_fn(fn);
    }
}

//<main_func> ::= pub fn main () void <Block>
void main_func_rule(){
    expect_id("main");
    expect(TOKEN_LEFT_BRACKET);
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_VOID);
    block_rule();
    encoutered_main = true;
}

//<Param_list> ::= ( <Params> ) 
void param_list_rule(){
    expect(TOKEN_LEFT_BRACKET);
    params_rule();
    expect(TOKEN_RIGHT_BRACKET);
}

// <Params> ::= <Param> <Param_tail> | eps
void params_rule(){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        param_rule();
        param_tail_rule();
    }
}

// <Param_tail> ::= , <Param> <Param_tail> | eps
void param_tail_rule(){
    // ,
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_COMMA){
        expect(TOKEN_COMMA);
        param_rule();
        param_tail_rule();
    }
    //eps
}

// <Param> ::= param_id : <Param_type>
void param_rule(){
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_COLON);
    base_type_rule();
}



// <Base_type> ::= i32 | f64 | []u8
void base_type_rule(){
    switch (all_tokens[token_index]->type)
    {
    case TOKEN_I32:
        expect(TOKEN_I32);
        break;
    case TOKEN_F64:
        expect(TOKEN_F64);
        break;
    case TOKEN_U8:
        expect(TOKEN_U8);
        break;
    case TOKEN_I32_OPT:
        expect(TOKEN_I32_OPT);
        break;
    case TOKEN_F64_OPT:
        expect(TOKEN_F64_OPT);
        break;
    case TOKEN_U8_OPT:
        expect(TOKEN_U8_OPT);
        break;
    default:
        break;
    }
}

// <Return_type> ::= <Optional_question_mark> <Base_type>
void return_type_rule(){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_VOID){
        expect(TOKEN_VOID);
    } else {
        base_type_rule();
    }
}

// <Block> ::= { <Statements> } 
void block_rule_fn(Node *fn){
    expect(TOKEN_LEFT_BRACE);
    add_frame(frameStack);
    while(fn->params != NULL){
        add_item_fn(frameStack, fn->params);
        fn->params = fn->params->next;
    }
    printf("entering statements\n");
    statements_rule();
    expect(TOKEN_RIGHT_BRACE);
    printFrameStack(frameStack);
    removeFrame(frameStack);
}

void block_rule(){
    expect(TOKEN_LEFT_BRACE);
    skip_comments();
    add_frame(frameStack);
    printf("entering statements\n");
    statements_rule();
    expect(TOKEN_RIGHT_BRACE);
    printFrameStack(frameStack);
    removeFrame(frameStack);
}

// <Statements> ::= <Statement> <Statements> | eps
void statements_rule(){
    while(is_statement_start(all_tokens[token_index]->type)){
        statement_rule();
    }

    printf("exiting statements\n");
   
}

// Function that defines whether or not we have any statements to parse
bool is_statement_start(TokenType type){
    skip_comments();
    return type == TOKEN_CONST || type == TOKEN_VAR || type == TOKEN_IDENTIFIER ||
           type == TOKEN_IF || type == TOKEN_WHILE || type == TOKEN_RETURN || type == TOKEN_IDENTIFIER_FUNC;
}

void statement_rule(){
        skip_comments();
        switch (all_tokens[token_index]->type){
        case TOKEN_CONST:
        case TOKEN_VAR:
            printf("var\n");
            var_rule();
            break;
        // assigment or function call
        case TOKEN_IDENTIFIER_FUNC:
        case TOKEN_IDENTIFIER:
            printf("assigment\n");
            assigment_rule();
            break;
        case TOKEN_IF:
            conditionals_rule();
            break;
        case TOKEN_WHILE:
            while_statement_rule();
            break;
        case TOKEN_RETURN:
            return_statement_rule();
            break; 
        default:
            printf("XD?\n");
            syntax_error();
            break;
        
    }
}

//<var_def> ::= <Var_mode> id <Var_type> = <Expression> ;
void var_rule(){
    skip_comments();
    Node *variable = (Node *)malloc(sizeof(Node));
    variable->height = 1;
    if(all_tokens[token_index]->type == TOKEN_CONST){
        expect(TOKEN_CONST);
        variable->t_const = true;
    } else if(all_tokens[token_index]->type == TOKEN_VAR){
        expect(TOKEN_VAR);
        variable->t_const = false;
    } else {
        syntax_error();
    }
    variable->fn = false;
    variable->id = all_tokens[token_index]->value;
    expect(TOKEN_IDENTIFIER);
    if(all_tokens[token_index]->type == TOKEN_COLON){
        expect(TOKEN_COLON);
        variable->type = all_tokens[token_index]->type;
        base_type_rule();
    }

    expect(TOKEN_ASSIGN);
        //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        expect(TOKEN_IDENTIFIER_FUNC);
        //zaglushka
        variable->type = TOKEN_I32;
        add_item(frameStack, variable);
        function_call_rule();
        return;
    }
    //expression_rule();
    //дальше пиздец)
     Token *token_copy = (Token *)malloc(sizeof(Token));
        if (token_copy == NULL) {
            error_exit(ERR_INTERNAL);
        }
    *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        variable->type = search(frameStack, all_tokens[token_index-1]->value)->type;
        add_item(frameStack, variable);
        function_call_rule();
        return;
    }
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
	if (token_copy->type == TOKEN_IDENTIFIER) add_element(expr, token_copy);
    while(all_tokens[token_index]->type != TOKEN_SEMICOLON){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        add_element(expr, all_tokens[token_index]);
        next_token();
    }
	Token *end_token = (Token *)malloc(sizeof(Token));
	if (end_token == NULL) {
        error_exit(ERR_INTERNAL);
    }
	end_token->type = TOKEN_END;
	end_token->value = "$";
	add_element(expr, end_token);
// //TODO free endtoken
    print_expression(expr);
    parse_expression(expr);
    add_item(frameStack, variable);
    expect(TOKEN_SEMICOLON);
    // free(token_copy);
    //free_expression(expr);
}



//<assigment> ::= id = <Expression> ;
void assigment_rule(){
    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        expect(TOKEN_IDENTIFIER_FUNC);
        function_call_rule();
        return;
    } 
    if(search(frameStack, all_tokens[token_index]->value) == NULL){
        error_exit(ERR_UNDEF_VAR);
    }

    expect(TOKEN_IDENTIFIER);

    //if the identifier was a user function identifier
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    expect(TOKEN_ASSIGN);
    
    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        expect(TOKEN_IDENTIFIER_FUNC);
        function_call_rule();
        return;
    }
    //expression_rule();
    //дальше пиздец)
     Token *token_copy = (Token *)malloc(sizeof(Token));
        if (token_copy == NULL) {
            error_exit(ERR_INTERNAL);
        }
    *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_UNDEF_VAR);
        }
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
	if (token_copy->type == TOKEN_IDENTIFIER) add_element(expr, token_copy);
    while(all_tokens[token_index]->type != TOKEN_SEMICOLON){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
            if(search(frameStack, all_tokens[token_index]->value) == NULL){
                error_exit(ERR_UNDEF_VAR);
            }
        }
        add_element(expr, all_tokens[token_index]);
        next_token();
    }
	Token *end_token = (Token *)malloc(sizeof(Token));
	if (end_token == NULL) {
        error_exit(ERR_INTERNAL);
    }
	end_token->type = TOKEN_END;
	end_token->value = "$";
	add_element(expr, end_token);
// //TODO free endtoken
    print_expression(expr);
    parse_expression(expr);
    expect(TOKEN_SEMICOLON);
    // free(token_copy);
    //free_expression(expr);
}

//<function_call> ::= ( <Arguments> );
void function_call_rule(){
    printf("function call\n");
    Node *signature = NULL;
    token_index--;
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        expect(TOKEN_IDENTIFIER_FUNC);
    } else {
        signature = search(frameStack, all_tokens[token_index]->value);
        if(signature == NULL){
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER);
    }
    expect(TOKEN_LEFT_BRACKET);
    arguments_rule();
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
    printf("function call end\n");
    skip_comments();
}

//<Arguments> ::= <Argument> <Arguments_tail>
void arguments_rule(){
    if(all_tokens[token_index]->type != TOKEN_RIGHT_BRACKET){
        argument_rule();
        arguments_tail_rule();
    }
}

//<Arguments_tail> ::= , <Argument> <Arguments_tail> | eps
void arguments_tail_rule(){
    if(all_tokens[token_index]->type == TOKEN_COMMA){
        expect(TOKEN_COMMA);
        if(all_tokens[token_index]->type == TOKEN_RIGHT_BRACKET){ // in case there is a comma before the last argument
            return;
        }
        argument_rule();
        arguments_tail_rule();
    }
}

//<Argument> ::= id | <Literal>
void argument_rule(){
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
    } else {
        literal_rule();
    }
}

//<Literal> ::= int | float | string
void literal_rule(){
    switch (all_tokens[token_index]->type) {
    case TOKEN_INT:
        expect(TOKEN_INT);
        break;
    case TOKEN_FLOAT:
        expect(TOKEN_FLOAT);
        break;
    case TOKEN_STRING:
        expect(TOKEN_STRING);
        break;
    default:
        syntax_error();
        break;
    }
}

//<Conditionals> ::= if ( Expression ) <Optional_null> <Block> <Optional_else>
void conditionals_rule(){
    expect(TOKEN_IF);
    expect(TOKEN_LEFT_BRACKET);
    //заглушка
    printf("conditionals\n");
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
    while(all_tokens[token_index]->type != TOKEN_RIGHT_BRACE || all_tokens[token_index]->type != TOKEN_PIPE){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
            if(search(frameStack, all_tokens[token_index]->value) == NULL){
                error_exit(ERR_UNDEF_VAR);
            }
        }
        if(all_tokens[token_index]->type == TOKEN_RIGHT_BRACKET){
            Token *token_copy = (Token *)malloc(sizeof(Token));
            if (token_copy == NULL) {
                error_exit(ERR_INTERNAL);
            }
            *token_copy = *all_tokens[token_index];
            expect(TOKEN_RIGHT_BRACKET);
            if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE || all_tokens[token_index]->type == TOKEN_PIPE){
                break;
            }
            add_element(expr, token_copy);
        }
        add_element(expr, all_tokens[token_index]);
        next_token();
    }
    Token *end_token = (Token *)malloc(sizeof(Token));
	if (end_token == NULL) {
        error_exit(ERR_INTERNAL);
    }
	end_token->type = TOKEN_END;
	end_token->value = "$";
	add_element(expr, end_token);
    print_expression(expr);
    parse_expression(expr);
// //TODO free endtoken
    //expect(TOKEN_RIGHT_BRACKET);
    optional_null_rule();
    block_rule();
    optional_else_rule();
}

//<Optional_null> ::= |id| | eps
void optional_null_rule(){
   if(all_tokens[token_index]->type == TOKEN_PIPE){
       expect(TOKEN_PIPE);
       Node *variable = (Node *)malloc(sizeof(Node));
        variable->height = 1;
        variable->t_const = false;
        variable->fn = false;
        variable->id = all_tokens[token_index]->value;
        //TODO: Yarik dodelat' :)
        variable->type = TOKEN_I32;
        add_item(frameStack, variable);
       expect(TOKEN_IDENTIFIER);
       expect(TOKEN_PIPE);
   }
}

//<Optional_else> ::= else <else_body> | eps
void optional_else_rule(){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_ELSE){
        expect(TOKEN_ELSE);
        else_body_rule();
    }
}
//<else_body> ::= <Block> | <Conditionals>
void else_body_rule(){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE){
        printf("else block\n");
        block_rule();
    } else if(all_tokens[token_index]->type == TOKEN_IF){
        conditionals_rule();
    } else {
        printf("else body error\n");
        syntax_error();
    }
}

//<while_statement> ::= while ( Expression ) <Optional_null> <Block>
void while_statement_rule(){
    printf("while\n");
    expect(TOKEN_WHILE);
    expect(TOKEN_LEFT_BRACKET);
    //expression_rule();
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
    while(all_tokens[token_index]->type != TOKEN_RIGHT_BRACE || all_tokens[token_index]->type != TOKEN_PIPE){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
            if(search(frameStack, all_tokens[token_index]->value) == NULL){
                error_exit(ERR_UNDEF_VAR);
            }
        }
        if(all_tokens[token_index]->type == TOKEN_RIGHT_BRACKET){
            Token *token_copy = (Token *)malloc(sizeof(Token));
            if (token_copy == NULL) {
                error_exit(ERR_INTERNAL);
            }
            *token_copy = *all_tokens[token_index];
            expect(TOKEN_RIGHT_BRACKET);
            if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE || all_tokens[token_index]->type == TOKEN_PIPE){
                break;
            }
            add_element(expr, token_copy);
        }
        add_element(expr, all_tokens[token_index]);
        next_token();
    }
    Token *end_token = (Token *)malloc(sizeof(Token));
	if (end_token == NULL) {
        error_exit(ERR_INTERNAL);
    }
	end_token->type = TOKEN_END;
	end_token->value = "$";
	add_element(expr, end_token);
    print_expression(expr);
    parse_expression(expr);
    //expect(TOKEN_RIGHT_BRACKET);
    optional_null_rule();
    block_rule();
}

//<return> ::= return <Expression_opt> ;
void return_statement_rule(){
    expect(TOKEN_RETURN);
    if(all_tokens[token_index]->type == TOKEN_SEMICOLON){
        expect(TOKEN_SEMICOLON);
        return;
    }
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        expect(TOKEN_IDENTIFIER_FUNC);
        function_call_rule();
        return;
    }

    Token *token_copy = (Token *)malloc(sizeof(Token));
        if (token_copy == NULL) {
            error_exit(ERR_INTERNAL);
        }
    *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_UNDEF_VAR);
        }
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
	if (token_copy->type == TOKEN_IDENTIFIER) add_element(expr, token_copy);
    while(all_tokens[token_index]->type != TOKEN_SEMICOLON){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
            if(search(frameStack, all_tokens[token_index]->value) == NULL){
                error_exit(ERR_UNDEF_VAR);
            }
        }
        add_element(expr, all_tokens[token_index]);
        next_token();
    }
	Token *end_token = (Token *)malloc(sizeof(Token));
	if (end_token == NULL) {
        error_exit(ERR_INTERNAL);
    }
	end_token->type = TOKEN_END;
	end_token->value = "$";
	add_element(expr, end_token);
// //TODO free endtoken
    print_expression(expr);
    parse_expression(expr);

    expect(TOKEN_SEMICOLON);
}



int main(){
    //fill all_tokens array with tokens
     Token *current_token = init_token();
    while (current_token->type != TOKEN_EOF) {
        get_token(current_token);
        save_token(current_token);
    }
    token_index = 0;
    //init frame stack :)
    frameStack = init_frameStack();
    add_frame(frameStack);
    while (all_tokens[token_index]->type != TOKEN_EOF) {
        fill_sym_table_fn(frameStack, token_index);
        token_index++;
    }
    token_index = 0;
    
    program_rule();
     if(encoutered_main){
        printf("yep\n");
     } else{
        syntax_error();
     }
    printFrameStack(frameStack);
     //free(current_token->value);
     //free(current_token);
     return 0;
 }