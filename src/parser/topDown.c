#include "topDown.h"



// extern const char *tokenName[];
//extern int intResCounter;
//Current token pointer
Token *all_tokens[100000];
FrameStack *frameStack;
int token_index = 0;
bool encoutered_main = false;
bool encountered_return = false;
bool encountered_strcmp = false;
bool encountered_substring = false;
InstructionList *instructionList;
int else_case_cnt = 0;
int end_case_cnt = 0;
int while_case_cnt = 0;
int while_end_case_cnt = 0;

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
    ////printf("TYPE COMPAT: %s %s\n", tokenName[variable], tokenName[expr_type]);
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
        //printf("Adding built-in function: %s\n", node->id);
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
            //printf("ID: %s\n", node->id);

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
char* build_builtin_lit_arg(TokenType param_type, char *param_id, bool from_main){
    //printf("param_typeeeee: %s :: %s\n", tokenName[param_type], param_id);
    switch(param_type){
        case TOKEN_INT:
            return _strcat("int@", param_id);
        case TOKEN_FLOAT:
        {
            float tmp = atof(param_id);
            char tmp_str[100];
            sprintf(tmp_str, "%a", tmp);
            return _strcat("float@", tmp_str);
        }
        case TOKEN_STRING:
            return escape_sequence(param_id);
        case TOKEN_IDENTIFIER:
            return _strcat(what_frame(from_main), param_id);
        default:
            return "";
    }
}

char* what_frame(bool from_main){
    if(from_main){
        return "GF@";
    } else {
        return "LF@";
    }
}

