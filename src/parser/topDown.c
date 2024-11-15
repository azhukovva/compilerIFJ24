#include "topDown.h"



// extern const char *tokenName[];
 
//Current token pointer
Token *all_tokens[100000];
FrameStack *frameStack;
int token_index = 0;
bool encoutered_main = false;
bool encountered_return = false;
InstructionList *instructionList;

char *my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);
    if (dup == NULL) {
        return NULL;
    }
    memcpy(dup, s, len);
    return dup;
}

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

TokenType type_compatibility(TokenType variable, TokenType expr_type, bool from_args){
    printf("TYPE COMPAT: %s %s\n", tokenName[variable], tokenName[expr_type]);
    if(expr_type == TOKEN_NULL && variable == TOKEN_EMPTY){
        error_exit(ERR_EXPR_DERIV);
    }
    if(variable != TOKEN_EMPTY){
        if(variable != expr_type){
            if(variable == TOKEN_I32_OPT && (expr_type == TOKEN_I32 || expr_type == TOKEN_NULL)){
                return TOKEN_I32_OPT;
            }
            if(variable == TOKEN_F64_OPT && (expr_type == TOKEN_F64 || expr_type == TOKEN_NULL)){
                return TOKEN_F64_OPT;
            }
            if(variable == TOKEN_U8_OPT && (expr_type == TOKEN_U8 || expr_type == TOKEN_NULL)){
                return TOKEN_U8_OPT;
            } else if(from_args){
                error_exit(ERR_WRONG_PARAM_RET);
            } else{
                error_exit(ERR_EXPR_TYPE);
            }
        }
    } else {
        return expr_type;
    }
    return expr_type;
}

bool is_data_type(TokenType type){
    return type == TOKEN_I32 || type == TOKEN_F64 || type == TOKEN_U8 ||
           type == TOKEN_I32_OPT || type == TOKEN_F64_OPT || type == TOKEN_U8_OPT || type == TOKEN_VOID; 
}

void add_builtin_functions(FrameStack *frameStack) {
    // Define the built-in functions
    const char *builtin_functions[][5] = {
        {"ifj.readstr", "?[]u8", ""},
        {"ifj.readi32", "?i32", ""},
        {"ifj.readf64", "?f64", ""},
        {"ifj.write", "void", "any"},
        {"ifj.i2f", "f64", "i32"},
        {"ifj.f2i", "i32", "f64"},
        {"ifj.length", "i32", "[]u8"},
        {"ifj.concat", "[]u8", "[]u8,[]u8"},
        {"ifj.substring", "?[]u8", "[]u8,i32,i32"},
        {"ifj.strcmp", "i32", "[]u8,[]u8"},
        {"ifj.ord", "i32", "[]u8,i32"},
        {"ifj.chr", "[]u8", "i32"},
        {"ifj.string", "[]u8", "[]u8"}
    };

    for (int i = 0; i < (int)sizeof(builtin_functions) / (int)sizeof(builtin_functions[0]); i++) {
        Node *node = (Node *)malloc(sizeof(Node));
        if (node == NULL) {
            error_exit(ERR_INTERNAL);
        }
        node->id = my_strdup(builtin_functions[i][0]);
        if (node->id == NULL) {
            error_exit(ERR_INTERNAL);
        }
        node->type = get_token_type(builtin_functions[i][1]); 
        node->fn = true;
        node->t_const = false;
        node->params = NULL;
        node->left = NULL;
        node->right = NULL;
        node->height = 1;

        // Parse parameters
        char *params_str = my_strdup(builtin_functions[i][2]);
        if (params_str == NULL) {
            error_exit(ERR_INTERNAL);
        }
        char *param = strtok(params_str, ",");
        Param *last_param = NULL;
        while (param != NULL) {
            Param *new_param = (Param *)malloc(sizeof(Param));
            if (new_param == NULL) {
                error_exit(ERR_INTERNAL);
            }
            new_param->id = NULL; // Built-in functions do not have parameter names
            new_param->type = get_token_type(param); // Implement this function to convert string to TokenType
            new_param->next = NULL;

            if (last_param == NULL) {
                node->params = new_param;
            } else {
                last_param->next = new_param;
            }
            last_param = new_param;

            param = strtok(NULL, ",");
        }
        free(params_str);
        printf("Adding built-in function: %s\n", node->id);
        add_item(frameStack, node);
    }
}

