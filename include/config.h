#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <stdbool.h>

#define SITE_PATH_MAX 250

typedef struct ServerConfig {
  unsigned short int port;
	char address[16];
  char site_root[SITE_PATH_MAX];
} ServerConfig;

ServerConfig *readConfig(char *filePath);

#endif
