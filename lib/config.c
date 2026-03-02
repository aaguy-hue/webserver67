#include <stdio.h>
#include <stdbool.h>
//#include <yaml.h>
#include <string.h>
#include <cyaml/cyaml.h>
#include "util.h"
#include "config.h"

char **matchKeyToPtr(char *key, struct _cfg_tmp *cfg) {
	trim(key);
	if (strcmp(key, "port") == 0) {
		return &(cfg->port);
	}
	return NULL;
}

// mapping at the top level
static const cyaml_schema_field_t top_mapping_schema[] = {
	CYAML_FIELD_UINT(
			"port", CYAML_FLAG_DEFAULT,
			ServerConfig, port
	),
	CYAML_FIELD_END
};

// value schema for top level
static const cyaml_schema_value_t top_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			ServerConfig, top_mapping_schema
	)
};

static const cyaml_config_t parser_config = {
	.log_fn = cyaml_log, // default logging func
	.mem_fn = cyaml_mem, // default memory allocator
	.log_level = CYAML_LOG_INFO
};

bool readConfig(char *filePath, ServerConfig *config) {
	cyaml_err_t err;

	err = cyaml_load_file(
			filePath, &parser_config,
			&top_schema, (cyaml_data_t **)config, NULL
	);
	if (err != CYAML_OK) {
		fprintf(stderr, "[-] Failed to parse config: %s\n", cyaml_strerror(err));
		return false;
	}

	// todo: I may or may not have to call cyaml_free()}
	return true;
}