void build_builtin(char *id, int curr_token, char *var_id, bool from_main){
    Param *params = NULL;
    while(all_tokens[curr_token]->type != TOKEN_RIGHT_BRACKET){
      if(all_tokens[curr_token]->type == TOKEN_IDENTIFIER || is_lit(all_tokens[curr_token]->type)){
          Param *new_param = (Param *)malloc(sizeof(Param));
        if (new_param == NULL) {
            error_exit(ERR_INTERNAL);
        }
        new_param->id = (char *)malloc(strlen(all_tokens[curr_token]->value) + 1);
        if (new_param->id == NULL) {
            error_exit(ERR_INTERNAL);
        }
        strcpy(new_param->id, all_tokens[curr_token]->value);
        new_param->type = all_tokens[curr_token]->type;
        new_param->next = NULL;
        if(params == NULL){
            params = new_param;
        } else {
            Param *temp = params;
            while(temp->next != NULL){
                temp = temp->next;
            }
            temp->next = new_param;
        }
      }
      curr_token += 1;
    }
    //printf("var_id :: %s\n", id);
    //printf("mama sdohla\n");
    //TokenType var_type = search(frameStack, var_id)->type;
    ////printf("ya ebanat: %s\n", tokenName[params->type]);
    if(strcmp(id, "ifj.i2f") == 0){
        build_instruction(instructionList, "INT2FLOAT", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id, from_main), NULL);
    } else if(strcmp(id, "ifj.f2i") == 0){
        build_instruction(instructionList, "FLOAT2INT", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id, from_main), NULL);
    } else if(strcmp(id, "ifj.concat") == 0){
        build_instruction(instructionList, "CONCAT", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id, from_main),build_builtin_lit_arg(params->next->type, params->next->id, from_main));
    } else if(strcmp(id, "ifj.readi32") == 0){
        build_instruction(instructionList, "READ", _strcat(what_frame(from_main), var_id), "int", NULL);
    } else if(strcmp(id, "ifj.readf64") == 0){
        build_instruction(instructionList, "READ", _strcat(what_frame(from_main), var_id), "float", NULL);
    } else if(strcmp(id, "ifj.readstr") == 0){
        build_instruction(instructionList, "READ", _strcat(what_frame(from_main), var_id), "string", NULL);
    } else if(strcmp(id, "ifj.write") == 0){
        build_instruction(instructionList, "WRITE", build_builtin_lit_arg(params->type, params->id, from_main), NULL, NULL);
    } else if(strcmp(id, "ifj.length") == 0){
        build_instruction(instructionList, "STRLEN", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id,from_main), NULL);
    } else if(strcmp(id, "ifj.chr") == 0){
        build_instruction(instructionList, "INT2CHAR", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id,from_main), NULL);
    } else if(strcmp(id, "ifj.ord") == 0){
        build_instruction(instructionList, "STRI2INT", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id, from_main), build_builtin_lit_arg(params->next->type, params->next->id, from_main));
    } else if(strcmp(id, "ifj.string") == 0){
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), var_id), build_builtin_lit_arg(params->type, params->id, from_main), NULL);
    } else if(strcmp(id, "ifj.strcmp") == 0){
        build_instruction(instructionList, "CREATEFRAME", NULL, NULL, NULL);
        build_instruction(instructionList, "DEFVAR", "TF@param1", NULL, NULL);
        build_instruction(instructionList, "MOVE", "TF@param1", build_builtin_lit_arg(params->type, params->id, from_main), NULL);
        build_instruction(instructionList, "DEFVAR", "TF@param2", NULL, NULL);
        build_instruction(instructionList, "MOVE", "TF@param2", build_builtin_lit_arg(params->next->type, params->next->id, from_main), NULL);
        build_instruction(instructionList, "DEFVAR", "TF@return_value", NULL, NULL);
        build_instruction(instructionList, "PUSHFRAME", NULL, NULL, NULL);
        build_instruction(instructionList, "CALL", "ifj_strcmp", NULL, NULL);
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), var_id), "TF@return_value", NULL);
        encountered_strcmp = true;
    } else if(strcmp(id, "ifj.substring") == 0){
        build_instruction(instructionList, "CREATEFRAME", NULL, NULL, NULL);
        build_instruction(instructionList, "DEFVAR", "TF@param1", NULL, NULL);
        build_instruction(instructionList, "MOVE", "TF@param1", build_builtin_lit_arg(params->type, params->id, from_main), NULL);
        build_instruction(instructionList, "DEFVAR", "TF@param2", NULL, NULL);
        build_instruction(instructionList, "MOVE", "TF@param2", build_builtin_lit_arg(params->next->type, params->next->id, from_main), NULL);
        build_instruction(instructionList, "DEFVAR", "TF@param3", NULL, NULL);
        build_instruction(instructionList, "MOVE", "TF@param3", build_builtin_lit_arg(params->next->next->type, params->next->next->id, from_main), NULL);
        build_instruction(instructionList, "DEFVAR", "TF@return_value", NULL, NULL);
        build_instruction(instructionList, "PUSHFRAME", NULL, NULL, NULL);
        build_instruction(instructionList, "CALL", "ifj_substring", NULL, NULL);
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), var_id), "TF@return_value", NULL);
        encountered_substring = true;
    }
    
   //print_param_list( "queres ?",params);
}

void parse_while( bool from_main, int index, bool in_while){
    int nested = 0;
    if(in_while) return;
    while(all_tokens[index]->type != TOKEN_LEFT_BRACE){
        if(all_tokens[index]->type == TOKEN_EOF){
            syntax_error();
        }
        index++;
    }
    index++;
    while(all_tokens[index]->type != TOKEN_RIGHT_BRACE || nested != 0){
        if(all_tokens[index]->type == TOKEN_EOF){
            syntax_error();
        }
        //printf("AT: %s\n", all_tokens[index]->value);
        if((all_tokens[index]->type == TOKEN_VAR ||all_tokens[index]->type == TOKEN_CONST) && all_tokens[index+1]->type == TOKEN_IDENTIFIER ){
            build_instruction(instructionList, "DEFVAR", _strcat(what_frame(from_main), all_tokens[index+1]->value), NULL, NULL);
        }
        if(all_tokens[index]->type == TOKEN_LEFT_BRACE){
            nested = nested + 1;
        }
        if(all_tokens[index]->type == TOKEN_RIGHT_BRACE){
            nested = nested - 1;
        }
        index++;
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
        //printf("Skipping comment\n");
        next_token();
    }
}
void expect(TokenType type){
    ////printf("Checking: type: %s val: %s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value);
    //if we get a comment we skip it and get the next token
    skip_comments();
    //printf("Checking: type: %s val: %s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value);
    if(all_tokens[token_index]->type == type){
        next_token();
    } else {
        //printf("Expected: %s\n", tokenName[type]);
        syntax_error();
    }
}

