# Webserver67

A simple webserver written in C. See `todo.txt` for immediate priorities for what needs to be done.

## Dependencies
- C Standard Library
- [LibCYAML v1.4.2](https://github.com/tlsa/libcyaml)
  - [LibYAML](https://pyyaml.org/wiki/LibYAML)
- Make
- GCC

## Current Capabilities
- Receives HTTP requests
- Processes control data (i.e. first line)
- Processes request headers
- Reads YAML config files
- Prepares a response

## Planned Capabilities
- Process more types of request headers
- Set correct Content-Type header in response
- Reverse proxy

## Limitations
- Only supports HTTP/1.1
  - Things like processing control data become more annoying in later versions
- Only supports unix systems
  - Socket-related functions are in different headers on windows
  - Relies on unistd.h and time.h, unistd is unix-specific and time functions
    are different on windows
- Does not support chunked transfer coding
  - Assumes content is just one giant blob sent with the request

