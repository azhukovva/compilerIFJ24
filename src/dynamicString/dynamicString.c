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
    // if(str->len == 0){
    //     str->str = (char*)malloc(2);  // 1 char + 1 for '\0'(null terminator)
    //     str->len = 1;
    // }
    // else{
    //     str->str = (char*)realloc(str->str, str->len + 2); // 1 for char + 1 for '\0'
    //     str->len++;
    // }
    // str->str[str->len - 1] = c; // str->len - 1 - index of last character in the string
    // str->str[str->len] = '\0';

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

    //  if (str->len + 1 >= str->capacity) {
    //     str->capacity *= 2;          // zvětšujeme kapacitu na dvojnásobek
    //     str->str = realloc(str->str, str->capacity * sizeof(char));
    //     if (str->str == NULL) {
    //         fprintf(stderr, "Reallocation failed\n");
    //         exit(1);
    //     }
    // }
    // str->str[str->len] = c;          // přidání znaku
    // str->len++;                      // zvýšení délky
    // str->str[str->len] = '\0';
}

void free_string(dynamicString *str)
{
    if (str == NULL)
        return;
    free(str->str);
    str->str = NULL;
    str->len = 0;
}
