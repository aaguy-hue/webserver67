#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdbool.h>

bool isDirectory(const char *path);

void generateDirectoryListing(char *folderPath, char *folderName, char **outStr, int outStrSize);

bool compressFile(char **inputFileName, const char *acceptEncoding);

#endif