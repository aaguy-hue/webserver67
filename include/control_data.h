#ifndef CONTROL_DATA_H

#define CONTROL_DATA_H

#define REQUEST_TARGET_MAXLEN 200

typedef enum {
	INVALID_METHOD,
	GET,
	POST,
	PUT,
	PATCH,
	DELETE,
	HEAD,
	OPTIONS
} RequestMethod;

typedef enum {
	INVALID_VERSION,
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
