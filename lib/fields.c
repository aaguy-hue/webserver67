#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "hashmap.h"
#include "fields.h"

/**
 * Btw make sure your string is properly null-terminated
 * or this is going to segfault and destroy you
 *
 * Note: this modifies the string in place
*/
void strToLower(char *str) {
	for (int i = 0; str[i] != '\0'; i++) {
		str[i] = (char)tolower((unsigned char) str[i]);
	}
}

int field_compare(const void *a, const void *b, void *fdata)
{
	(void) fdata;
	// comparison based on field name
	const Field *fa = a;
	const Field *fb = b;
	return strcmp(fa->name, fb->name);
}
/*
bool field_iter(const void *item, void *fdata)
{
	(void) item;
	(void) fdata;
	//const Field *f = item;
	return true;
}
*/
uint64_t field_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
	const Field *f = item;
	return hashmap_sip(f->name, strlen(f->name), seed0, seed1);
}

/**
 * Trim leading and trailing white space characters from a string in place.
 *
 * @param str The string to trim.
 */
void trim(char *str) {
    int i;
    int begin = 0;
    int end = strlen(str) - 1;

    // Find the index of the first non-whitespace character
    while (isspace((unsigned char)str[begin])) {
        begin++;
    }

    // Find the index of the last non-whitespace character
    while (end >= begin && isspace((unsigned char)str[end])) {
        end--;
    }

    // Shift all non-whitespace characters to the start of the string
    for (i = begin; i <= end; i++) {
        str[i - begin] = str[i];
    }

    // Null-terminate the new, shorter string
    str[i - begin] = '\0';
}

struct hashmap *readRequest(char *buf) {
	// 2nd param is initial capacity
	struct hashmap *map = hashmap_new(sizeof(Field), 10, 0, 0, field_hash, field_compare, NULL, NULL);

	char *p = buf;
	while (*p)
	{
		// assume no carriage return (\r) for simplicity
		// since i would otherwise have to set two \0 chars
		size_t len = strcspn(p, "\n");
		if (len == 0) {
			break; // reached blank line, headers over
		}

		bool goPast = false;
		if (p[len] == '\n') {
			p[len] = '\0';
			goPast = true;
		}

		char *colon = strchr(p, ':');
		if (!colon) {
			printf("[-] Malformed input detected! Line: %s\n", p);
			p += len;
			if (goPast) p++;
			continue;
		}
		*colon = '\0';
		char *str1 = p;
		char *str2 = colon+1;

		Field field;
		strncpy(field.name, str1, FIELD_NAME_MAXLEN-1);
		field.name[FIELD_NAME_MAXLEN-1] = '\0';
		trim(field.name);
		strToLower(field.name);

		strncpy(field.value, str2, FIELD_VALUE_MAXLEN-1);
		field.value[FIELD_VALUE_MAXLEN-1] = '\0';
		trim(field.value);

		const char *existing = getHeader(map, field.name);
		if (existing != NULL) {
			char tmp[FIELD_VALUE_MAXLEN];
			memset(tmp, 0, FIELD_VALUE_MAXLEN);

			strncpy(tmp, field.value, FIELD_VALUE_MAXLEN-1);
			tmp[FIELD_VALUE_MAXLEN-1] = '\0';

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
			snprintf(field.value, FIELD_VALUE_MAXLEN-1,
					"%s, %s", existing, tmp);
#pragma GCC diagnostic pop
		}

		hashmap_set(map, &field);

		p += len;
		if (goPast) p++;
		else break;
	}

	printf("Skibidi: %s\n", getHeader(map, "Skibidi"));

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

