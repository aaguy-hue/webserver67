#include <stdio.h>
#include <string.h>

#include "response.h"
#include "startline.h"


// IMPORTANT NOTE: \r\n is the line ending expected by HTTP
#define STATUS_LINE_MAXLEN 8000

// TODO: do the header lines
void createHeaderLines(char **buf, struct hashmap *headers)
{
		(void)headers;
		*buf = "\r\n";
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

		char *headerLines;
		createHeaderLines(&headerLines, NULL);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
		snprintf(responseBuf, HTTP_RESPONSE_MAXLEN-1, "%s\r\n%s\r\n%s",
				statusLineBuf, headerLines, response->body);
#pragma GCC diagnostic pop

		// TODO: replace wth strncpy
		strcpy(out, responseBuf);
}

void generateResponse(HttpResponse *response, HttpRequest *request) {
		(void)request;

		response->statusLine->statusCode = 200;
		response->statusLine->version = HTTP11;

		char *reasonPhrase = "heya!";
		strncpy(response->statusLine->reasonPhrase, reasonPhrase, REASON_PHRASE_MAXLEN-1);

		char *resBody = "{'error': 'you are bad'}";
		strncpy(response->body, resBody, CONTENT_MAXLEN-1);

		// headers later
}
