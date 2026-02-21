#ifndef CONTROL_DATA_H

#define CONTROL_DATA_H

#define REQUEST_TARGET_MAXLEN 200

typedef enum {
	GET,
	POST,
	PUT,
	PATCH,
	DELETE,
	HEAD,
	OPTIONS
} RequestMethod;

typedef enum {
	HTTP09,
	HTTP10,
	HTTP11
} ProtocolVersion;

typedef struct {
	char target[REQUEST_TARGET_MAXLEN];
	RequestMethod method;
	ProtocolVersion version;
} ControlData;

ControlData getControlData(char** buf);

#endif
