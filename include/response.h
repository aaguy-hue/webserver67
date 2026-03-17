#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stddef.h>
#include "startline.h"
#include "request.h"
#include "config.h"
// we include request.h since there's some common things
// such as the content maxlen

#define HTTP_RESPONSE_MAXLEN (1<<16)
#define HTTP_RESPONSE_MAXSIZE (1<<12)

typedef enum {
	CONTENT_ENCODING_NONE,
	CONTENT_ENCODING_GZIP,
} ContentEncoding;

typedef struct {
	ContentEncoding encoding;
	size_t bodyLen;
	char body[CONTENT_MAXLEN];
	char fileName[SITE_PATH_MAX];
	StatusLine *statusLine;
	struct hashmap *headers;
} HttpResponse;

HttpResponse *initializeResponse();

void freeResponse(HttpResponse *response);

void sendStatusLine(HttpResponse *response, int clientfd);
void sendHeaders(HttpResponse *response, int clientfd);
void sendBody(HttpResponse *response, int clientfd);

void generateResponse(HttpResponse *response, HttpRequest *request, char *server_root, bool directory_browsing);

#endif
