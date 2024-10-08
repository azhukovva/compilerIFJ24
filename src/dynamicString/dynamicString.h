/**
 * @file dynamicString.h
 * IFJ24 Compiler
 *
 * @brief Dynamic string implementation
 * @author Aryna Zhukava (xzhuka01)
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"

typedef struct
{
    char *str;
    int len;
} dynamicString;

/**
 * @brief Function to init a dynamic string
 * @param str - pointer to the dynamic string
 * @param size - size of string
 * @return void
 */
void init_string(dynamicString *str, int len);

/**
 * @brief Function to print dynamic string
 * @param str - pointer to the dynamic string
 * @return void
 */
void print_string(dynamicString *str);

/**
 * @brief Function to put char to the dynamic string
 * @param str - pointer to the dynamic string
 * @param c - char to append
 * @return void
 */
void append_string(dynamicString *str, char c);

/**
 * @brief Function to free the dynamic string
 * @param str - pointer to the dynamic string
 * @return void
 */
void free_string(dynamicString *str);