TokenType get_token_type(const char *type_str) {
    if (strcmp(type_str, "i32") == 0) return TOKEN_I32;
    if (strcmp(type_str, "?i32") == 0) return TOKEN_I32_OPT;
    if (strcmp(type_str, "f64") == 0) return TOKEN_F64;
    if (strcmp(type_str, "?f64") == 0) return TOKEN_F64_OPT;
    if (strcmp(type_str, "[]u8") == 0) return TOKEN_U8;
    if (strcmp(type_str, "?[]u8") == 0) return TOKEN_U8_OPT;
    if (strcmp(type_str, "any") == 0) return TOKEN_IDENTIFIER;
    if (strcmp(type_str, "void") == 0) return TOKEN_VOID;
    // Add other types as needed
    return TOKEN_EMPTY;
}

void fill_sym_table_fn(FrameStack *frameStack, int token_index) {
    // Add built-in functions to the frame stack
    static bool builtins_added = false;
    if (!builtins_added) {
        add_builtin_functions(frameStack);
        builtins_added = true;
    }

    // Process user-defined functions
    while (all_tokens[token_index]->type != TOKEN_EOF) {
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
                if (is_data_type(all_tokens[token_index]->type)) {
                    Param *new_param = (Param *)malloc(sizeof(Param));
                    if (new_param == NULL) {
                        error_exit(ERR_INTERNAL);
                    }
                    new_param->id = (char *)malloc(strlen(all_tokens[token_index]->value) + 1);
                    if (new_param->id == NULL) {
                        error_exit(ERR_INTERNAL);
                    }
                    strcpy(new_param->id, all_tokens[token_index-2]->value);
                    new_param->type = all_tokens[token_index]->type;
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
        token_index++;
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
    printf("Checking: type: %s val: %s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value);
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
    Node *fn = search(frameStack, all_tokens[token_index]->value);
    build_instruction(instructionList, "LABEL", all_tokens[token_index]->value, NULL, NULL);
    Param *temp_param = fn->params;
    int i = 0;
    while (temp_param != NULL) {
        i++;
        build_instruction(instructionList, "DEFVAR", _strcat("LF@", temp_param->id), NULL, NULL);
        build_instruction(instructionList, "MOVE", _strcat("LF@", temp_param->id), _strcat("LF@param", itoa(i)), NULL);
        temp_param = temp_param->next;
    }
    if(!strcmp(all_tokens[token_index]->value, "main")){
        main_func_rule(fn);
    } else {
        expect(TOKEN_IDENTIFIER);
        param_list_rule();
        return_type_rule();
        block_rule_fn(fn);
        if(encountered_return == false && fn->type != TOKEN_VOID){
            printf("IN FUNC:   %s\n", fn->id);
            error_exit(ERR_MISS_OVERFL_RETURN);
        }
        encountered_return = false;
    }
}

//<main_func> ::= pub fn main () void <Block>
void main_func_rule(Node *fn){
    expect_id("main");
    expect(TOKEN_LEFT_BRACKET);
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_VOID);
    block_rule_fn(fn);
    if(encountered_return == false && fn->type != TOKEN_VOID){
        error_exit(ERR_MISS_OVERFL_RETURN);
    }
    encoutered_main = true;
    encountered_return = false;
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
    
    Node *return_node = (Node *)malloc(sizeof(Node));
    if (return_node == NULL) {
        error_exit(ERR_INTERNAL);
    }
    return_node->id = "$return$";
    return_node->type = fn->type;
    return_node->used = true;
    return_node->height = 1;
    return_node->params = NULL;
    return_node->left = NULL;
    return_node->right = NULL;
    add_item(frameStack, return_node);

    // Use a temporary pointer to traverse the parameters
    Param *temp_param = fn->params;
    while (temp_param != NULL) {
        add_item_fn(frameStack, temp_param);
        temp_param = temp_param->next;
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
            printf("assignment\n");
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
    variable->type = TOKEN_EMPTY;
    variable->used = false;
    if(all_tokens[token_index]->type == TOKEN_CONST){
        expect(TOKEN_CONST);
        variable->t_const = true;
        variable->value = all_tokens[token_index+2]->value;
    } else if(all_tokens[token_index]->type == TOKEN_VAR){
        expect(TOKEN_VAR);
        variable->t_const = false;
    } else {
        syntax_error();
    }
    variable->fn = false;
    variable->id = all_tokens[token_index]->value;
    build_instruction(instructionList, "DEFVAR", _strcat("LF@", all_tokens[token_index]->value), NULL, NULL);
    expect(TOKEN_IDENTIFIER);
    if(search(frameStack, variable->id) != NULL){
        error_exit(ERR_UNDEF_VAR);
    }
    if(all_tokens[token_index]->type == TOKEN_COLON){
        expect(TOKEN_COLON);
        variable->type = all_tokens[token_index]->type;
        base_type_rule();
    }

    expect(TOKEN_ASSIGN);
    if(all_tokens[token_index]->type == TOKEN_STRING){
        error_exit(ERR_EXPR_DERIV);
    }
        //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        variable->type = type_compatibility(variable->type, search(frameStack, all_tokens[token_index-1]->value)->type, false);
        add_item(frameStack, variable);
        function_call_rule();
        return;
    }
    //expression_rule();
    //дальше пиздец)
    //  Token *token_copy = (Token *)malloc(sizeof(Token));
    //     if (token_copy == NULL) {
    //         error_exit(ERR_INTERNAL);
    //     }
    // *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        variable->type = type_compatibility(variable->type, search(frameStack, all_tokens[token_index-1]->value)->type, false);
        add_item(frameStack, variable);
        function_call_rule();
        return;
    }
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
        
    }
    init_expression(expr);
	if (all_tokens[token_index-1]->type == TOKEN_IDENTIFIER) add_element(expr, all_tokens[token_index-1]);
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

    print_expression(expr);
    TokenType expr_type = parse_expression(expr, frameStack);
    variable->type = type_compatibility(variable->type, expr_type, false);
    printf("Variable ID: %s --- Variable type: %s\n", variable->id,tokenName[variable->type]);
    add_item(frameStack, variable);
    expect(TOKEN_SEMICOLON);

}



