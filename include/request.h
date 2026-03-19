#ifndef HTTP_REQUEST_H

#define HTTP_REQUEST_H

#define CONTENT_MINLEN (1 << 12)

#include "startline.h"

typedef struct HttpRequest {
	char *content;
	unsigned int contentSize;

	RequestLine *requestLine;
	struct hashmap *headers;

	void (*setRequestLine) (struct HttpRequest *request, RequestLine requestLine);
	void (*setContent) (struct HttpRequest *request, const char *content, unsigned int contentSize);
	void (*reset) (struct HttpRequest *request);
	void (*free) (struct HttpRequest *request);
} HttpRequest;

HttpRequest *initializeRequest();

#endif