//Check if the id of token value matches expected one
void expect_id(char *id_to_check){
    ////printf("type: %s val: %s id_to_check:%s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value, id_to_check);
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER && !strcmp(all_tokens[token_index]->value, id_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the string in token matches expected one
void expect_string(char *string_to_check){
    ////printf("type: %s val: %s id_to_check:%s\n", tokenName[all_tokens[token_index]->type], all_tokens[token_index]->value, string_to_check);
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
    if(strcmp(fn->id, "main") == 0){
        build_instruction(instructionList, "DEFVAR", "GF@PAMAGITI", NULL, NULL);
        build_instruction(instructionList, "DEFVAR", "GF@supa_giga_expr_res", NULL, NULL);
    } else {
        build_instruction(instructionList, "DEFVAR", "LF@PAMAGITI", NULL, NULL);
        build_instruction(instructionList, "DEFVAR", "LF@supa_giga_expr_res", NULL, NULL);
    }
    
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
        block_rule_fn(fn, false);
        if(encountered_return == false && fn->type != TOKEN_VOID){
            //printf("IN FUNC:   %s\n", fn->id);
            error_exit(ERR_MISS_OVERFL_RETURN);
        }
        if(encountered_return == false && fn->type == TOKEN_VOID){
            build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
            build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
        }
        encountered_return = false;
    }
}

//<main_func> ::= pub fn main () void <Block>
void main_func_rule(Node *fn){
    bool from_main = true;
    expect_id("main");
    expect(TOKEN_LEFT_BRACKET);
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_VOID);
    block_rule_fn(fn, from_main);
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
void block_rule_fn(Node *fn, bool from_main){
    expect(TOKEN_LEFT_BRACE);
    add_frame(frameStack);
    skip_comments();
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

    //printf("entering statements\n");
    statements_rule(from_main, false);
    expect(TOKEN_RIGHT_BRACE);
    //printFrameStack(frameStack);
    removeFrame(frameStack);

}

void block_rule(){
    expect(TOKEN_LEFT_BRACE);
    skip_comments();
    add_frame(frameStack);
    //printf("entering statements\n");
    statements_rule(false, false);
    expect(TOKEN_RIGHT_BRACE);
    //printFrameStack(frameStack);
    removeFrame(frameStack);
}

void if_block(bool from_main, int what_else_case){
    expect(TOKEN_LEFT_BRACE);
    skip_comments();
    add_frame(frameStack);
    //printf("entering statements\n");
    statements_rule(from_main, false);
    expect(TOKEN_RIGHT_BRACE);
    //printFrameStack(frameStack);
    removeFrame(frameStack);
    if(all_tokens[token_index]->type == TOKEN_ELSE){
        build_instruction(instructionList, "JUMP", _strcat("end_case", itoa(end_case_cnt)), NULL, NULL);
    }
    build_instruction(instructionList, "LABEL", _strcat("else_case", itoa(what_else_case)), NULL, NULL);
}

void else_block(bool from_main){
    expect(TOKEN_LEFT_BRACE);
    skip_comments();
    add_frame(frameStack);
    //printf("entering statements\n");
    statements_rule(from_main, false);
    expect(TOKEN_RIGHT_BRACE);
    //printFrameStack(frameStack);
    removeFrame(frameStack);
    build_instruction(instructionList, "LABEL", _strcat("end_case", itoa(end_case_cnt)), NULL, NULL);
    end_case_cnt++;
}
void while_block(bool from_main, int what_while_case, int what_while_end_case, Expression *expr, bool in_while, int pipe_index){
    expect(TOKEN_LEFT_BRACE);
    skip_comments();
    add_frame(frameStack);
    //printf("entering statements\n");
    statements_rule(from_main, in_while);
    expect(TOKEN_RIGHT_BRACE);
    //printFrameStack(frameStack);
    removeFrame(frameStack);
    parse_expression(expr, frameStack, from_main);
    if(all_tokens[pipe_index]->type == TOKEN_PIPE){
        build_instruction(instructionList, "TYPE", _strcat(what_frame(from_main), "supa_giga_expr_res"), _strcat(what_frame(from_main), "supa_giga_expr_res"), NULL);
        build_instruction(instructionList, "JUMPIFNEQ", _strcat("while_end_case_", itoa(what_while_end_case)), _strcat(what_frame(from_main), "supa_giga_expr_res"), "string@nil");
    } else {
        build_instruction(instructionList, "JUMPIFEQ", _strcat("while_case_", itoa(what_while_case)), _strcat(what_frame(from_main), "supa_giga_expr_res"),"bool@true");
    }
    build_instruction(instructionList, "LABEL", _strcat("while_end_case_", itoa(what_while_end_case)), NULL, NULL);
    
}

// <Statements> ::= <Statement> <Statements> | eps
void statements_rule(bool from_main, bool in_while){
    skip_comments();
    while(is_statement_start(all_tokens[token_index]->type)){
        statement_rule(from_main, in_while);
    }

    //printf("exiting statements\n");
   
}

// Function that defines whether or not we have any statements to parse
bool is_statement_start(TokenType type){
    skip_comments();
    return type == TOKEN_CONST || type == TOKEN_VAR || type == TOKEN_IDENTIFIER ||
           type == TOKEN_IF || type == TOKEN_WHILE || type == TOKEN_RETURN || type == TOKEN_IDENTIFIER_FUNC || type == TOKEN_UNDERLINE;
}

void statement_rule(bool from_main, bool in_while){
        skip_comments();
        switch (all_tokens[token_index]->type){
        case TOKEN_CONST:
        case TOKEN_VAR:
            //printf("var\n");
            var_rule(from_main, in_while);
            break;
        // assigment or function call
        case TOKEN_IDENTIFIER_FUNC:
        case TOKEN_IDENTIFIER:
        case TOKEN_UNDERLINE:
           // printf("assignment\n");
            assigment_rule(from_main);
            break;
        case TOKEN_IF:
            conditionals_rule(from_main);
            break;
        case TOKEN_WHILE:
            while_statement_rule(from_main, in_while);
            break;
        case TOKEN_RETURN:
            return_statement_rule(from_main);
            break; 
        default:
            //printf("XD?\n");
            syntax_error();
            break;
        
    }
}

//<var_def> ::= <Var_mode> id <Var_type> = <Expression> ;
void var_rule(bool from_main, bool in_while){
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
    if(!in_while){
        if(from_main){
            build_instruction(instructionList, "DEFVAR", _strcat("GF@", all_tokens[token_index]->value), NULL, NULL);
        } else {
            build_instruction(instructionList, "DEFVAR", _strcat("LF@", all_tokens[token_index]->value), NULL, NULL);
        }
    }

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
        //printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        variable->type = type_compatibility(variable->type, search(frameStack, all_tokens[token_index-1]->value)->type, false);
        add_item(frameStack, variable);
        function_call_rule(variable->id, from_main);
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
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET && all_tokens[token_index-1]->type == TOKEN_IDENTIFIER){
        variable->type = type_compatibility(variable->type, search(frameStack, all_tokens[token_index-1]->value)->type, false);
        add_item(frameStack, variable);
        function_call_rule(variable->id, from_main);
        if(from_main){
            build_instruction(instructionList, "MOVE", _strcat("GF@", variable->id), "TF@return_value", NULL);
        } else {
            build_instruction(instructionList, "MOVE", _strcat("LF@", variable->id), "TF@return_value", NULL);
        }
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

    //print_expression(expr);

    TokenType expr_type = parse_expression(expr, frameStack, from_main);
    //printf("here\n");
    variable->type = type_compatibility(variable->type, expr_type, false);
    if(from_main){
        build_instruction(instructionList, "MOVE", _strcat("GF@", variable->id), "GF@supa_giga_expr_res", NULL);
    } else {
        build_instruction(instructionList, "MOVE", _strcat("LF@", variable->id), "LF@supa_giga_expr_res", NULL);
    }
    //printf("Variable ID: %s --- Variable type: %s\n", variable->id,tokenName[variable->type]);
    add_item(frameStack, variable);
    expect(TOKEN_SEMICOLON);
    skip_comments();

}



//<assigment> ::= id = <Expression> ;
void assigment_rule(bool from_main){
    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        if(search(frameStack, all_tokens[token_index]->value) != NULL){
            if(search(frameStack, all_tokens[token_index]->value)->type != TOKEN_VOID){
                error_exit(ERR_WRONG_PARAM_RET);
            }
            expect(TOKEN_IDENTIFIER_FUNC);
            function_call_rule(NULL, from_main);
            return;
        } else {
            error_exit(ERR_DEFINE);
        }
    } 
    Node *variable = NULL;

    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
       // printf("here\n");
        variable = search(frameStack, all_tokens[token_index-1]->value);
        //printf("there\n");
        if(variable == NULL){
            error_exit(ERR_DEFINE);
        }
    } else if (all_tokens[token_index]->type == TOKEN_UNDERLINE) {
        expect(TOKEN_UNDERLINE);
    } else {
        syntax_error();
    }
    
    

    //if the identifier was a user function identifier
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET && all_tokens[token_index-1]->type == TOKEN_IDENTIFIER){
        //printf("here\n");
        if(search(frameStack, all_tokens[token_index-1]->value) != NULL){
            if(variable->type != TOKEN_VOID){
                error_exit(ERR_WRONG_PARAM_RET);
            }
        } else{
            error_exit(ERR_DEFINE);
        }
        //printf("hereeee\n");
        function_call_rule(NULL, from_main);
        return;
    }
    expect(TOKEN_ASSIGN);
    if(all_tokens[token_index]->type == TOKEN_STRING){
        error_exit(ERR_EXPR_TYPE);
    }
    if(variable != NULL){
        if(variable->t_const){
            error_exit(ERR_UNDEF_VAR);
        }
    }

    //inbuilt function call
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        //printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        if(variable != NULL) {
            type_compatibility(variable->type, expr_type, false);
            set_usage(frameStack, variable->id);
            //printf("mama sdohla232\n");
            function_call_rule(variable->id, from_main);
            return;
        }
        //printf("mama sdohla23\n");
        function_call_rule(NULL, from_main);
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
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET && all_tokens[token_index-1]->type == TOKEN_IDENTIFIER){
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(variable->type, expr_type, false);
        set_usage(frameStack, variable->id);
        function_call_rule(variable->id, from_main);
        if(from_main){
            build_instruction(instructionList, "MOVE", _strcat("GF@", variable->id), "TF@return_value", NULL);
        } else {
            build_instruction(instructionList, "MOVE", _strcat("LF@", variable->id), "TF@return_value", NULL);
        }
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
    //print_expression(expr);
    TokenType expr_type = parse_expression(expr, frameStack, from_main);
    if(from_main){
        build_instruction(instructionList, "MOVE", _strcat("GF@", variable->id), "GF@supa_giga_expr_res", NULL);
    } else {
        build_instruction(instructionList, "MOVE", _strcat("LF@", variable->id), "LF@supa_giga_expr_res", NULL);
    }
    type_compatibility(variable->type, expr_type, false);
    expect(TOKEN_SEMICOLON);
    skip_comments();
    set_usage(frameStack, variable->id);
    // free(token_copy);
    //free_expression(expr);
}

