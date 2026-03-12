#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
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


#ifdef _WIN32

// too lazy to work on compatibility rn since I'd need to rewrite my Makefile to allow for windows

#else

#include <dirent.h>
#include <string.h>
#include "request.h"

void generateDirectoryListing(char *folderPath, char *folderName, char **outStr, int outStrSize) {
    
    const char *startStr = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Directory Listing</title></head><body><h1>Listing for ";
    const char *endStr = "</ul></body></html>";
    
    printf("Hello!2 %s %zu %s %zu\n", startStr, strlen(startStr), folderName, strlen(endStr));
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
