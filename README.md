# Anthracite
A simple web server written in C++

## Module-Based Backends
Anthracite includes (read: will include) a system for allowing different "backend modules" to handle requests.
This allows for anthracite to be extended for additional use-cases. For example, the following
backends could be implemented: 

- File: Return files from a directory 
- Reverse Proxy: Pass the request to another server 
- Web Framework: Pass the request into an application built on your favorite web framework 

## Building & Debugging

Once you have the repository cloned, you can run the following command to build Anthracite:

```shell
make build
```

It will create a binary file named `./anthracite`

To save time, you can use the following command to build and run anthracite on port `8080`:

```shell
make run
```

To save time again, you can use the following command to build and debug anthracite in gdb:

```shell
make debug
```

## Usage

Run the following commands to serve all files located in `./www/`:

```shell
./anthracite [PORT_NUMBER]
```

## Todo
- [x] Serve HTML Pages
- [x] Properly parse HTTP requests 
- [x] Add module-based backend system for handling requests
- [ ] Cleanup (this one will never truly be done) 
- [ ] Proper error handling
- [ ] Flesh out module-based backend system for handling requests
- [ ] Fix glaring security issues 
- [ ] Faster parsing 
- [ ] Multithreading 
- [ ] Speed optimizations such as keepint the most visited html pages in memory 
- [ ] Cleanup codebase 
- [ ] Enable cache support 
- [ ] Support newer HTTP versions 

## Screenshots

![A picture of the default index.html page used by Anthracite](https://github.com/nickorlow/anthracite/blob/main/.screenshots/default-page.png?raw=true)
![A picture of the Anthracite default 404 not found page](https://github.com/nickorlow/anthracite/blob/main/.screenshots/404-page.png?raw=true)

---

_"By industry, we thrive"_