//<function_call> ::= ( <Arguments> );
void function_call_rule(char *id, bool from_main){
    //printf("function call\n");
    bool builtin = false;
    Node *signature = NULL;
    token_index--;
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        signature = search(frameStack, all_tokens[token_index]->value);
        if(signature == NULL){
            
            error_exit(ERR_DEFINE);
        }
        
        builtin = true;
        expect(TOKEN_IDENTIFIER_FUNC);
    } else {
        signature = search(frameStack, all_tokens[token_index]->value);
        if(signature == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER);
    }
    expect(TOKEN_LEFT_BRACKET);
    //printNode(signature);
    if(!builtin){
        build_instruction(instructionList, "CREATEFRAME", NULL, NULL, NULL);
        Param *temp_param = signature->params;
        int i = 0;
        while (temp_param != NULL) {
            i++;
            build_instruction(instructionList, "DEFVAR", _strcat("TF@param", itoa(i)), NULL, NULL);
            temp_param = temp_param->next;
        }
    }
    int curr_token = token_index;
    parse_args(signature, token_index, builtin, from_main);
    arguments_rule();    
    if(builtin) build_builtin(signature->id, curr_token, id, from_main);
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
    //printf("function call end\n");
    if(!builtin){
        build_instruction(instructionList, "DEFVAR", "TF@return_value", NULL, NULL);
        build_instruction(instructionList, "PUSHFRAME", NULL, NULL, NULL);
        build_instruction(instructionList, "CALL", signature->id, NULL, NULL);
    }
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
    //printf("TYPE: %s\n", tokenName[type]);
    return type == TOKEN_I32_OPT || type == TOKEN_F64_OPT || type == TOKEN_U8_OPT;
}

