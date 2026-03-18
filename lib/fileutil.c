#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileutil.h"

bool isDirectory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        perror("[-] stat failed");
        return false;
    }

#ifdef _WIN32
    return path_stat.st_mode & _S_IFDIR != 0;
#else
    return S_ISDIR(path_stat.st_mode);
#endif
}

bool fileExists(const char *path) {
    struct stat path_stat;
    return stat(path, &path_stat) == 0;
}

long long fileSize(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        perror("[-] stat failed");
        return -1;
    }
    return (long long) path_stat.st_size;
}


#ifdef _WIN32

// too lazy to work on compatibility rn since I'd need to rewrite my Makefile to allow for windows

#else

#include <dirent.h>
#include "request.h"

void generateDirectoryListing(char *folderPath, char *folderName, char **outStr, int outStrSize) {
    
    const char *startStr = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Directory Listing</title></head><body><h1>Listing for ";
    const char *endStr = "</ul></body></html>";
    
    strncat(*outStr, startStr, outStrSize - 1);
    strncat(*outStr, folderName, outStrSize - strlen(*outStr) - 1);
    strncat(*outStr, "</h1><ul>", outStrSize - strlen(*outStr) - 1);
    
    printf("[+] Generating directory listing for folder: %s. Start of generated HTML: %s\n", folderPath, *outStr);
    DIR *dir = opendir(folderPath);
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        // todo: comparing against DT_DIR is the most efficient way, but it's less portable
        // should probably use the isDirectory function instead, but stat'ing every file is slow since each stat requires a syscall
        // for now, I will take the L and accept the fact that this won't work on all systems
        if (entry->d_type == DT_DIR) {
            snprintf(*outStr + strlen(*outStr), outStrSize - strlen(*outStr) - 1, "<li><a href=\"%s/%s/\">[DIR] %s/</a></li>", folderName, entry->d_name, entry->d_name);
        } else {
            snprintf(*outStr + strlen(*outStr), outStrSize - strlen(*outStr) - 1, "<li><a href=\"%s/%s\">[FILE] %s</a></li>", folderName, entry->d_name, entry->d_name);
        }
    }

    closedir(dir);

    strncat(*outStr, endStr, outStrSize - strlen(*outStr) - 1);
    (*outStr)[outStrSize - 1] = '\0';
}
#endif

#include "zlib.h"

#define COMPRESSION_CHUNK_SIZE 8192

bool acceptsGzipEncoding(const char *acceptEncoding) {
    return strstr(acceptEncoding, "gzip") != NULL;
}

#include "config.h"

// note: we expect that outFileName is a pointer to fileName with enough space to hold the additional .gz extension, and that acceptEncoding is a string containing the value of the Accept-Encoding header from the request
// if the client supports gzip encoding and we successfully create a compressed version of the file, we update the fileName to point to the compressed version and return true, otherwise we return false and leave fileName unchanged
bool compressFile(char **inputFileName, const char *acceptEncoding) {
    if (!acceptsGzipEncoding(acceptEncoding)) {
        printf("[-] Client does not support gzip encoding. Skipping compression for file: %s\n", *inputFileName);
        return false;
    }

    // todo: hardcoding in SITE_PATH_MAX + 200 is really gross, should probably define a constant for this
    char outputFileName[SITE_PATH_MAX + 200];
    strncpy(outputFileName, *inputFileName, SITE_PATH_MAX + 200);
    outputFileName[SITE_PATH_MAX + 200] = '\0';

    char buffer[COMPRESSION_CHUNK_SIZE];
    size_t bytesRead;
    FILE *inputFile = fopen(*inputFileName, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Error opening input file for compression\n");
        return false;
    }

    strcat(outputFileName, ".gz");
    gzFile outputFile = gzopen(outputFileName, "wb");
    if (outputFile == NULL) {
        fprintf(stderr, "Error opening gzip file for writing\n");
        fclose(inputFile);
        return false; // return still uncompressed file if we fail to create compressed version
    }
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0) {
        if (gzwrite(outputFile, buffer, bytesRead) < 0) {
            fprintf(stderr, "Error writing to gzip file\n");
            // Handle error, close files
            fclose(inputFile);
            gzclose(outputFile);
            return false; // return still uncompressed file if we fail to create compressed version
        }
    }

    gzclose(outputFile);
    fclose(inputFile);

    strncpy(*inputFileName, outputFileName, SITE_PATH_MAX + 200);
    (*inputFileName)[SITE_PATH_MAX + 200] = '\0';
    return true;
}
