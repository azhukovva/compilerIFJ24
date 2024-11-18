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


void push_instruction(InstructionList *il, Instruction *instruction) {
    if (il->head == NULL) {
        il->head = instruction;
        il->tail = instruction;
    } else {
        il->tail->next = instruction;
        il->tail = instruction;
    }
}

//build_instruction(il, "ADD", strcat("LF@", token->data->id), "int@5", null);

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

void print_instruction_list(InstructionList *il) {
    Instruction *current = il->head;
    while (current != NULL) {
        print_instruction(current);
        current = current->next;
    }
}

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

void build_strcmp(){
    build_instruction(instructionList, "LABEL", "ifj_strcmp", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s2", NULL, NULL);
    build_instruction(instructionList, "MOV", "LF@s1", "LF@param1", NULL);
    build_instruction(instructionList, "MOV", "LF@s2", "LF@param2", NULL);
    // Compare s1 < s2
    build_instruction(instructionList, "LT", "LF@return_value", "LF@s1", "LF@s2");
    build_instruction(instructionList, "JUMPIFEQ", "label_lt", "LF@return_value", "bool@true");

    // Compare s1 > s2
    build_instruction(instructionList, "GT", "LF@return_value", "LF@s1", "LF@s2");
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
void build_substring(){
    build_instruction(instructionList, "LABEL", "ifj_substring", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@s1", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@i", NULL, NULL);
    build_instruction(instructionList, "DEFVAR", "LF@j", NULL, NULL);
    build_instruction(instructionList, "MOV", "LF@s1", "LF@param1", NULL);
    build_instruction(instructionList, "MOV", "LF@i", "LF@param2", NULL);
    build_instruction(instructionList, "MOV", "LF@i", "LF@param3", NULL);

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
	printf("%s\n", string_id);
    return string_id;
}