void check_param_lists(Param *param, Param *args){
    while (param != NULL || args != NULL) {
        if (param == NULL || args == NULL) {
            //printf("lol\n");
            error_exit(ERR_WRONG_PARAM_RET); // One list is longer than the other
        }
        //printf("mama ya soshel s uma param: %s args: %s\n", tokenName[param->type], tokenName[args->type]);
        if(param->type == TOKEN_IDENTIFIER){
            // if(args->type == TOKEN_I32 || args->type == TOKEN_F64 || args->type == TOKEN_FLOAT_EXP){
            //     error_exit(ERR_WRONG_PARAM_RET);
            // }
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
            //printf("lol\n");
            error_exit(ERR_WRONG_PARAM_RET); // Types do not match
        }
        param = param->next;
        args = args->next;
    }
}

// void print_param_list(const char *label, Param *head) {
//     //printf("%s: ", label);
//     Param *current = head;
//     while (current != NULL) {
//         //printf("%s ", current->id);
//         //printf("%s ", tokenName[current->type]);
//         current = current->next;
//     }
//     //printf("\n");
// }
void build_arg_lit(int i){
    switch(all_tokens[token_index]->type){
        case TOKEN_INT:
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), _strcat("int@", all_tokens[token_index]->value), NULL);
            return;
        case TOKEN_FLOAT:
        {
            float tmp = atof(all_tokens[token_index]->value);
            char tmp_str[100];
            sprintf(tmp_str, "%a", tmp);
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), _strcat("float@", tmp_str), NULL);
            return;
        }
        case TOKEN_STRING:
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), escape_sequence(all_tokens[token_index]->value), NULL);
            return;
        case TOKEN_NULL:
            build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), "nil@nil", NULL);;
            return;
        default:
            return;
    }
}

