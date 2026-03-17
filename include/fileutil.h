#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdbool.h>

bool isDirectory(const char *path);

void generateDirectoryListing(char *folderPath, char *folderName, char **outStr, int outStrSize);

char *compressFile(char *fileName, char **outFileName, const char *acceptEncoding, bool *successfullyCompressed);

#endif