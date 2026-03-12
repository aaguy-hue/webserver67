#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fields.h"
#include "response.h"
#include "startline.h"
#include "config.h"
#include "util.h"


// IMPORTANT NOTE: \r\n is the line ending expected by HTTP
#define STATUS_LINE_MAXLEN 8000

void createHeaderLines(char **buf, struct hashmap *headers)
{
	if (!headers) {
		strncat(*buf, "\r\n", 3);
		return;
	}

	size_t lineLen = FIELD_NAME_MAXLEN + FIELD_VALUE_MAXLEN + 5; // +5 for ": " and "\r\n" and null
    size_t iter = 0;
    void *item;
	while (hashmap_iter(headers, &iter, &item)) {
		const Field *f = item;
		printf("[+] Processing header: %s: %s\n", f->name, f->value);
        char lineBuf[lineLen];
        snprintf(lineBuf, lineLen, "%s: %s\r\n", f->name, f->value);
        strncat(*buf, lineBuf, lineLen);
    }
	strncat(*buf, "\r\n", 3);
	printf("[+] Finished processing headers. Final header lines buffer: \n%s\n", *buf);
}

void createResponseText(HttpResponse *response, char *out) {
		char responseBuf[HTTP_RESPONSE_MAXLEN];

    // create status line
		char statusLineBuf[STATUS_LINE_MAXLEN];
		StatusLine *statusLine = response->statusLine;
    char *versionStr = getStrFromVersion(statusLine->version);
		memset(statusLineBuf, 0, STATUS_LINE_MAXLEN);
    snprintf(statusLineBuf, STATUS_LINE_MAXLEN-1, "%s %u %s",
				versionStr, statusLine->statusCode, statusLine->reasonPhrase);
		statusLineBuf[STATUS_LINE_MAXLEN-1] = '\0';

		char headerLines[HTTP_RESPONSE_MAXLEN];
		memset(headerLines, 0, HTTP_RESPONSE_MAXLEN);
		char *headerLinesPtr = headerLines;
		createHeaderLines(&headerLinesPtr, response->headers);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
		snprintf(responseBuf, HTTP_RESPONSE_MAXLEN-1, "%s\r\n%s\r\n%s",
				statusLineBuf, headerLines, response->body);
#pragma GCC diagnostic pop

		// TODO: replace wth strncpy
		strcpy(out, responseBuf);

        printf("Finished creating response text!\n");
}

// TODO: this should be moved to request.c since it's more about processing the request than generating the response
static bool field_iter_processing(const void *item, void *fdata) {
	(void) fdata;
	const Field *f = item;

	if (strcmp(f->name, "content-length") == 0) {
		printf("[+] Content-Length header found with value: %s\n", f->value);
	} else if (strcmp(f->name, "content-type") == 0) {
		printf("[+] Content-Type header found with value: %s\n", f->value);
	}

	return true;
}

char *generateContentType(const char *target) {
    // printf("[+] Generating content type for target: %s\n", target);
    // this is really basic, ideally we would want to use something more robust like libmagic
    // this is mostly good enough though and covers the most common types
    // I don't plan on adding multipart data ever
    // See https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types for a reference list of MIME types
    if (fileExtensionMatches(target, ".html")) {
        return "text/html; charset=UTF-8";
    } else if (fileExtensionMatches(target, ".css")) {
        return "text/css";
    } else if (fileExtensionMatches(target, ".js")) {
        return "text/javascript";
    } else if (fileExtensionMatches(target, ".json")) {
        return "application/json";
    } else if (fileExtensionMatches(target, ".png")) {
        return "image/png";
    } else if (fileExtensionMatches(target, ".jpg") || fileExtensionMatches(target, ".jpeg")) {
        return "image/jpeg";
    } else if (fileExtensionMatches(target, ".svg")) {
        return "image/svg+xml";
    } else if (fileExtensionMatches(target, ".gif")) {
        return "image/gif";
    } else if (fileExtensionMatches(target, ".av1") || fileExtensionMatches(target, ".avif")) {
        return "image/avif";
    } else if (fileExtensionMatches(target, ".webp")) {
        return "image/webp";
    } else if (fileExtensionMatches(target, ".apng")) {
        return "image/apng";
    } else if (fileExtensionMatches(target, ".mp3")) {
        return "audio/mpeg";
    } else if (fileExtensionMatches(target, ".mp4")) {
        return "video/mp4";
    } else if (fileExtensionMatches(target, ".txt")) {
        return "text/plain";
    } else {
        return "application/octet-stream"; // default for unknown binary types
    }
}

