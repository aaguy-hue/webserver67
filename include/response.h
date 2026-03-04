#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "startline.h"
#include "request.h"
// we include request.h since there's some common things
// such as the content maxlen

#define HTTP_RESPONSE_MAXLEN (1<<16)
#define HTTP_RESPONSE_MAXSIZE (1<<12)

typedef struct {
	char body[CONTENT_MAXLEN];
	StatusLine *statusLine;
	struct hashmap *headers;
} HttpResponse;

void createResponseText(HttpResponse *response, char *out);

void generateResponse(HttpResponse *response, HttpRequest *request);

#endif
