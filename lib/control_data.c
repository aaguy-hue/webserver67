#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "control_data.h"

// I have no plans for windows compat but this still bothers me
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
	exit(EXIT_FAILURE);
}

ProtocolVersion getVersionFromStr(char* versionstr)
{
	if (strcmp(versionstr, "HTTP/0.9") == 0) return HTTP09;
	if (strcmp(versionstr, "HTTP/1.0") == 0) return HTTP10;
	if (strcmp(versionstr, "HTTP/1.1") == 0) return HTTP11;

	printf("[-] Invalid HTTP version: '%s'\n", versionstr);
	exit(EXIT_FAILURE);
}

ControlData getControlData(char** buf)
{
	size_t len = strcspn(*buf, "\n");
	(*buf)[len] = '\0';

	char* line = *buf;

	//char *saveptr;
	char *token = strtok_r(line, " ", &line);

	ControlData retval = {0};
	retval.method = getMethodFromStr(token);

	token = strtok_r(line, " ", &line);
	strncpy(retval.target, token, REQUEST_TARGET_MAXLEN-1);
	retval.target[REQUEST_TARGET_MAXLEN-1] = '\0';

	token = strtok_r(line, "\n", &line);
	retval.version = getVersionFromStr(token);

	(*buf) += len+1;
	return retval;
}


