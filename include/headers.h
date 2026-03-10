#ifndef HEADERS_H
#define HEADERS_H

#include <stdbool.h>

typedef enum {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
} StatusCode;

bool statusCodeIsOK(unsigned int code);

// typedef enum {
//     Host,
//     ContentLength,
//     ContentType,
//     Connection,
//     UserAgent,
//     Accept,
//     Cookie,
//     Referer,
//     Authorization,
//     UpgradeInsecureRequests
// } Headers;


#endif