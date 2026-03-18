#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdbool.h>

bool isDirectory(const char *path);

void generateDirectoryListing(char *folderPath, char *folderName, char **outStr, int outStrSize);

bool compressFile(char **inputFileName, const char *acceptEncoding);

bool fileExists(const char *path);

long long fileSize(const char *path);

#endif