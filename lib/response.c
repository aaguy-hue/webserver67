#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "fields.h"
#include "fileutil.h"
#include "response.h"
#include "startline.h"
#include "config.h"
#include "util.h"


// IMPORTANT NOTE: \r\n is the line ending expected by HTTP
#define STATUS_LINE_MAXLEN 8000
#define HTTP_HEADER_LINES_MAXLEN (1<<12)

HttpResponse *initializeResponse() {
    HttpResponse *response = malloc(sizeof(HttpResponse));
    if (response == NULL) {
        printf("[-] Failed to allocate memory for HTTP response");
        exit(EXIT_FAILURE);
    }

    response->encoding = CONTENT_ENCODING_NONE;
    response->statusLine = malloc(sizeof(StatusLine));
	if (response->statusLine == NULL) {
		printf("[-] Failed to allocate memory for HTTP response status line");
		free(response);
		exit(EXIT_FAILURE);
	}

    memset(response->specialBody, 0, CONTENT_MAXLEN);
    memset(response->fileName, 0, SITE_PATH_MAX);
    return response;
}

void freeResponse(HttpResponse *response) {
    if (response->headers != NULL) {
        hashmap_free(response->headers);
    }
    if (response->statusLine != NULL) {
        free(response->statusLine);
    }
    free(response);
}

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

void sendResponse(const void *buffer, size_t bufferSize, int clientfd) {
    ssize_t total = 0;
    long int bufferLen = (long int)bufferSize;
    while (total < bufferLen) {
        ssize_t sent = send(clientfd, (const uint8_t *)buffer + total, bufferLen - total, 0);
        if (sent == -1) {
            perror("[-] Failed to send response!");
            break;
        } else if (sent == 0) {
            printf("[-] Connection closed by client while sending response\n");
            break; // connection closed
        }
        total += sent;
    }
    printf("Sent response!\n");
}

void sendStatusLine(HttpResponse *response, int clientfd) {
    char statusLineBuf[STATUS_LINE_MAXLEN];
    StatusLine *statusLine = response->statusLine;
    char *versionStr = getStrFromVersion(statusLine->version);
    memset(statusLineBuf, 0, STATUS_LINE_MAXLEN);
    snprintf(statusLineBuf, STATUS_LINE_MAXLEN-1, "%s %u %s\r\n",
             versionStr, statusLine->statusCode, statusLine->reasonPhrase);
    sendResponse(statusLineBuf, strlen(statusLineBuf), clientfd);
}

void sendHeaders(HttpResponse *response, int clientfd) {
    char headerLines[HTTP_HEADER_LINES_MAXLEN];
    memset(headerLines, 0, HTTP_HEADER_LINES_MAXLEN);
    char *headerLinesPtr = headerLines;
    createHeaderLines(&headerLinesPtr, response->headers);
    sendResponse(headerLines, strlen(headerLines), clientfd);
}

#define SEND_FILE_CHUNK_SIZE (1 << 14) // 16KB

