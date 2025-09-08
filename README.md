# cframe

### A Minimal HTTP Server Framework for Oracle ARM-Based Linux Servers

> _"If the lack of namespacing or cross-platform support bothers you, please, by all means, f*** off :)"_  
> – Author's note

## Overview

`cframe.h` is a lightweight, header-only HTTP server framework written in C, designed for personal projects running on ARM-based Oracle Linux servers. Its minimal design avoids external dependencies apart from POSIX/Linux system headers and a custom string library (`zen-string.h`).

This library is not intended to be cross-platform or general-purpose. It's purpose-built for speed, simplicity, and ease of use in low-resource environments or personal server deployments.

This library is licensed under the `MIT` License.

---

## Features

- Single-header, zero-dependency (besides system and `zen-string.h`)
- Event-driven using `epoll`
- URI routing system with dynamic/static support
- Minimal MIME type handling
- Customizable 404 handling
- Raw byte response support
- Explicit and simple type definitions

---


Examples are available in the `examples` directory. 