void parse_args(Node *signature, int index, bool builtin, bool from_main){
    Param *param = signature->params;
    Param *args = NULL;
    int i = 0;
    while(all_tokens[index]->type != TOKEN_RIGHT_BRACKET){
        i++;
        //printf("ya ebal\n");
        if(all_tokens[index]->type == TOKEN_EOF){
            //printf("ya ebal\n");
            syntax_error();
        }
        if(all_tokens[index]->type == TOKEN_IDENTIFIER){
            if(!builtin) build_instruction(instructionList, "MOVE", _strcat("TF@param", itoa(i)), _strcat(what_frame(from_main), all_tokens[index]->value), NULL);
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
            if(!builtin) build_arg_lit(i);
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
    // print_param_list("Signature", signature->params);
    // print_param_list("Arguments", args);
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
//TODO: add from_main flag as parameter
void conditionals_rule(bool from_main){
    skip_comments();
    expect(TOKEN_IF);
    expect(TOKEN_LEFT_BRACKET);
    //заглушка
    //printf("conditionals\n");
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
    //print_expression(expr);
    TokenType id_w_null = parse_expression(expr, frameStack, from_main);
    else_case_cnt++;
    if(all_tokens[token_index]->type == TOKEN_PIPE){
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), "PAMAGITI"), _strcat(what_frame(from_main), "supa_giga_expr_res"), NULL);
        build_instruction(instructionList, "TYPE", _strcat(what_frame(from_main), "supa_giga_expr_res"), _strcat(what_frame(from_main), "supa_giga_expr_res"), NULL);
        build_instruction(instructionList, "JUMPIFEQ", _strcat("else_case", itoa(else_case_cnt)), _strcat(what_frame(from_main), "supa_giga_expr_res"), "string@nil");
    } else{
        build_instruction(instructionList, "JUMPIFEQ", _strcat("else_case", itoa(else_case_cnt)), _strcat(what_frame(from_main), "supa_giga_expr_res"), "bool@false");
    }
    optional_null_rule(id_w_null, from_main, false);
    if_block(from_main, else_case_cnt);
    optional_else_rule(from_main);
    skip_comments();
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
void optional_null_rule(TokenType id_w_null, bool from_main, bool in_while){
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
        if(!in_while){ 
            build_instruction(instructionList, "DEFVAR", _strcat(what_frame(from_main), variable->id), NULL, NULL);
            build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), variable->id), _strcat(what_frame(from_main), "PAMAGITI"), NULL);
        }
   }
}

