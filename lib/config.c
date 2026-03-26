#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <string.h>
#include <cyaml/cyaml.h>
#include "util.h"
#include "config.h"
#include "fileutil.h"

struct cmdArgs {
	char *configFilePath;
	unsigned short int port;
	char *siteRoot;
};

//char **matchKeyToPtr(char *key, struct _cfg_tmp *cfg) {
//	trim(key);
//	if (strcmp(key, "port") == 0) {
//		return &(cfg->port);
//	}
//	return NULL;
//}

static const cyaml_schema_value_t compressed_file_type_entry = {
	CYAML_VALUE_STRING(CYAML_FLAG_POINTER, char, 0, 5),
};

// mapping at the top level
static const cyaml_schema_field_t top_mapping_schema[] = {
	CYAML_FIELD_UINT(
			"port", CYAML_FLAG_DEFAULT,
			ServerConfig, port
	),
	CYAML_FIELD_STRING(
			"address", CYAML_FLAG_DEFAULT,
			ServerConfig, address, 7
	),
	CYAML_FIELD_STRING(
			"site_root", CYAML_FLAG_DEFAULT,
			ServerConfig, site_root, 1
	),
	CYAML_FIELD_BOOL(
		"directory_browsing", CYAML_FLAG_OPTIONAL,
		ServerConfig, directory_browsing
	),
	CYAML_FIELD_SEQUENCE(
			"compressed_file_types", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL,
			ServerConfig, compressed_file_types,
			&compressed_file_type_entry, 0, CYAML_UNLIMITED
	),
	CYAML_FIELD_END
};

// value schema for top level
static const cyaml_schema_value_t file_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			ServerConfig, top_mapping_schema
	)
};

static const cyaml_config_t parser_config = {
	.log_fn = cyaml_log, // default logging func
	.mem_fn = cyaml_mem, // default memory allocator, though we don't allocate
						 // anything bc we pass in the pointerto readConfig
	.log_level = CYAML_LOG_INFO
};

static char *searchForConfigFile() {
	const char *possiblePaths[] = {
		"./config.yml",
		"/etc/webserver67/config.yml",
		NULL
	};

	for (int i = 0; possiblePaths[i] != NULL; i++) {
		if (fileExists(possiblePaths[i])) {
			printf("[+] Found config file at path: %s\n", possiblePaths[i]);
			return strdup(possiblePaths[i]);
		}
	}

	fprintf(stderr, "[-] Failed to find config file in any of the following paths:\n");
	for (int i = 0; possiblePaths[i] != NULL; i++) {
		fprintf(stderr, "    %s\n", possiblePaths[i]);
	}
	return NULL;
}

static struct cmdArgs parseCmdArgs(int argc, char *argv[]) {
	struct cmdArgs args = {0};

	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "--config") == 0 || strcmp(argv[i], "-c") == 0) && i + 1 < argc) {
			args.configFilePath = strdup(argv[i + 1]);
			argv[i] = NULL;
			argv[i + 1] = NULL;
			i++;
		} else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
			if (strIsNumeric(argv[i + 1])) {
				args.port = (unsigned short int)atoi(argv[i + 1]);
			} else {
				fprintf(stderr, "[-] Invalid port number provided in command line argument: %s\n", argv[i + 1]);
			}
			argv[i] = NULL;
			argv[i + 1] = NULL;
			i++;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "[-] Unrecognized command line argument: %s\n", argv[i]);
		}
	}

	// Last positional argument is the site root
	if (argc > 1 && argv[argc - 1] != NULL && argv[argc - 1][0] != '-') {
		args.siteRoot = argv[argc - 1];
		argv[argc - 1] = NULL;
	}

	return args;
}

// Will read configuration from a specific file
static ServerConfig *readConfigFile(char *filePath) {
	cyaml_err_t err;

	// annoyingly, cyaml will allocate the thing for you
	// you can't just allocate it on your own which is dumb
	ServerConfig *config;
	err = cyaml_load_file(
			filePath, &parser_config,
			&file_schema, (cyaml_data_t **)&config, NULL
	);
	if (err != CYAML_OK) {
		fprintf(stderr, "[-] Failed to parse config: %s\n", cyaml_strerror(err));
		return NULL;
	}

	// todo: I may or may not have to call cyaml_free()}
	return config;
}

bool fileTypeShouldBeCompressed(ServerConfig *cfg, const char *fileName) {
	if (cfg->compressed_file_types == NULL || cfg->compressed_file_types_count == 0) {
		return false;
	}

	for (unsigned i = 0; i < cfg->compressed_file_types_count; i++) {
		const char *ext = cfg->compressed_file_types[i];
		size_t extLen = strlen(ext);
		size_t fileNameLen = strlen(fileName);
		if (fileNameLen >= extLen && strcmp(fileName + fileNameLen - extLen, ext) == 0) {
			return true;
		}
	}
	return false;
}


// Eventually we will read configuration in the following order of precedence:
// 1. Command line arguments
// 2. config.yml in the current working directory
// 3. /etc/webserver67/config.yml
// For now we just do the highest priority file and then cmd line args
ServerConfig *readConfig(int argc, char *argv[]) {
	struct cmdArgs args = parseCmdArgs(argc, argv);

	if (args.configFilePath == NULL) {
		args.configFilePath = searchForConfigFile();
		if (args.configFilePath == NULL) {
			return NULL;
		}
	} else {
		printf("[+] Using config file from command line argument: %s\n", args.configFilePath);
	}
	
	ServerConfig *cfg = readConfigFile(args.configFilePath);
	if (cfg == NULL) {
		return NULL;
	}

	if (args.port != 0) {
		cfg->port = args.port;
	}
	if (args.siteRoot != NULL) {
		strncpy(cfg->site_root, args.siteRoot, SITE_PATH_MAX - 1);
		cfg->site_root[SITE_PATH_MAX - 1] = '\0'; // ensure null termination
	}
	if (!fileExists(cfg->site_root)) { // fileExists works for dirs as well since it just does stat
		fprintf(stderr, "[-] Site root directory specified in config does not exist: %s\n", cfg->site_root);
		free(cfg);
		return NULL;
	}

	return cfg;
}