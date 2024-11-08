#include "topDown.h"



// extern const char *tokenName[];

//Current token pointer
Token *current_token;
bool encoutered_main = false;
bool encountered_return = false;

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
    // printf("Checking: type: %s val: %s\n", tokenName[current_token->type], current_token->value);
    if(current_token->type == type){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the id of token value matches expected one
void expect_id(char *id_to_check){
    //printf("type: %s val: %s id_to_check:%s\n", tokenName[current_token->type], current_token->value, id_to_check);
    if(current_token->type == TOKEN_IDENTIFIER && !strcmp(current_token->value, id_to_check)){
        next_token();
    } else {
        syntax_error();
    }
}

//Check if the string in token matches expected one
void expect_string(char *string_to_check){
    //printf("type: %s val: %s id_to_check:%s\n", tokenName[current_token->type], current_token->value, string_to_check);
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
    if(current_token->type == TOKEN_PUB){
        function_rule();
        functions_tail_rule();
    }
}

// <function> ::= pub fn func_id <Param_list> <Return_type> <Block>
void function_rule(){
    expect(TOKEN_PUB);
    expect(TOKEN_FN);
    if(!strcmp(current_token->value, "main")){
        main_func_rule();
    } else {
        expect(TOKEN_IDENTIFIER);
        param_list_rule();
        return_type_rule();
        block_rule();
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
    base_type_rule();
}



// <Base_type> ::= i32 | f64 | []u8
void base_type_rule(){
    switch (current_token->type)
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
    if(current_token->type == TOKEN_VOID){
        expect(TOKEN_VOID);
    } else {
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
           type == TOKEN_IF || type == TOKEN_WHILE || type == TOKEN_RETURN || type == TOKEN_IDENTIFIER_FUNC;
}

void statement_rule(){
    if(encountered_return){
        syntax_error();
    }
    switch (current_token->type){
        case TOKEN_CONST:
        case TOKEN_VAR:
            // printf("var\n");
            //printf("Checking token: type: %s val: %s\n", tokenName[current_token->type], current_token->value);
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
    var_mode_rule();
    expect(TOKEN_IDENTIFIER);
    var_type_rule();
    expect(TOKEN_ASSIGN);
    
    //expression_rule();
    
    //заглушка
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_SEMICOLON);
}

//<Var_mode> ::= const | var
void var_mode_rule(){
    if(current_token->type == TOKEN_CONST){
        expect(TOKEN_CONST);
    } else if(current_token->type == TOKEN_VAR){
        expect(TOKEN_VAR);
    } else {
        syntax_error();
    }
}

//<Var_type> ::= : <Base_type> | eps
void var_type_rule(){
    if(current_token->type == TOKEN_COLON){
        expect(TOKEN_COLON);
        param_type_rule();
    }
}
//<assigment> ::= id = <Expression> ;
void assigment_rule(){
    //inbuilt function call
    if(current_token->type == TOKEN_IDENTIFIER_FUNC){
        expect(TOKEN_IDENTIFIER_FUNC);;
        function_call_rule();
        return;
    } 
    expect(TOKEN_IDENTIFIER);

    //if the identifier was a user function identifier
    if(current_token->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    expect(TOKEN_ASSIGN);
    
    //inbuilt function call
    if(current_token->type == TOKEN_IDENTIFIER_FUNC){
        printf("here\n");
        expect(TOKEN_IDENTIFIER_FUNC);
        function_call_rule();
        return;
    }

    //expression_rule();
    //заглушка
    expect(TOKEN_IDENTIFIER);
    if(current_token->type == TOKEN_LEFT_BRACKET){
        function_call_rule();
        return;
    }
    expect(TOKEN_SEMICOLON);
}

//<function_call> ::= ( <Arguments> );
void function_call_rule(){
    printf("function call\n");
    expect(TOKEN_LEFT_BRACKET);
    arguments_rule();
    expect(TOKEN_RIGHT_BRACKET);
    expect(TOKEN_SEMICOLON);
    printf("function call end\n");
}

//<Arguments> ::= <Argument> <Arguments_tail>
void arguments_rule(){
    argument_rule();
    arguments_tail_rule();
}

//<Arguments_tail> ::= , <Argument> <Arguments_tail> | eps
void arguments_tail_rule(){
    if(current_token->type == TOKEN_COMMA){
        expect(TOKEN_COMMA);
        if(current_token->type == TOKEN_RIGHT_BRACKET){ // in case there is a comma before the last argument
            return;
        }
        argument_rule();
        arguments_tail_rule();
    }
}

//<Argument> ::= id | <Literal>
void argument_rule(){
    if(current_token->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
    } else {
        literal_rule();
    }
}

//<Literal> ::= int | float | string
void literal_rule(){
    switch (current_token->type) {
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
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_RIGHT_BRACKET);
    optional_null_rule();
    block_rule();
    optional_else_rule();
}

//<Optional_null> ::= |id| | eps
void optional_null_rule(){
  //  we can't process pipes yet
  //  if(current_token->type == TOKEN_PIPE){
  //      expect(TOKEN_PIPE);
  //      expect(TOKEN_IDENTIFIER);
  //      expect(TOKEN_PIPE);
  //  }
}

//<Optional_else> ::= else <else_body> | eps
void optional_else_rule(){
    if(current_token->type == TOKEN_ELSE){
        expect(TOKEN_ELSE);
        else_body_rule();
    }
}
//<else_body> ::= <Block> | <Conditionals>
void else_body_rule(){
    if(current_token->type == TOKEN_LEFT_BRACE){
        block_rule();
    } else if(current_token->type == TOKEN_IF){
        conditionals_rule();
    } else {
        syntax_error();
    }
}

//<while_statement> ::= while ( Expression ) <Block>
void while_statement_rule(){
    printf("while\n");
    expect(TOKEN_WHILE);
    expect(TOKEN_LEFT_BRACKET);
    //expression_rule();
    //заглушка
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_RIGHT_BRACKET);
    block_rule();
}

//<return> ::= return <Expression_opt> ;
void return_statement_rule(){
    expect(TOKEN_RETURN);
    expression_opt_rule();
    expect(TOKEN_SEMICOLON);
    encountered_return = true;
}

//<Expression_opt> ::= expression | id  | eps
void expression_opt_rule(){
     if(current_token->type == TOKEN_IDENTIFIER){
        expect(TOKEN_IDENTIFIER);
     }
}


// int main(){
    
//      current_token = init_token();
//      next_token();
// //     printf("Token: %s, %s\n", tokenName[current_token->type], current_token->value);
//      program_rule();
//      if(encoutered_main){
//         printf("yep\n");
//      } else{
//         syntax_error();
//      }
     
//      //free(current_token->value);
//      free(current_token);
//      return 0;
//  }