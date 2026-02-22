#include <string.h>
#include <stdio.h>
#include "control_data.h"
#include "util.h"

// I have no plans for windows compat but this still bothers me, why does
// windows exist :(((( I can't complain though since I'm a windows user
#if defined(_WIN32) || defined(_WIN64)
#define strtok_r strtok_s
#endif

RequestMethod getMethodFromStr(char* methodstr)
{
	if (strcmp(methodstr, "GET") == 0)     return GET;
	if (strcmp(methodstr, "POST") == 0)    return POST;
	if (strcmp(methodstr, "PUT") == 0)     return PUT;
	if (strcmp(methodstr, "PATCH") == 0)   return PATCH;
	if (strcmp(methodstr, "DELETE") == 0)  return DELETE;
	if (strcmp(methodstr, "HEAD") == 0)    return HEAD;
	if (strcmp(methodstr, "OPTIONS") == 0) return OPTIONS;

	printf("[-] Invalid HTTP method: '%s'\n", methodstr);
	return INVALID_METHOD;
}

ProtocolVersion getVersionFromStr(char* versionstr)
{
	if (strcmp(versionstr, "HTTP/0.9") == 0) return HTTP09;
	if (strcmp(versionstr, "HTTP/1.0") == 0) return HTTP10;
	if (strcmp(versionstr, "HTTP/1.1") == 0) return HTTP11;

	printf("[-] Invalid HTTP version: '%s'\n", versionstr);
	return INVALID_VERSION;
}

// Processes the first line of control data from the request
// Returns early if there's an issue in processing
ControlData getControlData(char** buf)
{
	size_t len = strcspn(*buf, "\n");
	(*buf)[len] = '\0';

	char* line = *buf;

	char *token = strtok_r(line, " ", &line);

	ControlData retval = {0};
	retval.method = getMethodFromStr(token);
	if (retval.method == INVALID_METHOD) return retval;

	token = strtok_r(line, " ", &line);
	strncpy(retval.target, token, REQUEST_TARGET_MAXLEN-1);
	retval.target[REQUEST_TARGET_MAXLEN-1] = '\0';

	token = strtok_r(line, " \r\n", &line);
	trim(token); // do this to remove \r if windows clients
	retval.version = getVersionFromStr(token);
	if (retval.version == INVALID_VERSION) return retval;

	(*buf) += len+1;
	return retval;
}


