#ifndef HTTP_REQUEST_H

#define HTTP_REQUEST_H

#define CONTENT_MAXLEN (1 << 12)

#include "startline.h"

typedef struct {
	char content[CONTENT_MAXLEN];
	RequestLine *requestLine;
	struct hashmap *headers;
} HttpRequest;

#endif