//<Optional_else> ::= else <else_body> | eps
void optional_else_rule(bool from_main){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_ELSE){
        expect(TOKEN_ELSE);
        else_body_rule(from_main);
    }
}
//<else_body> ::= <Block> | <Conditionals>
void else_body_rule(bool from_main){
    skip_comments();
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACE){
        //printf("else block\n");
        skip_comments();
        else_block(from_main);
        skip_comments();
    } else if(all_tokens[token_index]->type == TOKEN_IF){
        skip_comments();
        conditionals_rule(from_main);
        skip_comments();
    } else {
        //printf("else body error\n");
        syntax_error();
    }
}

//<while_statement> ::= while ( Expression ) <Optional_null> <Block>
// flag in_while to block DEFVARS in var_rule
void while_statement_rule(bool from_main, bool in_while){
    //printf("while\n");
    skip_comments();
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
    //print_expression(expr);
    TokenType id_w_null = parse_expression(expr, frameStack, from_main);
    //expect(TOKEN_RIGHT_BRACKET);
    parse_while(from_main, token_index, in_while);
    in_while = true;
    while_end_case_cnt++;
    int pipe_index = token_index;
    if(all_tokens[pipe_index]->type == TOKEN_PIPE){
        build_instruction(instructionList, "DEFVAR", _strcat(what_frame(from_main), all_tokens[pipe_index+1]->value), NULL, NULL);
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), "PAMAGITI"), _strcat(what_frame(from_main), "supa_giga_expr_res"), NULL);
        build_instruction(instructionList, "TYPE", _strcat(what_frame(from_main), "supa_giga_expr_res"), _strcat(what_frame(from_main), "supa_giga_expr_res"), NULL);
        build_instruction(instructionList, "JUMPIFNEQ", _strcat("while_end_case_", itoa(while_end_case_cnt)), _strcat(what_frame(from_main), "supa_giga_expr_res"), "string@nil");
        while_case_cnt++;
        build_instruction(instructionList, "LABEL", _strcat("while_case_", itoa(while_case_cnt)), NULL, NULL);
        build_instruction(instructionList, "MOVE", _strcat(what_frame(from_main), all_tokens[pipe_index+1]->value), _strcat(what_frame(from_main), "PAMAGITI"), NULL);
    }
    else{
        while_case_cnt++;
        build_instruction(instructionList, "JUMPIFEQ", _strcat("while_end_case_", itoa(while_end_case_cnt)), _strcat(what_frame(from_main), "supa_giga_expr_res"), "bool@false");
        build_instruction(instructionList, "LABEL", _strcat("while_case_", itoa(while_case_cnt)), NULL, NULL);
    }
    optional_null_rule(id_w_null, from_main, true);
    while_block(from_main, while_case_cnt, while_end_case_cnt, expr, in_while, pipe_index);
    in_while = false;
    skip_comments();
}

