#ifndef HTTP_REQUEST_H

#define HTTP_REQUEST_H

#define CONTENT_MINLEN (1 << 12)

#include <stdbool.h>
#include "startline.h"

typedef struct HttpRequest {
	char *content;
	unsigned int contentSize;
	
	RequestLine *requestLine;
	struct hashmap *headers;
	
	void (*reset) (struct HttpRequest *request);
	void (*free) (struct HttpRequest *request);
} HttpRequest;

typedef struct HttpRequestBuilder {
	HttpRequest *request;

	bool isRequestLineSet;
	bool areHeadersSet;
	bool isContentSet;

	char *remainingBuf; // this is used to store the remaining buffer after parsing the request line and headers, so that it can be passed to the response generator for processing

	void (*reset)(struct HttpRequestBuilder *builder);
} HttpRequestBuilder;

HttpRequest *initializeRequest();
struct HttpRequestBuilder *initializeRequestBuilder(HttpRequest *request);

bool processRequestChunk(struct HttpRequestBuilder *requestBuilder, char *chunk, unsigned int chunkSize);

#endif
