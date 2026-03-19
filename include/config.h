#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <stdbool.h>

#define SITE_PATH_MAX 500

typedef struct ServerConfig {
  bool directory_browsing;
  unsigned short int port;
	char address[16];
  char site_root[SITE_PATH_MAX];

  char **compressed_file_types;
  unsigned compressed_file_types_count;
} ServerConfig;

ServerConfig *readConfig(char *filePath);

bool fileTypeShouldBeCompressed(ServerConfig *cfg, const char *fileName);

#endif