//<assigment> ::= id = <Expression> ;
void assigment_rule(){
    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        expect(TOKEN_IDENTIFIER_FUNC);
        function_call_rule();
        return;
    } 
    Node *variable = search(frameStack, all_tokens[token_index]->value);
    if(variable == NULL){
        error_exit(ERR_UNDEF_VAR);
    }

    expect(TOKEN_IDENTIFIER);

    //if the identifier was a user function identifier
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    expect(TOKEN_ASSIGN);
    if(all_tokens[token_index]->type == TOKEN_STRING){
        error_exit(ERR_EXPR_TYPE);
    }
    if(variable->t_const){
        error_exit(ERR_UNDEF_VAR);
    }

    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(variable->type, expr_type, false);
        set_usage(frameStack, variable->id);
        function_call_rule();
        return;
    }
    //expression_rule();
    //дальше пиздец)
    //  Token *token_copy = (Token *)malloc(sizeof(Token));
    //     if (token_copy == NULL) {
    //         error_exit(ERR_INTERNAL);
    //     }
    // *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            if(all_tokens[token_index+1]->type == TOKEN_LEFT_BRACKET){
                
                error_exit(ERR_DEFINE);
            }
            error_exit(ERR_UNDEF_VAR);
        }
        set_usage(frameStack, all_tokens[token_index]->value);
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(variable->type, expr_type, false);
        set_usage(frameStack, variable->id);
        function_call_rule();
        return;
    }
    Expression *expr = (Expression *)malloc(sizeof(Expression));
    if(expr == NULL){
        error_exit(ERR_INTERNAL);
    }
    init_expression(expr);
	if (all_tokens[token_index-1]->type == TOKEN_IDENTIFIER) add_element(expr, all_tokens[token_index-1]);
    while(all_tokens[token_index]->type != TOKEN_SEMICOLON){
        if(all_tokens[token_index]->type == TOKEN_EOF){
            syntax_error();
        }
        if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
            set_usage(frameStack, all_tokens[token_index]->value);
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
    TokenType expr_type = parse_expression(expr, frameStack);
    type_compatibility(variable->type, expr_type, false);
    expect(TOKEN_SEMICOLON);
    set_usage(frameStack, variable->id);
    // free(token_copy);
    //free_expression(expr);
}

