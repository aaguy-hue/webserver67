#ifndef FIELDS_H
#define FIELDS_H

#include "hashmap.h"

#define FIELD_NAME_MAXLEN 200
#define FIELD_VALUE_MAXLEN 1000

typedef struct {
        char name[FIELD_NAME_MAXLEN];
        char value[FIELD_VALUE_MAXLEN];
} Field;

void readHeaders(struct hashmap *map, char **buf);

const char *getHeader(struct hashmap *map, const char *header);
const char *popHeader(struct hashmap *map, const char *header);

struct hashmap *createFieldHashmap(size_t initial_capacity);

Field createField(const char *name, const char *value);

#endif
