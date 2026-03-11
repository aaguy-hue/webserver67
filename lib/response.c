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

void loadFileFromSiteRoot(const char *siteRoot, const char *target, char *outBuf, size_t outBufSize) {
    char filePath[SITE_PATH_MAX + 1];
    snprintf(filePath, SITE_PATH_MAX, "%s/%s", siteRoot, target);
    filePath[SITE_PATH_MAX] = '\0';

    FILE *f = fopen(filePath, "r");
    if (f == NULL) {
        fprintf(stderr, "[-] Failed to open file at path: %s\n", filePath);
        strncpy(outBuf, "<h1>404 Not Found</h1>", outBufSize-1);
        outBuf[outBufSize-1] = '\0';
        return;
    }

    size_t bytesRead = fread(outBuf, 1, outBufSize-1, f);
    outBuf[bytesRead] = '\0';

    fclose(f);
}

void generateResponse(HttpResponse *response, HttpRequest *request) {
		(void)request;

		response->statusLine->statusCode = 200;
		response->statusLine->version = HTTP11;

		char *reasonPhrase = "heya!";
		strncpy(response->statusLine->reasonPhrase, reasonPhrase, REASON_PHRASE_MAXLEN-1);

		char *resBody = "<!DOCTYPE html> \
<html> \
<head> \
<meta charset=\"UTF-8\"> \
<title>A Small Happy Page</title> \
 \
<style> \
body { \
    margin: 0; \
    font-family: Arial, sans-serif; \
    background: linear-gradient(135deg, #ffd6e7, #d6f0ff); \
    display: flex; \
    justify-content: center; \
    align-items: center; \
    height: 100vh; \
    text-align: center; \
    overflow: hidden; \
} \
 \
.card { \
    background: white; \
    padding: 40px; \
    border-radius: 20px; \
    box-shadow: 0 10px 30px rgba(0,0,0,0.15); \
    max-width: 400px; \
} \
 \
h1 { \
    margin-top: 0; \
} \
 \
button { \
    padding: 12px 20px; \
    font-size: 16px; \
    border: none; \
    border-radius: 10px; \
    background: #ff6fa8; \
    color: white; \
    cursor: pointer; \
    transition: transform 0.15s; \
} \
 \
button:hover { \
    transform: scale(1.05); \
} \
 \
.heart { \
    position: absolute; \
    font-size: 20px; \
    animation: floatUp 3s linear forwards; \
} \
 \
@keyframes floatUp { \
    from { \
        transform: translateY(0); \
        opacity: 1; \
    } \
    to { \
        transform: translateY(-200px); \
        opacity: 0; \
    } \
} \
 \
.confetti { \
    position: absolute; \
    width: 8px; \
    height: 8px; \
    animation: fall 3s linear forwards; \
} \
 \
@keyframes fall { \
    from { \
        transform: translateY(-50px) rotate(0deg); \
    } \
    to { \
        transform: translateY(100vh) rotate(720deg); \
    } \
} \
</style> \
</head> \
 \
<body> \
 \
<div class=\"card\"> \
<h1>Hey there 👋</h1> \
<p>This tiny website exists for one reason:</p> \
<p><b>To make you smile today.</b></p> \
 \
<button onclick=\"celebrate()\">Click for happiness</button> \
 \
<p id=\"message\" style=\"margin-top:20px;\"></p> \
</div> \
 \
<script> \
 \
const messages = [ \
'You matter more than you think.', \
'Someone is glad you\\'re around.', \
'Today might be random, but you\\'re not.', \
'The world is slightly better because you\\'re in it.', \
'I hope something good happens to you today.' \
]; \
 \
function celebrate() { \
 \
document.getElementById(\"message\").innerText = \
messages[Math.floor(Math.random()*messages.length)]; \
 \
for(let i=0;i<20;i++){ \
    createHeart(); \
} \
 \
for(let i=0;i<60;i++){ \
    createConfetti(); \
} \
 \
} \
 \
function createHeart(){ \
    const heart = document.createElement(\"div\"); \
    heart.className = \"heart\"; \
    heart.innerText = \"💖\"; \
 \
    heart.style.left = Math.random()*window.innerWidth + \" px\"; \
    heart.style.bottom = \"0px\"; \
 \
    document.body.appendChild(heart); \
 \
    setTimeout(()=>heart.remove(),3000); \
} \
 \
function createConfetti(){ \
    const conf = document.createElement(\"div\"); \
    conf.className = \"confetti\"; \
 \
    const colors = [\"#ff6fa8\",\"#6fd3ff\",\"#ffd36f\",\"#8aff8a\",\"#c48aff\"]; \
    conf.style.background = colors[Math.floor(Math.random()*colors.length)]; \
 \
    conf.style.left = Math.random()*window.innerWidth + \"px\"; \
 \
    document.body.appendChild(conf); \
 \
    setTimeout(()=>conf.remove(),3000); \
} \
 \
</script> \
 \
</body> \
</html>";
		strncpy(response->body, resBody, CONTENT_MAXLEN-1);

		// hashmap_get(request->headers, &(Field){.name="Content-Length"});

		hashmap_scan(request->headers, field_iter_processing, NULL);

		// headers later
		response->headers = generateResponseHeaders(request);
}
