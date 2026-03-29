#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"
#include "fields.h"
#include "hashmap.h"
#include "util.h"

void setRequestLine(HttpRequest *request, RequestLine requestLine) {
    if (request->requestLine == NULL) {
        request->requestLine = malloc(sizeof(RequestLine));
        if (request->requestLine == NULL) {
            printf("[-] Failed to allocate memory for HTTP request line");
            free(request->content);
            free(request);
            exit(EXIT_FAILURE);
        }
    }

    *(request->requestLine) = requestLine;
}

void setContent(HttpRequest *request, const char *content, unsigned int contentSize) {
    // note: we add 1 to add space for the null byte
    if (contentSize >= request->contentSize) {
        request->content = realloc(request->content, (contentSize + 1) * sizeof(char));
        if (request->content == NULL) {
            printf("[-] Failed to reallocate memory for HTTP request content");
            request->free(request);
            exit(EXIT_FAILURE);
        }
        request->contentSize = contentSize + 1;
    }

    strncpy(request->content, content, contentSize);
    request->content[contentSize] = '\0';
}

void resetRequest(struct HttpRequest *request) {
    if (request->headers != NULL) {
        hashmap_free(request->headers);
        request->headers = NULL;
    }
    if (request->requestLine != NULL) {
        memset(request->requestLine, 0, sizeof(RequestLine));
    }
    if (request->content != NULL) {
        memset(request->content, 0, request->contentSize);
    }


}

void freeRequest(HttpRequest *request) {
    if (request->headers != NULL) {
        hashmap_free(request->headers);
    }
    if (request->requestLine != NULL) {
        free(request->requestLine);
    }
    if (request->content != NULL) {
        free(request->content);
    }
    free(request);
}


HttpRequest *initializeRequest() {
    HttpRequest *request = malloc(sizeof(HttpRequest));
    if (request == NULL) {
        printf("[-] Failed to allocate memory for HTTP request");
        exit(EXIT_FAILURE);
    }

    request->requestLine = NULL;
    request->headers = NULL;
    request->content = calloc(CONTENT_MINLEN, sizeof(char));
    request->contentSize = CONTENT_MINLEN;
    if (request->content == NULL) {
        printf("[-] Failed to allocate memory for HTTP request content");
        free(request->requestLine);
        free(request);
        exit(EXIT_FAILURE);
    }
    
    request->reset = &resetRequest;
    request->free = &freeRequest;

    return request;
}

void resetRequestBuilder(struct HttpRequestBuilder *builder) {
    builder->isRequestLineSet = false;
    builder->areHeadersSet = false;
    builder->isContentSet = false;
    builder->remainingBuf = NULL;
    builder->request->reset(builder->request);
}

struct HttpRequestBuilder *initializeRequestBuilder(HttpRequest *request) {
    struct HttpRequestBuilder *builder = malloc(sizeof(struct HttpRequestBuilder));
    if (builder == NULL) {
        printf("[-] Failed to allocate memory for HTTP request builder");
        exit(EXIT_FAILURE);
    }

    builder->request = request;
    builder->isRequestLineSet = false;
    builder->areHeadersSet = false;
    builder->isContentSet = false;
    builder->remainingBuf = NULL;
    builder->reset = &resetRequestBuilder;

    return builder;
}

void processRequestChunk(struct HttpRequestBuilder *requestBuilder, char *chunk, unsigned int chunkSize) {
    strncat(requestBuilder->remainingBuf ? requestBuilder->remainingBuf : "", chunk, chunkSize);
    requestBuilder->remainingBuf = NULL; // reset the buf

    if (!requestBuilder->isRequestLineSet) {
        // for simplicity, we assume the request line will always be in the first chunk, which should be the case for most requests since the request line is usually very small
        RequestLine requestLine = getRequestLine((char**)&chunk);
        if (requestLine.method == INVALID_METHOD || requestLine.version == INVALID_VERSION) {
            printf("[-] Invalid HTTP request line");
            requestBuilder->request->free(requestBuilder->request);
            exit(EXIT_FAILURE);
        }
        setRequestLine(requestBuilder->request, requestLine);
        requestBuilder->remainingBuf = chunk;
        requestBuilder->isRequestLineSet = true;
    } else if (!requestBuilder->areHeadersSet) {
        // note: the headers section always ends in a blank line (i.e., two consecutive newlines), so we can use that to determine when we've reached the end of the headers section
        // http uses crlf for newlines always, but advises to support for lf just in case
        // we add a \0 at the end of the two newlines to separate the headers section from the content section, so that we can easily parse the headers without worrying about the content, which we'll process in the next chunk
        // omg c is so bad at string manipulation so I have to write these yap paragraphs to explain this simple logic
        char *crlf = strstr(chunk, "\r\n\r\n");
        char *lf = strstr(chunk, "\n\n");
        if (crlf != NULL) {
            requestBuilder->remainingBuf = crlf + 4; // move to content
            *crlf = '\0'; // don't start reading content
            requestBuilder->areHeadersSet = true;
        } else if (lf != NULL) {
            requestBuilder->remainingBuf = lf + 2; // same as above
            *lf = '\0';
            requestBuilder->areHeadersSet = true;
        } else {
            strrchr(chunk, '\n')[0] = '\0';
        }

        if (requestBuilder->request->headers == NULL) {
            requestBuilder->request->headers = createFieldHashmap(20);
        }

        readHeaders(requestBuilder->request->headers, (char**)&chunk);

        // save the last part of the chunk that contains the start of the next line of headers, to be used when reading the next chunk
        // btw the reason I don't add strlen(chunk) is bc readHeaders moves the ptr as it reads the headers
        if (requestBuilder->remainingBuf == NULL) requestBuilder->remainingBuf = chunk + 1;
    } else if (!requestBuilder->isContentSet) {
        // first time we read content section
        const char *contentLengthStr = popHeader(requestBuilder->request->headers, "content-length");
		int contentLength = contentLengthStr && strIsNumeric(contentLengthStr) ? atoi(contentLengthStr) : 0;

        setContent(requestBuilder->request, chunk, contentLength);
        requestBuilder->isContentSet = true;
    } else {
        // this means we've already processed the request line and headers, so we just need to append the content
        strncat(requestBuilder->request->content, chunk, chunkSize);
    }
}

