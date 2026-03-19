#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"
#include "hashmap.h"

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
        // request->headers = NULL;
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

    request->setRequestLine = &setRequestLine;
    request->setContent = &setContent;
    request->reset = &resetRequest;
    request->free = &freeRequest;

    return request;
}