//<function_call> ::= ( <Arguments> );
void function_call_rule(){
    printf("function call\n");
    
    Node *signature = NULL;
    token_index--;
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        signature = search(frameStack, all_tokens[token_index]->value);
        if(signature == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
    } else {
        signature = search(frameStack, all_tokens[token_index]->value);
        if(signature == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER);
    }
    expect(TOKEN_LEFT_BRACKET);
    build_instruction(instructionList, "CREATEFRAME", NULL, NULL, NULL);
    printNode(signature);
    Param *temp_param = signature->params;
    int i = 0;
    while (temp_param != NULL) {
        i++;
        build_instruction(instructionList, "DEFVAR", _strcat("TF@param", itoa(i)), NULL, NULL);
        temp_param = temp_param->next;
    }
    parse_args(signature, token_index);
    arguments_rule();
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
    printf("function call end\n");
    build_instruction(instructionList, "DEFVAR", "TF@return_value", NULL, NULL);
    build_instruction(instructionList, "PUSHFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "CALL", signature->id, NULL, NULL);
    skip_comments();
}

bool is_lit(TokenType type){
    return type == TOKEN_INT || type == TOKEN_FLOAT || type == TOKEN_STRING || type == TOKEN_NULL;
}
TokenType convert_from_lit(TokenType type){
    switch(type){
        case TOKEN_INT:
            return TOKEN_I32;
        case TOKEN_FLOAT:
            return TOKEN_F64;
        case TOKEN_STRING:
            return TOKEN_U8;
        default:
            return TOKEN_EMPTY;
    }
}
bool is_optional_type(TokenType type){
    printf("TYPE: %s\n", tokenName[type]);
    return type == TOKEN_I32_OPT || type == TOKEN_F64_OPT || type == TOKEN_U8_OPT;
}

void check_param_lists(Param *param, Param *args){
    while (param != NULL || args != NULL) {
        if (param == NULL || args == NULL) {
            printf("lol\n");
            error_exit(ERR_WRONG_PARAM_RET); // One list is longer than the other
        }
        if(param->type == TOKEN_IDENTIFIER){
            param = param->next;
            args = args->next;
            continue;
        }
        if( is_optional_type(param->type) && args->type == TOKEN_NULL){
            param = param->next;
            args = args->next;
            continue;
        }
        if (!type_compatibility(param->type, args->type, true)) {
            printf("lol\n");
            error_exit(ERR_WRONG_PARAM_RET); // Types do not match
        }
        param = param->next;
        args = args->next;
    }
}

void print_param_list(const char *label, Param *head) {
    printf("%s: ", label);
    Param *current = head;
    while (current != NULL) {
        printf("%d ", current->type);
        current = current->next;
    }
    printf("\n");
}
char *what_lit(TokenType type){
    switch(type){
        case TOKEN_INT:
            return "int@";
        case TOKEN_FLOAT:
            return "float@";
        case TOKEN_STRING:
            return "string@";
        default:
            return "";
    }
}

void parse_args(Node *signature, int index){
    Param *param = signature->params;
    Param *args = NULL;
    int i = 0;
    while(all_tokens[index]->type != TOKEN_RIGHT_BRACKET){
        i++;
        printf("ya ebal\n");
        if(all_tokens[index]->type == TOKEN_EOF){
            printf("ya ebal\n");
            syntax_error();
        }
        if(all_tokens[index]->type == TOKEN_IDENTIFIER){
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), _strcat("LF@", all_tokens[index]->value), NULL);
            Node *to_check = search(frameStack, all_tokens[index]->value);
            if(to_check == NULL){
                error_exit(ERR_UNDEF_VAR);
            }
            Param *new_param = (Param *)malloc(sizeof(Param));
            if(new_param == NULL){
                error_exit(ERR_INTERNAL);
            }
            new_param->type = to_check->type;
            new_param->id = to_check->id;
            new_param->next = NULL;
            if(args == NULL){
                args = new_param;
            } else {
                Param *tmp = args;
                while(tmp->next != NULL){
                    tmp = tmp->next;
                }
                tmp->next = new_param;
            }
            set_usage(frameStack, all_tokens[index]->value);
        }  
        if(is_lit(all_tokens[index]->type)){
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), _strcat(what_lit(all_tokens[token_index]->type), all_tokens[token_index]->value), NULL);
            Param *new_param = (Param *)malloc(sizeof(Param));
            if(new_param == NULL){
                error_exit(ERR_INTERNAL);
            }
            if(all_tokens[index]->type == TOKEN_NULL){
                new_param->type = TOKEN_NULL;
            } else {
                new_param->type = convert_from_lit(all_tokens[index]->type);
            }
            new_param->id = all_tokens[index]->value;
            new_param->next = NULL;
            if(args == NULL){
                args = new_param;
            } else {
                Param *tmp = args;
                while(tmp->next != NULL){
                    tmp = tmp->next;
                }
                tmp->next = new_param;
            }
        }
        index++;
    }
    print_param_list("Signature", signature->params);
    print_param_list("Arguments", args);
    check_param_lists(param, args);
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
    case TOKEN_NULL:
        expect(TOKEN_NULL);
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
            set_usage(frameStack, all_tokens[token_index]->value);
        }
        if(all_tokens[token_index]->type == TOKEN_RIGHT_BRACKET){
            // Token *token_copy = (Token *)malloc(sizeof(Token));
            // if (token_copy == NULL) {
            //     error_exit(ERR_INTERNAL);
            // }
            // *token_copy = *all_tokens[token_index];
            expect(TOKEN_RIGHT_BRACKET);
            if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE || all_tokens[token_index]->type == TOKEN_PIPE){
                break;
            }
            add_element(expr, all_tokens[token_index-1]);
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
    TokenType id_w_null = parse_expression(expr, frameStack);