void sendBody(HttpResponse *response, int clientfd) {
    if (response->specialBodyUsed) {
        sendResponse(response->specialBody, strlen(response->specialBody), clientfd);
        return;
    }

    FILE *f = fopen(response->pathToFile, "rb");
    if (f == NULL) { // shouldn't happen since we already check for file existence in generateResponse
        printf("[-] Failed to open file at path: %s. Very strange.\n", response->pathToFile);
        return;
    }

    size_t bytesRead;
    uint8_t buffer[SEND_FILE_CHUNK_SIZE];
    while ((bytesRead= fread(buffer, 1, SEND_FILE_CHUNK_SIZE, f)) > 0) {
        sendResponse(buffer, bytesRead, clientfd);
    }
    
    fclose(f);
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

size_t getBodyLength(HttpResponse *response) {
    if (response->specialBodyUsed) {
        return strlen(response->specialBody);
    } else {
        return fileSize(response->pathToFile);
    }
}

struct hashmap *generateResponseHeaders(HttpRequest *request, HttpResponse *response) {
	(void)request; // todo: use request headers to generate response headers
	struct hashmap *headers = createFieldHashmap(10);

    char *contentType = generateContentType(response->fileName);
	Field contentTypeHeader = createField("Content-Type", contentType);
	hashmap_set(headers, &contentTypeHeader);

    char contentLengthStr[25];
    snprintf(contentLengthStr, 25, "%zu", getBodyLength(response));
	Field contentLengthHeader = createField("Content-Length", contentLengthStr);
	hashmap_set(headers, &contentLengthHeader);

    if (response->encoding != CONTENT_ENCODING_NONE) {
        char contentEncodingStr[20];
        if (response->encoding == CONTENT_ENCODING_GZIP) {
            strncpy(contentEncodingStr, "gzip", 20);
        } else {
            printf("[-] Invalid content encoding in response object: %d\n", response->encoding);
            exit(EXIT_FAILURE);
        }

        Field contentEncodingHeader = createField("Content-Encoding", contentEncodingStr);
        hashmap_set(headers, &contentEncodingHeader);
    }

	return headers;
}

void generate404PageInMemory(char **buf, int max_size, const char *site_root) {
    char filePath[SITE_PATH_MAX + 1];
    snprintf(filePath, SITE_PATH_MAX, "%s/404.html", site_root);
    filePath[SITE_PATH_MAX] = '\0';


    FILE *f = fopen(filePath, "rb");
    if (f == NULL) {
        strncpy(*buf, "<h1>404 Not Found</h1>", max_size-1);
        (*buf)[max_size-1] = '\0';
    } else {
        size_t bytesRead = fread(*buf, 1, max_size-1, f);
        (*buf)[bytesRead] = '\0';
        fclose(f);
    }
}

void make404Response(HttpResponse *response, size_t outBufSize, const char *site_root) {
    fprintf(stderr, "[-] Failed to open file at path: %s\n", response->pathToFile);

    strncpy(response->pathToFile, "/404.html", SITE_PATH_MAX);
    response->pathToFile[SITE_PATH_MAX] = '\0';

    // If no 404 file, generate it in-memory
    if (!fileExists(response ->pathToFile)) {
        char *bodyPtr = response->specialBody;
        response->specialBodyUsed = true;
        generate404PageInMemory(&bodyPtr, outBufSize, site_root);
    }
}

int loadDirectoryBrowsing(HttpResponse *response, char *filePath) {
    char *bodyPtr = response->specialBody;
    response->specialBodyUsed = true;
    generateDirectoryListing(filePath, response->fileName, &bodyPtr, SPECIAL_BODY_MAXLEN);
    strncpy(response->fileName, "/directory.html", SITE_PATH_MAX); // ensure content type is text/html
    response->fileName[SITE_PATH_MAX] = '\0';
    return 200;
}

int loadFileFromSiteRoot(ServerConfig *cfg, HttpRequest *request, HttpResponse *response) {
    printf("[+] Loading file from site root. Site root: %s, target: %s\n", cfg->site_root, response->fileName);
    if (strcmp(response->fileName, "/") == 0) {
        strncpy(response->fileName, "/index.html", SITE_PATH_MAX);
        response->fileName[SITE_PATH_MAX] = '\0';
    }

    snprintf(response->pathToFile, SITE_PATH_MAX * 2, "%s%s", cfg->site_root, response->fileName);
    response->pathToFile[SITE_PATH_MAX + 200] = '\0';

    if (isDirectory(response->pathToFile)) {
        printf("[+] Target is a directory. File path: %s\n", response->pathToFile);
        if (!cfg->directory_browsing) {
            printf("[-] Directory browsing is disabled. Cannot load directory at path: %s\n", response->pathToFile);
            return 404;
        }
        printf("[+] Directory browsing is enabled. Generating directory listing for path: %s\n", response->pathToFile);
        return loadDirectoryBrowsing(response, response->pathToFile);
    }

    const char *acceptEncoding = getHeader(request->headers, "Accept-Encoding");
    char *filePathPtr = response->pathToFile;
    bool successfullyCompressed = compressFile(&filePathPtr, acceptEncoding);

    if (successfullyCompressed) {
        response->encoding = CONTENT_ENCODING_GZIP;
        printf("[+] Successfully compressed file: %s\n", response->pathToFile);
    } else {
        printf("[+] Serving uncompressed file: %s\n", response->pathToFile);
    }

    if (!fileExists(response->pathToFile)) {
        printf("[-] File does not exist at path: %s\n", response->pathToFile);
        return 404;
    }
    return 200;
}

void generateResponse(HttpResponse *response, HttpRequest *request, ServerConfig *cfg) {
    char *url = request->requestLine->target;
    strncpy(response->fileName, url, SITE_PATH_MAX-1);
    response->fileName[SITE_PATH_MAX-1] = '\0';

    response->statusLine->statusCode = loadFileFromSiteRoot(cfg, request, response);
    response->statusLine->version = HTTP11;

    if (response->statusLine->statusCode == 404) {
        make404Response(response, SPECIAL_BODY_MAXLEN, cfg->site_root);
    }

    char *reasonPhrase = "";
    strncpy(response->statusLine->reasonPhrase, reasonPhrase, REASON_PHRASE_MAXLEN-1);

    hashmap_scan(request->headers, field_iter_processing, NULL);

    response->headers = generateResponseHeaders(request, response);
}