struct hashmap *generateResponseHeaders(HttpRequest *request, HttpResponse *response) {
	(void)request; // todo: use request headers to generate response headers
	struct hashmap *headers = createFieldHashmap(10);

    char *contentType = generateContentType(response->fileName);
	Field contentTypeHeader = createField("Content-Type", contentType);
	hashmap_set(headers, &contentTypeHeader);

    char contentLengthStr[25];
    snprintf(contentLengthStr, 25, "%zu", response->bodyLen);
	Field contentLengthHeader = createField("Content-Length", contentLengthStr);
	hashmap_set(headers, &contentLengthHeader);

	return headers;
}

void generate404(char **buf, int max_size, const char *site_root) {
    char filePath[SITE_PATH_MAX + 1];
    snprintf(filePath, SITE_PATH_MAX, "%s/404.html", site_root);
    filePath[SITE_PATH_MAX] = '\0';


    FILE *f = fopen(filePath, "r");
    if (f == NULL) {
        strncpy(*buf, "<h1>404 Not Found</h1>", max_size-1);
        (*buf)[max_size-1] = '\0';
    } else {
        size_t bytesRead = fread(*buf, 1, max_size-1, f);
        (*buf)[bytesRead] = '\0';
        fclose(f);
    }
}

int loadFileFromSiteRoot(const char *site_root, HttpResponse *response, size_t outBufSize) {
    printf("[+] Loading file from site root. Site root: %s, target: %s\n", site_root, response->fileName);
    if (strcmp(response->fileName, "/") == 0) {
        strncpy(response->fileName, "/index.html", SITE_PATH_MAX);
        response->fileName[SITE_PATH_MAX] = '\0';
    }

    char filePath[SITE_PATH_MAX + 200];
    snprintf(filePath, SITE_PATH_MAX + 200, "%s/%s", site_root, response->fileName);
    filePath[SITE_PATH_MAX + 200] = '\0';

    FILE *f = fopen(filePath, "r");
    if (f == NULL) {
        // todo: add ability to add custom 404.html page
        fprintf(stderr, "[-] Failed to open file at path: %s\n", filePath);
        char *bodyPtr = response->body;
        generate404(&bodyPtr, outBufSize, site_root);
        strncpy(response->fileName, "/index.html", SITE_PATH_MAX);
        response->fileName[SITE_PATH_MAX] = '\0';
        return 404;
    }

    size_t bytesRead = fread(response->body, 1, outBufSize-1, f);
    response->body[bytesRead] = '\0';
    response->bodyLen = (size_t)bytesRead;

    fclose(f);
    return 200;
}

void generateResponse(HttpResponse *response, HttpRequest *request, char *site_root) {
    char *url = request->requestLine->target;
    strncpy(response->fileName, url, SITE_PATH_MAX-1);
    response->fileName[SITE_PATH_MAX-1] = '\0';

    response->statusLine->statusCode = loadFileFromSiteRoot(site_root, response, CONTENT_MAXLEN);
    response->statusLine->version = HTTP11;

    char *reasonPhrase = "";
    strncpy(response->statusLine->reasonPhrase, reasonPhrase, REASON_PHRASE_MAXLEN-1);

    // hashmap_get(request->headers, &(Field){.name="Content-Length"});

    hashmap_scan(request->headers, field_iter_processing, NULL);

    // headers later
    response->headers = generateResponseHeaders(request, response);
}
