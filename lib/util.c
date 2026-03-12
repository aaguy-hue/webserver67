#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "util.h"

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
    while (isspace((unsigned char)str[begin]) || str[begin] == '\n' || str[begin] == '\r') {
        begin++;
    }

    // Find the index of the last non-whitespace character
    while (end >= begin && (isspace((unsigned char)str[end]) || str[end] == '\n' || str[end] == '\r' )) {
        end--;
    }

    // Shift all non-whitespace characters to the start of the string
    for (i = begin; i <= end; i++) {
        str[i - begin] = str[i];
    }

    // Null-terminate the new, shorter string
    str[i - begin] = '\0';
}

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

int minInt(int num1, int num2) {
    return (num1 < num2) ? num1 : num2;
}

bool strIsNumeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return false;
    }

    size_t sLen = strlen(str);
    for (size_t i = 0; i < sLen; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

bool fileExtensionMatches(const char *fileName, const char *suffix) {
    if (fileName == NULL || suffix == NULL) {
        return false;
    }

    fileName = strrchr(fileName, '.');
    // printf("[+] Checking file extension. Target: %s, suffix: %s\n", fileName, suffix);
    if (fileName == NULL) {
        return false; // can't find an extension
    }
    return strcmp(fileName, suffix) == 0;
}