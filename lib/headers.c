#include <stdbool.h>
#include "headers.h"

inline bool statusCodeIsOK(unsigned int code) {
    return code >= 200 && code < 300;
}