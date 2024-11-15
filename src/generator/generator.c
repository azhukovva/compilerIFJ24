#include "generator.h"




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