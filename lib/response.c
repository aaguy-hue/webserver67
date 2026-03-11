#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fields.h"
#include "response.h"
#include "startline.h"
#include "config.h"


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
	printf("[+] Finished processing headers. Final header lines buffer: %s\n", *buf);
	strncat(*buf, "\r\n", 3);
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

struct hashmap *generateResponseHeaders(HttpRequest *request) {
	(void)request; // todo: use request headers to generate response headers
	struct hashmap *headers = createFieldHashmap(10);

	Field contentTypeHeader = createField("Content-Type", "text/html; charset=utf-8");
	hashmap_set(headers, &contentTypeHeader);

	Field contentSizeHeader = createField("Content-Size", "67");
	hashmap_set(headers, &contentSizeHeader);

	return headers;
}

int loadFileFromSiteRoot(const char *site_root, const char *target, char *outBuf, size_t outBufSize) {
    printf("[+] Loading file from site root. Site root: %s, target: %s\n", site_root, target);
    if (strcmp(target, "/") == 0) {
        target = "/index.html";
    }

    char filePath[SITE_PATH_MAX + 1];
    snprintf(filePath, SITE_PATH_MAX, "%s/%s", site_root, target);
    filePath[SITE_PATH_MAX] = '\0';

    FILE *f = fopen(filePath, "r");
    if (f == NULL) {
        // todo: add ability to add custom 404.html page
        fprintf(stderr, "[-] Failed to open file at path: %s\n", filePath);
        strncpy(outBuf, "<h1>404 Not Found</h1>", outBufSize-1);
        outBuf[outBufSize-1] = '\0';
        return 404;
    }

    size_t bytesRead = fread(outBuf, 1, outBufSize-1, f);
    outBuf[bytesRead] = '\0';

    fclose(f);
    return 200;
}

void generateResponse(HttpResponse *response, HttpRequest *request, char *site_root) {
    char *url = request->requestLine->target;
    response->statusLine->statusCode = loadFileFromSiteRoot(site_root, url, response->body, CONTENT_MAXLEN);

    response->statusLine->version = HTTP11;

    char *reasonPhrase = "heya!";
    strncpy(response->statusLine->reasonPhrase, reasonPhrase, REASON_PHRASE_MAXLEN-1);

    // hashmap_get(request->headers, &(Field){.name="Content-Length"});

    hashmap_scan(request->headers, field_iter_processing, NULL);

    // headers later
    response->headers = generateResponseHeaders(request);
}
