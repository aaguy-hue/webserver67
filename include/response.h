#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stddef.h>
#include "startline.h"
#include "request.h"
#include "config.h"
// we include request.h since there's some common things
// such as the content maxlen

typedef enum {
	CONTENT_ENCODING_NONE,
	CONTENT_ENCODING_GZIP,
} ContentEncoding;

typedef struct {
	char fileName[SITE_PATH_MAX];
	char pathToFile[SITE_PATH_MAX+200];

	// This is only used for special cases where we generate the body in memory instead of reading from
	// a file, such as for directory browsing responses and 404 responses, since in those cases we 
	// don't have a file to read from and need to generate the body content in memory instead. 
	// For normal file responses this is unused and bodyLen is 0, and for directory browsing and 
	// 404 responses this is where we store the generated body content and bodyLen is set to the 
	// length of that content. specialBody should *not* be containing binary data since it's only 
	// used for directory browsing and 404 responses, which are always text/html, so we can safely 
	// treat it as a null-terminated string when generating the response.
	char specialBody[CONTENT_MAXLEN];
	bool specialBodyUsed;

	ContentEncoding encoding;
	StatusLine *statusLine;
	struct hashmap *headers;
} HttpResponse;

HttpResponse *initializeResponse();

void freeResponse(HttpResponse *response);

void sendStatusLine(HttpResponse *response, int clientfd);
void sendHeaders(HttpResponse *response, int clientfd);
void sendBody(HttpResponse *response, int clientfd);

void generateResponse(HttpResponse *response, HttpRequest *request, ServerConfig *cfg);

#endif
