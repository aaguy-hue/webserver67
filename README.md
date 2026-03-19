# Webserver67

A simple webserver written in C. See `todo.txt` for immediate priorities for what needs to be done.

## Dependencies
- C Standard Library
- [LibCYAML v1.4.2](https://github.com/tlsa/libcyaml)
  - [LibYAML](https://pyyaml.org/wiki/LibYAML)
- [Zlib v1.3.2.1 (compression)](https://github.com/madler/zlib)
- Make
- GCC

## Current Capabilities
- Receives HTTP requests
  - Processes control data (i.e. first line)
  - Processes a limited set of request headers
- Serve static files over HTTP
  - Can serve many common filetypes used on the web
- Directory browsing
- Reads YAML config files

## Planned Capabilities
- Chunked transfer coding in responses for videos and other large files
  - NOTE: There are no plans for receiving chunked transfer coding from clients
- Process more types of request headers
- Reverse proxy
- Allow subdirectories to have their own index.html files

## Limitations
- Only supports HTTP/1.1
  - Things like processing control data become more annoying in later versions
- Only supports unix systems
  - Socket-related functions are in different headers on windows
  - Relies on unistd.h and time.h, unistd is unix-specific and time functions
    are different on windows
- Does not support chunked transfer coding
  - Assumes content is just one giant blob sent with the request

