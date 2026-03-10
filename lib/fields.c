#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "hashmap.h"
#include "fields.h"
#include "util.h"

int field_compare(const void *a, const void *b, void *fdata)
{
	(void) fdata;
	// comparison based on field name
	const Field *fa = a;
	const Field *fb = b;
	return strcmp(fa->name, fb->name);
}

// bool field_iter(const void *item, void *fdata)
// {
// 	// (void) item;
// 	(void) fdata;
// 	const Field *f = item;
// 	printf("Field%s: %s\n", f->name, f->value);
// 	return true;
// }

uint64_t field_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
	const Field *f = item;
	return hashmap_sip(f->name, strlen(f->name), seed0, seed1);
}

Field createField(const char *name, const char *value) {
	Field field;
	strncpy(field.name, name, FIELD_NAME_MAXLEN-1);
	field.name[FIELD_NAME_MAXLEN-1] = '\0';
	trim(field.name);
	strToLower(field.name);

	strncpy(field.value, value, FIELD_VALUE_MAXLEN-1);
	field.value[FIELD_VALUE_MAXLEN-1] = '\0';
	trim(field.value);

	return field;
}

struct hashmap *createFieldHashmap(size_t initial_capacity) {
	return hashmap_new(sizeof(Field), initial_capacity, 0, 0, field_hash, field_compare, NULL, NULL);
}

struct hashmap *readRequest(char **buf) {
	// 2nd param is initial capacity
	struct hashmap *map = createFieldHashmap(20);

	while (**buf)
	{
		size_t len = strcspn(*buf, "\n");
		if (len == 0) {
			break; // reached blank line, headers over
		}

		if ((*buf)[len] == '\n') {
			(*buf)[len] = '\0';
		}

		trim(*buf);
		if (strcmp(*buf, "") == 0) {
			// this will go past the null char
			(*buf)+=2;
			break;
		}

		char *colon = strchr(*buf, ':');
		if (!colon) {
			printf("[-] Malformed input detected! Line: %s\n", (*buf));
			(*buf) += len+1;
		}
		*colon = '\0';
		char *str1 = *buf;
		char *str2 = colon+1;

		Field field = createField(str1, str2);
		// Field field;
		// strncpy(field.name, str1, FIELD_NAME_MAXLEN-1);
		// field.name[FIELD_NAME_MAXLEN-1] = '\0';
		// trim(field.name);
		// strToLower(field.name);

		// strncpy(field.value, str2, FIELD_VALUE_MAXLEN-1);
		// field.value[FIELD_VALUE_MAXLEN-1] = '\0';
		// trim(field.value);

		const char *existing = getHeader(map, field.name);
		if (existing != NULL) {
			char tmp[FIELD_VALUE_MAXLEN];
			memset(tmp, 0, FIELD_VALUE_MAXLEN);

			strncpy(tmp, field.value, FIELD_VALUE_MAXLEN-1);
			tmp[FIELD_VALUE_MAXLEN-1] = '\0';

			// gcc warns here that FIELD_VALUE_MAXLEN will be exceeded
			// since 2 things of same length are being added here
			// perhaps there's a better design choice I can make but the pragma
			// just hides the error bc i'm too lazy
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
			snprintf(field.value, FIELD_VALUE_MAXLEN-1,
					"%s, %s", existing, tmp);
#pragma GCC diagnostic pop
		}

		hashmap_set(map, &field);

		(*buf) += len + 1;
	}

	return map;
}

const char *getHeader(struct hashmap *map, const char *header) {
	Field lookupField = {0};

	strncpy(lookupField.name, header, FIELD_NAME_MAXLEN-1);
	lookupField.name[FIELD_NAME_MAXLEN - 1] = '\0';

	strToLower(lookupField.name);

	const Field *f = hashmap_get(map, &lookupField);

	if (f) {
		return f->value;
	}
	return NULL;
}

