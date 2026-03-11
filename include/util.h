#ifndef WEBSERVER_UTIL_H

#define WEBSERVER_UTIL_H

#include <stdbool.h>

void trim(char *str);

void strToLower(char *str);

int minInt(int num1, int num2);

bool strIsNumeric(const char *str);

#endif
