# Anthracite

Anthracite is an extensible, low-dependency, fast web server.

## Developing

To build/develop Anthracite, you must have C++23, CMake, Make, and Python3 installed.

Create a `build/` directory, run `cmake ..`, and then `make` to build.

## Todo
- [x] HTTP/1.0
- [x] Serve HTML Pages
- [x] Properly parse HTTP requests 
- [x] Add module-based backend system for handling requests
- [x] Multithreading 
- [x] HTTP/1.1
- [x] Enhance logging
- [x] Create library that can be used to implement custom backends (i.e. webapi, fileserver, etc) 
- [x] Faster parsing 
- [ ] HTTP/2 
- [ ] Improve benchmarking infrastructure
- [ ] Fix glaring security issues 
- [ ] Proper error handling
- [ ] User configuration
- [ ] Cleanup (this one will never truly be done) 

## Screenshots

![A picture of the default index.html page used by Anthracite](https://github.com/nickorlow/anthracite/blob/main/.screenshots/default-page.png?raw=true)
![A picture of the Anthracite default 404 not found page](https://github.com/nickorlow/anthracite/blob/main/.screenshots/404-page.png?raw=true)

---

_"By industry, we thrive"_
