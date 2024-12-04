/**
 * @file dynamicString.c
 * IFJ24 Compiler
 *
 * @brief Dynamic string implementation
 * @author  Aryna Zhukava (xzhuka01)
 */

#include "dynamicString.h"

void init_string(dynamicString *str, int size)
{
    str->str = (char *)malloc(size);
    str->len = size;
}

void print_string(dynamicString *str)
{
    printf("%s\n", str->str);
}

void append_string(dynamicString *str, char c)
{
    if (str->len == 0)
    {
        str->str = (char *)malloc(1);
        str->len = 1;
    }
    else
    {
        str->str = (char *)realloc(str->str, str->len + 1);
        str->len++;
    }
    str->str[str->len - 1] = c;
}

void free_string(dynamicString *str)
{
    if (str == NULL)
        return;
    free(str->str);
    str->str = NULL;
    str->len = 0;
}
