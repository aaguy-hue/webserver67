#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <stdbool.h>

#define SITE_PATH_MAX 250

typedef struct ServerConfig {
  unsigned short int port;
	char address[16];
  char site_root[SITE_PATH_MAX];
} ServerConfig;

// should never be used in other files
// I don't want to put this in config.c though since it's harder to ensure
// that it matches ServerConfig
//struct _cfg_tmp {
//	char* port;
//};

ServerConfig *readConfig(char *filePath);

#endif
