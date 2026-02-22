#ifndef HTTP_REQUEST_H

#define HTTP_REQUEST_H

#define CONTENT_MAXLEN 1048576

typedef struct {
	char content[CONTENT_MAXLEN];
	ControlData *controlData;
	struct hashmap *headers;
} HttpRequest;

#endif