// //TODO free endtoken
    //expect(TOKEN_RIGHT_BRACKET);
    optional_null_rule(id_w_null);
    block_rule();
    optional_else_rule();
}
TokenType convert_from_opt(TokenType type){
    switch(type){
        case TOKEN_I32_OPT:
            return TOKEN_I32;
        case TOKEN_F64_OPT:
            return TOKEN_F64;
        case TOKEN_U8_OPT:
            return TOKEN_U8;
        default:
            return type;
    }
}

//<Optional_null> ::= |id| | eps
void optional_null_rule(TokenType id_w_null){
    skip_comments();
   if(all_tokens[token_index]->type == TOKEN_PIPE){
       expect(TOKEN_PIPE);
       Node *variable = (Node *)malloc(sizeof(Node));
        variable->height = 1;
        variable->t_const = false;
        variable->fn = false;
        variable->used = true;
        variable->id = all_tokens[token_index]->value;
        variable->type = convert_from_opt(id_w_null);
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
            set_usage(frameStack, all_tokens[token_index]->value);
        }
        if(all_tokens[token_index]->type == TOKEN_RIGHT_BRACKET){
            // Token *token_copy = (Token *)malloc(sizeof(Token));
            // if (token_copy == NULL) {
            //     error_exit(ERR_INTERNAL);
            // }
            // *token_copy = *all_tokens[token_index];
            expect(TOKEN_RIGHT_BRACKET);
            if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE || all_tokens[token_index]->type == TOKEN_PIPE){
                break;
            }
            add_element(expr, all_tokens[token_index-1]);
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
    TokenType id_w_null = parse_expression(expr, frameStack);
    //expect(TOKEN_RIGHT_BRACKET);
    optional_null_rule(id_w_null);
    block_rule();
}

//<return> ::= return <Expression_opt> ;
void return_statement_rule(){
    expect(TOKEN_RETURN); 
    encountered_return = true;
    Node *return_node = search(frameStack, "$return$");
    if(all_tokens[token_index]->type == TOKEN_SEMICOLON){
        expect(TOKEN_SEMICOLON);
        build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
        return;
    }
    if(return_node->type == TOKEN_VOID){
        error_exit(ERR_MISS_OVERFL_RETURN);
    }
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        TokenType ret_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(return_node->type, ret_type,false);
        function_call_rule();
        build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
        return;
    }

    Token *token_copy = (Token *)malloc(sizeof(Token));
        if (token_copy == NULL) {
            error_exit(ERR_INTERNAL);
        }
    *token_copy = *all_tokens[token_index];
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_DEFINE);
        }
        set_usage(frameStack, all_tokens[token_index]->value);
        expect(TOKEN_IDENTIFIER);
    }
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET){
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(return_node->type, expr_type,false);
        function_call_rule();
        build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
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
                error_exit(ERR_DEFINE);
            }
            set_usage(frameStack, all_tokens[token_index]->value);
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
    TokenType expr_type = parse_expression(expr, frameStack);
    type_compatibility(return_node->type, expr_type,false);

    expect(TOKEN_SEMICOLON);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
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
    fill_sym_table_fn(frameStack, token_index);
        
    token_index = 0;
    
    instructionList = init_instruction_list();
    build_instruction(instructionList, "JUMP", "main", NULL, NULL);
    program_rule();
     if(encoutered_main){
        printf("yep\n");
     } else{
        syntax_error();
     }
    printFrameStack(frameStack);
     //free(current_token->value);
     //free(current_token);
    print_instruction_list(instructionList);
    return 0;
 }