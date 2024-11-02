#include "topDown.h"


extern const char *tokenName[];

//Current token pointer
Token *current_token;

//Get the next token
void next_token(){
    get_token(current_token);
}

//Error handling 
void syntax_error(){
    error_exit(ERR_SYNTAX);
}

//Compare current token type and expected token type. Get the next token if they match. 
void expect(TokenType type){
    if(current_token->type == type){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the id of token value matches expected one
void expect_id(char *id_to_check){
    printf("type: %s val: %s id_to_check:%s\n", tokenName[current_token->type], current_token->value, id_to_check);
    if(current_token->type == TOKEN_IDENTIFIER && !strcmp(current_token->value, id_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the string in token matches expected one
void expect_string(char *string_to_check){
    if(current_token->type == TOKEN_STRING && !strcmp(current_token->value, string_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

// <Program> ::= <Prologue> <functions> <main_func> eof
void program_rule(){
    prologue_rule();
    functions_rule();
    main_func_rule();
    //expect(TOKEN_EOF);
}
// <Prologue> ::= const ifj = @import(“ifj24.zig”) ;
void prologue_rule(){
   /* 
    *we cannot proccess this yet :)
    expect(TOKEN_CONST); 
    expect_id("ifj"); //ifj
    expect(TOKEN_ASSIGN);
    expect_id(TOKEN_IDENTIFIER); //@import
    expect(TOKEN_LEFT_BRACKET);
    expect_string("ifj24.zig"); //ifj24.zig
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
    
    */
}
// <functions> ::= <function> <functions_tail>
void functions_rule(){
    function_rule();
    functions_tail_rule();
}

// <functions_tail> ::= <function> <functions_tail> | eps
void functions_tail_rule(){
    if(current_token->type == TOKEN_PUB){
        function_rule();
        functions_tail_rule();
    }
}

// <function> ::= pub fn func_id <Param_list> <Return_type> <Block>
void function_rule(){
    expect(TOKEN_PUB);
    expect(TOKEN_FN);
    expect(TOKEN_IDENTIFIER);
   //expect(TOKEN_IDENTIFIER_FUNC); our lexer cannot differ identifiers
    param_list_rule();
    return_type_rule();
    block_rule();
}

//<Param_list> ::= ( <Params> ) 
void param_list_rule(){
    expect(TOKEN_LEFT_BRACKET);
    params_rule();
    expect(TOKEN_RIGHT_BRACKET);
}

// <Params> ::= <Param> <Param_tail> | eps
void params_rule(){
    if(current_token->type == TOKEN_IDENTIFIER){
        param_rule();
        param_tail_rule();
    }
}

// <Param_tail> ::= , <Param> <Param_tail> | eps
void param_tail_rule(){
    // ,
    if(current_token->type == TOKEN_COMMA){
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
    param_type_rule();
}

// <Param_type> ::= <Optional_question_mark> <Base_type>
void param_type_rule(){
    optional_q_mark_rule();
    base_type_rule();
}

// <Optional_question_mark> ::= ? | eps
void optional_q_mark_rule(){
    //?
    if(current_token->type == TOKEN_OPTIONAL_TYPE){
        expect(TOKEN_OPTIONAL_TYPE);
    }
    //eps
}

// <Base_type> ::= i32 | f64 | []u8
void base_type_rule(){
    if(current_token->type == TOKEN_I32){
        expect(TOKEN_I32);
    } else if(current_token->type == TOKEN_F64){
        expect(TOKEN_F64);
    } else if(current_token->type == TOKEN_U8){
        expect(TOKEN_U8);
    } else {
        syntax_error();
    }
}

// <Return_type> ::= <Optional_question_mark> <Base_type>
void return_type_rule(){
    if(current_token->type == TOKEN_VOID){
        expect(TOKEN_VOID);
    } else {
        optional_q_mark_rule();
        base_type_rule();
    }
}

// <Block> ::= { <Statements> } 
void block_rule(){
    expect(TOKEN_LEFT_BRACE);
    statements_rule();
    expect(TOKEN_RIGHT_BRACE);
}

// <Statements> ::= <Statement> <Statements> | eps
void statements_rule(){
    while(is_statement_start(current_token->type)){
        statement_rule();
    }
}

// Function that defines whether or not we have any statements to parse
bool is_statement_start(TokenType type){
    return type == TOKEN_CONST || type == TOKEN_VAR || type == TOKEN_IDENTIFIER ||
           type == TOKEN_IF || type == TOKEN_WHILE || type == TOKEN_RETURN;
}

void statement_rule(){
    // To be implemented
    expect(TOKEN_RETURN);
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_SEMICOLON);
}

//<main_func> ::= pub fn main () void <Block>
void main_func_rule(){
    // To be implemented
}


int main(){
    
    current_token = init_token();
    next_token();
    printf("Token: %s, %s\n", tokenName[current_token->type], current_token->value);
    program_rule();

    printf("yep\n");
    //free(current_token->value);
    free(current_token);
    
}