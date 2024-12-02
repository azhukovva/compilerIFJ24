/**
* @file generator.c
 * IFJ24 Compiler
 *
 * @brief This file contains the implementation of the code generator used in the IFJ24 compiler.
 *        It includes functions for initializing the instruction list, building and pushing instructions,
 *        and printing the instruction list. Additionally, it provides utility functions for string
 *        manipulation and escape sequence handling.
 *
 * @details The code generator is responsible for generating intermediate code instructions for the
 *          IFJ24 compiler. It supports operations for creating and managing instructions, handling
 *          string operations, and generating specific functions like string comparison and substring
 *          extraction.
 *
 * @authors
 *          Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

#include "generator.h"
#include <string.h>
#include <regex.h>

extern InstructionList *instructionList;

InstructionList *init_instruction_list() {
    InstructionList *list = (InstructionList *)malloc(sizeof(InstructionList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

// push the instruction to the end of the list
void push_instruction(InstructionList *il, Instruction *instruction) {
    if (il->head == NULL) {
        il->head = instruction;
        il->tail = instruction;
    } else {
        il->tail->next = instruction;
        il->tail = instruction;
    }
}


// create new instruction and push it to the list
void build_instruction(InstructionList *il, char *opcode, char *arg1, char *arg2, char *arg3) {
    Instruction *instruction = (Instruction *)malloc(sizeof(Instruction));
    instruction->opcode = opcode;
    instruction->args = NULL;
    if (arg1 != NULL) {
        Arg *arg = (Arg *)malloc(sizeof(Arg));
        arg->value = arg1;
        arg->next = NULL;
        instruction->args = arg;
        if (arg2 != NULL) {
            Arg *arg = (Arg *)malloc(sizeof(Arg));
            arg->value = arg2;
            arg->next = NULL;
            instruction->args->next = arg;
            if (arg3 != NULL) {
                Arg *arg = (Arg *)malloc(sizeof(Arg));
                arg->value = arg3;
                arg->next = NULL;
                instruction->args->next->next = arg;
            }
        }
    }
    push_instruction(il, instruction);
}

// print the instruction
void print_instruction(Instruction *instruction) {
    printf("%s ", instruction->opcode);
    Arg *arg = instruction->args;
    while (arg != NULL) {
        printf("%s ", arg->value);
        arg = arg->next;
    }
    printf("\n");
    if(!strcmp(instruction->opcode, "RETURN")){
        printf("\n");
    }
}

//print the whole list of instructions
void print_instruction_list(InstructionList *il) {
    Instruction *current = il->head;
    Instruction *while_start = NULL;
    bool ignoreDefvar = false;
    char *while_case = NULL;
    // print variable definitions out the while loops
    while (current != NULL) {
        if (!ignoreDefvar && !strcmp(current->opcode, "LABEL") && strstr(current->args->value, "while_case_") != NULL) {
            int i = 11;
            while_case = (char *)malloc(strlen(current->args->value) - 10);
            while_case[0] = '\0';
            while (current->args->value[i] != '\0') {
                char tmp[2] = {current->args->value[i], '\0'};
                strcat(while_case, tmp);
                i++;
            }
            while_start = current;
            char *end_case_label = (char *)malloc(strlen("while_end_case_") + strlen(while_case) + 1);
            strcpy(end_case_label, "while_end_case_");
            strcat(end_case_label, while_case);
            while (!(!strcmp(current->opcode, "LABEL") && (!strcmp(current->args->value, end_case_label)))) {
                if (!strcmp(current->opcode, "DEFVAR") && strstr(current->args->value, "TF@") == NULL) {
                    print_instruction(current);
                    current = current->next;
                } else {
                    current = current->next;
                }
            }
            free(end_case_label);
            current = while_start;
            ignoreDefvar = true;
        }
        if (ignoreDefvar && !strcmp(current->opcode, "DEFVAR") && strstr(current->args->value, "TF@") == NULL) {
            current = current->next;
            continue;
        }
        if (while_case != NULL) {
            char *end_case_label = (char *)malloc(strlen("while_end_case_") + strlen(while_case) + 1);
            strcpy(end_case_label, "while_end_case_");
            strcat(end_case_label, while_case);
            if (!strcmp(current->opcode, "LABEL") && (!strcmp(current->args->value, end_case_label))) {
                ignoreDefvar = false;
            }
            free(end_case_label);
        }

        print_instruction(current);
        current = current->next;
    }
    free(while_case);
}

// concatenate two strings
char *_strcat(const char *str1, const char *str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char *result = (char *)malloc(len1 + len2 + 1); // +1 for the null terminator

    if (result == NULL) {
        return NULL;
    }

    strcpy(result, str1);
    strcat(result, str2);

    return result;
}

char *itoa(int n) {
    char *s = (char *)malloc(12);
    int i, sign;
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
    return s;
}
void reverse(char *str) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;
    char temp;

    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// function to build instructions for built-in ifj.strcmp function
void build_strcmp(){
    build_instruction(instructionList, "LABEL", "ifj_strcmp", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s2", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@i", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@char1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@char2", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@len1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@len2", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@cond", NULL, NULL);
    build_instruction(instructionList, "MOVE", "LF@s1", "LF@param1", NULL);
    build_instruction(instructionList, "MOVE", "LF@s2", "LF@param2", NULL);
    build_instruction(instructionList, "MOVE", "LF@i", "int@0", NULL);

    // Loop to compare characters
    build_instruction(instructionList, "LABEL", "label_loop", NULL, NULL);
    build_instruction(instructionList, "STRLEN", "LF@len1", "LF@s1", NULL);
    build_instruction(instructionList, "STRLEN", "LF@len2", "LF@s2", NULL);
    build_instruction(instructionList, "LT", "LF@cond", "LF@i", "LF@len1");
    build_instruction(instructionList, "JUMPIFEQ", "label_end", "LF@cond", "bool@false");
    build_instruction(instructionList, "LT", "LF@cond", "LF@i", "LF@len2");
    build_instruction(instructionList, "JUMPIFEQ", "label_end", "LF@cond", "bool@false");

    build_instruction(instructionList, "GETCHAR", "LF@char1", "LF@s1", "LF@i");
    build_instruction(instructionList, "GETCHAR", "LF@char2", "LF@s2", "LF@i");
    build_instruction(instructionList, "EQ", "LF@cond", "LF@char1", "LF@char2");
    build_instruction(instructionList, "JUMPIFEQ", "label_continue", "LF@cond", "bool@true");

    build_instruction(instructionList, "LT", "LF@return_value", "LF@char1", "LF@char2");
    build_instruction(instructionList, "JUMPIFEQ", "label_lt", "LF@return_value", "bool@true");
    build_instruction(instructionList, "GT", "LF@return_value", "LF@char1", "LF@char2");
    build_instruction(instructionList, "JUMPIFEQ", "label_gt", "LF@return_value", "bool@true");

    build_instruction(instructionList, "LABEL", "label_continue", NULL, NULL);
    build_instruction(instructionList, "ADD", "LF@i", "LF@i", "int@1");
    build_instruction(instructionList, "JUMP", "label_loop", NULL, NULL);

    // End of comparison
    build_instruction(instructionList, "LABEL", "label_end", NULL, NULL);
    build_instruction(instructionList, "STRLEN", "LF@len1", "LF@s1", NULL);
    build_instruction(instructionList, "STRLEN", "LF@len2", "LF@s2", NULL);
    build_instruction(instructionList, "LT", "LF@return_value", "LF@len1", "LF@len2");
    build_instruction(instructionList, "JUMPIFEQ", "label_lt", "LF@return_value", "bool@true");
    build_instruction(instructionList, "GT", "LF@return_value", "LF@len1", "LF@len2");
    build_instruction(instructionList, "JUMPIFEQ", "label_gt", "LF@return_value", "bool@true");

    // s1 == s2
    build_instruction(instructionList, "MOVE", "LF@return_value", "int@0", NULL);
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);

    // s1 < s2
    build_instruction(instructionList, "LABEL", "label_lt", NULL, NULL);
    build_instruction(instructionList, "MOVE", "LF@return_value", "int@-1", NULL);
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);

    // s1 > s2
    build_instruction(instructionList, "LABEL", "label_gt", NULL, NULL);
    build_instruction(instructionList, "MOVE", "LF@return_value", "int@1", NULL);

    // End
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);

}

// function to build instructions for built-in ifj.substring function
void build_substring(){
    build_instruction(instructionList, "LABEL", "ifj_substring", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@i", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@j", NULL, NULL);
    build_instruction(instructionList, "MOVE", "LF@s1", "LF@param1", NULL);
    build_instruction(instructionList, "MOVE", "LF@i", "LF@param2", NULL);
    build_instruction(instructionList, "MOVE", "LF@i", "LF@param3", NULL);

    build_instruction(instructionList, "DEFVAR", "LF@tmp_i", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@tmp_j", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@tmp_length", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@tmp_char", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@tmp_cond", NULL, NULL);
    // Check if i < 0
    build_instruction(instructionList, "LT", "LF@return_value", "LF@i", "int@0");
    build_instruction(instructionList, "JUMPIFEQ", "label_null", "LF@return_value", "bool@true");

    // Check if j < 0
    build_instruction(instructionList, "LT", "LF@return_value", "LF@j", "int@0");
    build_instruction(instructionList, "JUMPIFEQ", "label_null", "LF@return_value", "bool@true");

    // Check if i > j
    build_instruction(instructionList, "GT", "LF@return_value", "LF@i","LF@j");
    build_instruction(instructionList, "JUMPIFEQ", "label_null", "LF@return_value", "bool@true");

    // Check if i >= ifj.length(s)
    build_instruction(instructionList, "STRLEN", "LF@tmp_length", "LF@s1", NULL);
    build_instruction(instructionList, "LT", "LF@return_value", "LF@i", "LF@tmp_length");
    build_instruction(instructionList, "NOT", "LF@return_value", "LF@return_value", NULL);
    build_instruction(instructionList, "JUMPIFEQ", "label_null", "LF@return_value", "bool@true");

    // Check if j > ifj.length(s)
    build_instruction(instructionList, "GT", "LF@return_value","LF@j", "LF@tmp_length");
    build_instruction(instructionList, "JUMPIFEQ", "label_null", "LF@return_value", "bool@true");
    // Initialize loop variables
    build_instruction(instructionList, "MOVE", "LF@tmp_i", "LF@i", NULL);
    build_instruction(instructionList, "MOVE", "LF@tmp_j","LF@j", NULL);
    build_instruction(instructionList, "MOVE", "LF@return_value", "string@", NULL);

    // Loop to extract substring
    build_instruction(instructionList, "LABEL", "label_loop", NULL, NULL);
    build_instruction(instructionList, "GETCHAR", "LF@tmp_char", "LF@s1", "LF@tmp_i");
    build_instruction(instructionList, "CONCAT", "LF@return_value", "LF@return_value", "LF@tmp_char");
    build_instruction(instructionList, "ADD", "LF@tmp_i", "LF@tmp_i", "int@1");
    build_instruction(instructionList, "LT", "LF@tmp_cond", "LF@tmp_i", "LF@tmp_j");
    build_instruction(instructionList, "JUMPIFEQ", "label_loop", "LF@tmp_cond", "bool@true");

    // End
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);

    // Null case
    build_instruction(instructionList, "LABEL", "label_null", NULL, NULL);
    build_instruction(instructionList, "MOVE", "LF@return_value", "nil@nil", NULL);
    build_instruction(instructionList, "POPFRAME", NULL, NULL, NULL);
    build_instruction(instructionList, "RETURN", NULL, NULL, NULL);
}


// function to convert a string to a ifj24code format
char* escape_sequence(char *s) {
    char* string_id = "string@";
    int strlength = strlen(s);
    regex_t regex;
    regmatch_t pmatch[1];
    const char *pattern = "/x[0-9A-Fa-f]{2}";

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        printf("Could not compile regex\n");
        return NULL;
    }

    for (int i = 0; i < strlength; i++) {
		char slash[3] = {s[i], s[i+1]};
		if (strcmp(slash, "\\n") == 0) {
		    string_id = _strcat(string_id, "\\010");
			i++;
			continue;
		}
		if (strcmp(slash, "\\t") == 0) {
		    string_id = _strcat(string_id, "\\009");
			i++;
			continue;
		}
		if (strcmp(slash, "\\r") == 0) {
		    string_id = _strcat(string_id, "\\013");
			i++;
			continue;
		}
        if (strcmp(slash, "\\\"") == 0) {
		    string_id = _strcat(string_id, "\\034");
			i++;
			continue;
		}
        if (regexec(&regex, &s[i], 1, pmatch, 0) == 0) {
            char hex[3] = {s[i+3], s[i+4], '\0'};
            int decimal = (int)strtol(hex, NULL, 16);
            char tmp[4];
            sprintf(tmp, "%d", decimal);
            string_id = _strcat(string_id, tmp);
            i += 4; // Skip the matched pattern
        } else if (((int)s[i] <= 32 && (int)s[i] > 0) || (int)s[i] == 35 || (int)s[i] == 92) {
            char* tmp = (char*)malloc(4);
            sprintf(tmp, "%03d", (int)s[i]);
            string_id = _strcat(string_id, "\\");
            string_id = _strcat(string_id, tmp);
        } else {
            char tmp[2] = {s[i], '\0'}; // Convert char to string
            string_id = _strcat(string_id, tmp);
        }
    }
    regfree(&regex);
	//printf("%s\n", string_id); //debug print
    return string_id;
}

