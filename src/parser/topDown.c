#include "topDown.h"
#include "../lexer/scanner.h"

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
    expect(TOKEN_EOF);
}

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
void functions_rule(){

}

void main_func_rule(){

}


/*int main(){
    
    current_token = init_token();
    next_token();

    program_rule();

    printf("yep\n");
    free_string(current_token->value);
    free(current_token);
    
}*/