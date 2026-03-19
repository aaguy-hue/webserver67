#ifndef ARRAYLISTSTR_H
#define ARRAYLISTSTR_H

#include <stdlib.h>

typedef struct {
    char **array;
    size_t size;
    size_t capacity;

    void (*append) (struct ArrayListStr *list, const char *str);
    char *(*get) (struct ArrayListStr *list, const char *str);
    void (*free) (struct ArrayListStr *list);
} ArrayListStr;

#endif