#include "arrayliststr.h"

void createArrayListStr(int capacity) {
    ArrayListStr *list = malloc(sizeof(ArrayListStr));
    if (list == NULL) {
        printf("[-] Failed to allocate memory for ArrayListStr");
        exit(EXIT_FAILURE);
    }

    list->array = malloc(sizeof(char *) * capacity);
    if (list->array == NULL) {
        printf("[-] Failed to allocate memory for ArrayListStr array");
        free(list);
        exit(EXIT_FAILURE);
    }
    list->size = 0;
    list->capacity = capacity;

    list->append = &appendToArrayList;
    list->get = &getFromArrayList;
    list->free = &freeArrayListStr;
    return list;
}

void appendToArrayList(ArrayListStr *list, const char *str) {
    if (list->size >= list->capacity) {
        size_t newCap = list->capacity * 2;
        char **newArray = realloc(list->array, sizeof(char *) * newCap);
        if (newArray == NULL) {
            printf("[-] Failed to reallocate memory for ArrayListStr array");
            freeArrayListStr(list);
            exit(EXIT_FAILURE);
        }
        list->array = newArray;
        list->capacity = newCap;
    }

    list->array[list->size] = strdup(str);
    if (list->array[list->size] == NULL) {
        printf("[-] Failed to duplicate string for ArrayListStr");
        freeArrayListStr(list);
        exit(EXIT_FAILURE);
    }
    list->size++;
}

char *getFromArrayList(ArrayListStr *list, const char *str) {
    for (size_t i = 0; i < list->size; i++) {
        if (strcmp(list->array[i], str) == 0) {
            return list->array[i];
        }
    }
    return NULL;
}

void freeArrayListStr(ArrayListStr *list) {
    for (size_t i = 0; i < list->size; i++) {
        free(list->array[i]);
    }
    free(list->array);
    free(list);
}
