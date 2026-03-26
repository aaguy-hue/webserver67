# Webserver67

A simple webserver written in C. See `todo.txt` for immediate priorities for what needs to be done.

## Usage

### Installation
1. Install the required dependencies (see **Dependencies** below).
2. Build and install:
    ```bash
    make
    make install
    ```
    You may need superuser privileges for `make install`.


### Running the Server

Basic command:
```bash
ws67 [options] <website_root>
```

Full form:
```bash
ws67 --port <port> --config <config_path> <website_root>
```

Short flags:
`-p` = `--port`
`-c` = `--config`


### Configuration Behavior

The server uses **exactly one configuration file**, selected in this order:
1. Path provided via `--config`
2. `./config.yml`
3. `/etc/webserver67/config.yml`

If none of these exist, the server runs with defaults.
> Note: Configuration files are not merged. Only one is loaded.

### Overrides
Command-line arguments override values from the selected config file:

`--port` overrides the configured port
`<website_root>` overrides the configured root directory

All command-line arguments are optional. If omitted, values are taken entirely from the config file.

### Examples
Run using default config resolution:
```bash
ws67
```

Specify a config file:
```bash
ws67 -c ./myconfig.yml
```

Override port and root directory:
```bash
ws67 -p 8080 ./public
```

Use a config file but override the root directory:
```bash
ws67 -c ./config.yml ./site
```

## Dependencies
All of these should present on most Linux distributions except LibCYAML.

- [LibCYAML v1.4.2](https://github.com/tlsa/libcyaml)
  - [LibYAML](https://pyyaml.org/wiki/LibYAML)
- [Zlib v1.3.2.1 (compression)](https://github.com/madler/zlib)
- C Standard Library
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