//<return> ::= return <Expression_opt> ;
void return_statement_rule(bool from_main){
    skip_comments();
    expect(TOKEN_RETURN); 
    encountered_return = true;
    Node *return_node = search(frameStack, "$return$");
    if(all_tokens[token_index]->type == TOKEN_SEMICOLON){
        expect(TOKEN_SEMICOLON);
        if(!from_main){
            build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
            build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
            return;
        }
        build_instruction(instructionList, "EXIT", "int@0", NULL, NULL);
        return;
    }
    if(return_node->type == TOKEN_VOID){
        error_exit(ERR_MISS_OVERFL_RETURN);
    }
    if(all_tokens[token_index]->type == TOKEN_IDENTIFIER_FUNC){
        //printf("here\n");
        if(search(frameStack, all_tokens[token_index]->value) == NULL){
            
            error_exit(ERR_DEFINE);
        }
        expect(TOKEN_IDENTIFIER_FUNC);
        TokenType ret_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        type_compatibility(return_node->type, ret_type,false);
        function_call_rule(NULL, from_main);
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
    if(all_tokens[token_index]->type == TOKEN_LEFT_BRACKET && all_tokens[token_index-1]->type == TOKEN_IDENTIFIER){
        //printf("here\n");
        TokenType expr_type = search(frameStack, all_tokens[token_index-1]->value)->type;
        //printf("there\n");
        type_compatibility(return_node->type, expr_type,false);
        function_call_rule(NULL, from_main);
        build_instruction(instructionList, "MOVE", "LF@return_value", "TF@return_value", NULL);
        build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
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
    //build_instruction(instructionList, "DEFVAR", "LF@supa_giga_expr_res", NULL, NULL);
    //print_expression(expr);
    TokenType expr_type = parse_expression(expr, frameStack, from_main);
    type_compatibility(return_node->type, expr_type,false);

    expect(TOKEN_SEMICOLON);
    build_instruction(instructionList, "MOVE", "LF@return_value", "LF@supa_giga_expr_res", NULL); 
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
    skip_comments();
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
    build_instruction(instructionList, ".IFJcode24", NULL, NULL, NULL);
    build_instruction(instructionList, "JUMP", "main", NULL, NULL);
    program_rule();
    
     if(encountered_strcmp){
        build_strcmp();
     }
     if(encountered_substring){
        build_substring();
     }
     if(encoutered_main){
        print_instruction_list(instructionList);
     } else{
        syntax_error();
     }
    //printFrameStack(frameStack);
     //free(current_token->value);
     //free(current_token);
    
    return 0;
 }