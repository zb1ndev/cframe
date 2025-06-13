# cframe.h
A small header-only C library for small websites / applications


## Overview

**cframe.h** is a lightweight, header-only C library for building simple HTTP servers on Windows. It provides a straightforward API for defining routes and serving HTTP responses, making it easy to create static or dynamic web servers with minimal code. The library depends on [zstring.h](https://github.com/zb1ndev/zstring.h) (minimum version 1.3.1).

## Features
- Header-only, single-file include
- Simple route and responder model
- Handles multiple connections using `select()`
- Built-in 404 error response
- Easy to extend and integrate

## Requirements
- Windows (uses Winsock2)
- [zstring.h](https://github.com/zb1ndev/zstring.h) (minimum version 1.3.1)
- C99 compatible compiler

## Usage
1. Copy `cframe.h` and `zstring.h` into your project.
2. In **one** source file, define `#define CFRAME_IMPLEMENTATION` and `#define Z_STRING_IMPLEMENTATION` **before** including `cframe.h`:

```c
#define Z_STRING_IMPLEMENTATION
#define CFRAME_IMPLEMENTATION
#include "cframe.h"
```
3. Define your routes and responders, then start the server:

```c
HTTPResponse home(void* data) {

    HTTPHandlerContext context = *(HTTPHandlerContext*)data;
    String response = string_from_format("This server is running on %s:%d", context.server->address, context.server->port);

    return (HTTPResponse) {
        200, "OK", CFRAME_MIME_HTML, CFRAME_CLOSE,
        response.length, string_to_bytes(&response)
    };

}

int main(void) {

    HTTPServer server = new_http_server("127.0.0.1", 8080, 1, new_route("/", &home));
    start_http_server(&server);
    close_http_server(&server);

    return 0;

}
```

## API
- `Route new_route(const char* address, HTTPResponse (*responder)(void*));`
- `HTTPServer new_http_server(const char* address, int port, size_t n, ...);`
- `int start_http_server(HTTPServer* server);`
- `void close_http_server(HTTPServer* server);`

See the `examples/` directory for more usage.

## License
MIT License © 2025 Joel Zbinden
