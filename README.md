# Anthracite

Anthracite is an extensible, low-dependency, fast web server.

## Developing

To build/develop Anthracite, you must have C++20, OpenSSL, CMake, Make, and Python3 installed.

Create a `build/` directory, run `cmake ..`, and then `make` to build.

## Features 

- HTTP/1.0 & HTTP/1.1 Support
- SSL via OpenSSL
- Event loop thread management
- libanthracite library for reating custom webservers 
- Configuration through configuration file
- Minimal dependencies (only OpenSSL & stantart library so far)

## Roadmap
- HTTP/2
- HTTP/3
- More threading modes
- Proxy backend
- Security/Error handling audit

## Screenshots

![A picture of the default index.html page used by Anthracite](https://github.com/nickorlow/anthracite/blob/main/.screenshots/default-page.png?raw=true)
![A picture of the Anthracite default 404 not found page](https://github.com/nickorlow/anthracite/blob/main/.screenshots/404-page.png?raw=true)

---

_"By industry, we thrive"_
