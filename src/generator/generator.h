/**
* @file generator.h
 * IFJ24 Compiler
 *
 * @brief Header file for the code generator implementation. It contains type definitions and function
 *        declarations for initializing the instruction list, building and pushing instructions, and
 *        printing the instruction list. Additionally, it provides utility functions for string
 *        manipulation and escape sequence handling.
 *
 * @details This file provides the interface for the code generator operations used in the IFJ24 compiler.
 *          The code generator is responsible for generating intermediate code instructions, handling
 *          string operations, and generating specific functions like string comparison and substring
 *          extraction.
 *
 * @authors
 *          Zhdanovich Iaroslav (xzhdan00)
 *          Denys Malytskyi (xmalytd00)
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

typedef struct Arg {
    char *value;
    struct Arg *next;
} Arg;

typedef struct Instruction {
    char *opcode;
    Arg *args;
    struct Instruction *next;
} Instruction;

typedef struct InstructionList {
    Instruction *head;
    Instruction *tail;
} InstructionList;

InstructionList *init_instruction_list();
void push_instruction(InstructionList *il, Instruction *instruction);
void build_instruction(InstructionList *il, char *opcode, char *arg1, char *arg2, char *arg3);
void print_instruction(Instruction *instruction);
void print_instruction_list(InstructionList *il);
char *_strcat(const char *s1, const char *s2);
char *itoa(int n);
void reverse(char *str);
void build_strcmp();
void build_substring();
char* escape_sequence(char *s);
#endif //GENERATOR_H
