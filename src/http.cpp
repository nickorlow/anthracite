#include <arpa/inet.h>
#include <malloc.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket.cpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

constexpr int HTTP_HEADER_BYTES = 8190;

enum http_method {
    GET,
    POST,
    DELETE,
    PUT,
    PATCH,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE,
    COPY,
    LINK,
    UNLINK,
    PURGE,
    LOCK,
    UNLOCK,
    PROPFIND,
    VIEW,
    UNKNOWN
};

static unordered_map<string, http_method> const http_method_map = {
    { "GET", http_method::GET },
    { "POST", http_method::POST },
    { "DELETE", http_method::DELETE },
    { "PUT", http_method::PUT },
    { "PATCH", http_method::PATCH },
    { "HEAD", http_method::HEAD },
    { "OPTIONS", http_method::OPTIONS },
    { "CONNECT", http_method::CONNECT },
    { "TRACE", http_method::TRACE },
    { "COPY", http_method::COPY },
    { "LINK", http_method::LINK },
    { "UNLINK", http_method::UNLINK },
    { "PURGE", http_method::PURGE },
    { "LOCK", http_method::LOCK },
    { "UNLOCK", http_method::UNLOCK },
    { "PROPFIND", http_method::PROPFIND },
    { "VIEW", http_method::VIEW },
    { "UNKNOWN", http_method::UNKNOWN }
};

static unordered_map<http_method, string> const http_reverse_method_map = {
    { http_method::GET, "GET" },
    { http_method::POST, "POST" },
    { http_method::DELETE, "DELETE" },
    { http_method::PUT, "PUT" },
    { http_method::PATCH, "PATCH" },
    { http_method::HEAD, "HEAD" },
    { http_method::OPTIONS, "OPTIONS" },
    { http_method::CONNECT, "CONNECT" },
    { http_method::TRACE, "TRACE" },
    { http_method::COPY, "COPY" },
    { http_method::LINK, "LINK" },
    { http_method::UNLINK, "UNLINK" },
    { http_method::PURGE, "PURGE" },
    { http_method::LOCK, "LOCK" },
    { http_method::UNLOCK, "UNLOCK" },
    { http_method::PROPFIND, "PROPFIND" },
    { http_method::VIEW, "VIEW" },
    { http_method::UNKNOWN, "UNKNOWN" }
};

static unordered_map<int, string> const http_status_map = {
    { 100, "CONTINUE" },
    { 101, "SWITCHING PROTOCOLS" },
    { 200, "OK" },
    { 201, "CREATED" },
    { 202, "ACCEPTED" },
    { 203, "NON-AUTHORITATIVE INFORMATION" },
    { 204, "NO CONTENT" },
    { 205, "RESET CONTENT" },
    { 206, "PARTIAL CONTENT" },
    { 300, "MULTIPLE CHOICES" },
    { 301, "MOVED PERMANENTLY" },
    { 302, "FOUND" },
    { 303, "SEE OTHER" },
    { 304, "NOT MODIFIED" },
    { 305, "USE PROXY" },
    { 307, "TEMPORARY REDIRECT" },
    { 400, "BAD REQUEST" },
    { 401, "UNAUTHORIZED" },
    { 402, "PAYMENT REQUIRED" },
    { 403, "FORBIDDEN" },
    { 404, "NOT FOUND" },
    { 405, "METHOD NOT ALLOWED" },
    { 406, "NOT ACCEPTABLE" },
    { 407, "PROXY AUTHENTICATION REQUIRED" },
    { 408, "REQUEST TIMEOUT" },
    { 409, "CONFLICT" },
    { 410, "GONE" },
    { 411, "LENGTH REQUIRED" },
    { 412, "PRECONDITION FAILED" },
    { 413, "PAYLOAD TOO LARGE" },
    { 414, "URI TOO LONG" },
    { 415, "UNSUPPORTED MEDIA TYPE" },
    { 416, "RANGE NOT SATISFIABLE" },
    { 417, "EXPECTATION FAILED" },
    { 418, "I'M A TEAPOT" },
    { 421, "MISDIRECTED REQUEST" },
    { 422, "UNPROCESSABLE ENTITY" },
    { 423, "LOCKED" },
    { 424, "FAILED DEPENDENCY" },
    { 426, "UPGRADE REQUIRED" },
    { 428, "PRECONDITION REQUIRED" },
    { 429, "TOO MANY REQUESTS" },
    { 431, "REQUEST HEADER FIELDS TOO LARGE" },
    { 451, "UNAVAILABLE FOR LEGAL REASONS" },
    { 500, "INTERNAL SERVER ERROR" },
    { 501, "NOT IMPLEMENTED" },
    { 502, "BAD GATEWAY" },
    { 503, "SERVICE UNAVAILABLE" },
    { 504, "GATEWAY TIMEOUT" },
    { 505, "HTTP VERSION NOT SUPPORTED" },
    { 506, "VARIANT ALSO NEGOTIATES" },
    { 507, "INSUFFICIENT STORAGE" },
    { 508, "LOOP DETECTED" },
    { 510, "NOT EXTENDED" },
    { 511, "NETWORK AUTHENTICATION REQUIRED" },
    { 420, "ENHANCE YOUR CALM" }
};

class name_value {
private:
    string _name;
    string _value;

protected:
    name_value() {}

public:
    name_value(string name, string value)
        : _name(std::move(name))
        , _value(std::move(value))
    {
    }
    virtual ~name_value() = default;
    name_value(const name_value&) = default;
    name_value& operator=(name_value const&) = default;
    name_value(name_value&&) = default;
    name_value& operator=(name_value&&) = default;

    string name() { return _name; }
    string value() { return _value; }

    virtual string to_string() { return ""; }
};

class http_header : public name_value {
public:
    http_header()
        : name_value()
    {
    }
    http_header(string name, string value)
        : name_value(name, value)
    {
    }

    string to_string() override { return name() + ": " + value() + "\r\n"; }
};

class query_param : public name_value {
public:
    query_param()
        : name_value()
    {
    }
    query_param(string name, string value)
        : name_value(name, value)
    {
    }

    string to_string() override { return name() + "=" + value(); }
};

enum http_version { HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2_0,
    HTTP_3_0 };

static std::unordered_map<std::string, http_version> const http_version_map = {
    { "HTTP/0.9", HTTP_0_9 },
    { "HTTP/1.0", HTTP_1_0 },
    { "HTTP/1.1", HTTP_1_1 },
    { "HTTP/2.0", HTTP_2_0 },
    { "HTTP/3.0", HTTP_3_0 }
};

static std::unordered_map<http_version, std::string> const http_reverse_version_map = {
    { HTTP_0_9, "HTTP/0.9" },
    { HTTP_1_0, "HTTP/1.0" },
    { HTTP_1_1, "HTTP/1.1" },
    { HTTP_2_0, "HTTP/2.0" },
    { HTTP_3_0, "HTTP/3.0" }
};

class http_request {
private:
    enum parser_state { METHOD,
        PATH,
        QUERY_PARAM_NAME,
        QUERY_PARAM_VALUE,
        VERSION,
        HEADER_NAME,
        HEADER_VALUE,
        BODY_CONTENT };
    http_method _method;
    http_version _http_version;
    string _path;
    string _client_ipaddr;
    string _body_content;
    unordered_map<string, http_header> _headers; // kinda goofy, whatever
    unordered_map<string, query_param> _query_params; // kinda goofy, whatever

public:
    http_request(anthracite_socket& s)
        : _path("")
    {
        string raw_data = s.recv_message(HTTP_HEADER_BYTES);
        _client_ipaddr = s.get_client_ip();

        parser_state state = METHOD;

        string scratch = "";
        string scratch_2 = "";
        for (int i = 0; i < raw_data.length(); i++) {
            switch (state) {
            case METHOD: {
                if (raw_data[i] == ' ') {
                    if (http_method_map.find(scratch) == http_method_map.end()) {
                        _method = http_method::UNKNOWN;
                    } else {
                        _method = http_method_map.find(scratch)->second;
                    }
                    scratch = "";
                    state = PATH;
                } else {
                    scratch += raw_data[i];
                }
            } break;

            case PATH: {
                switch (raw_data[i]) {
                case ' ':
                    state = VERSION;
                    break;
                case '?':
                    state = QUERY_PARAM_NAME;
                    break;
                default:
                    _path += raw_data[i];
                    break;
                }
            } break;

            case QUERY_PARAM_NAME: {
                if (raw_data[i] == ' ') {
                    scratch = "";
                    state = VERSION;
                } else if (raw_data[i] == '=') {
                    state = QUERY_PARAM_VALUE;
                } else {
                    scratch += raw_data[i];
                }
            } break;

            case QUERY_PARAM_VALUE: {
                if (raw_data[i] == ' ') {
                    _query_params[scratch] = query_param(scratch, scratch_2);
                    scratch = "";
                    scratch_2 = "";
                    state = VERSION;
                } else if (raw_data[i] == '&') {
                    _query_params[scratch] = query_param(scratch, scratch_2);
                    scratch = "";
                    scratch_2 = "";
                    state = QUERY_PARAM_NAME;
                } else {
                    scratch_2 += raw_data[i];
                }
            } break;

            case VERSION: {
                if (raw_data[i] == '\n') {
                    _http_version = http_version_map.find(scratch)->second;
                    scratch = "";
                    state = HEADER_NAME;
                } else if (raw_data[i] != '\r') {
                    scratch += raw_data[i];
                }
            } break;

            case HEADER_NAME: {
                if (raw_data[i] == '\n') {
                    scratch = "";
                    scratch_2 = "";
                    state = BODY_CONTENT;
                    break;
                } else if (raw_data[i] == ' ') {
                    scratch = "";
                    cout << "Error: Whitespace found in header name\n";
                    break;
                } else if (raw_data[i] == ':') {
                    state = HEADER_VALUE;
                    i++;
                } else {
                    scratch += raw_data[i];
                }
            } break;

            case HEADER_VALUE: {
                if (raw_data[i] == '\n') {
                    _headers[scratch] = http_header(scratch, scratch_2);
                    scratch = "";
                    scratch_2 = "";
                    state = HEADER_NAME;
                } else if (raw_data[i] != '\r') {
                    scratch_2 += raw_data[i];
                }
            } break;

            case BODY_CONTENT: {
                _body_content += raw_data[i];
            } break;
            }
        }
    }

    string path() { return _path; }

    http_method method() { return _method; }

    string client_ip() { return _client_ipaddr; }

    string to_string()
    {
        string response = "";
        response += http_reverse_method_map.find(_method)->second + " " + _path + "?";

        for (auto qp : _query_params) {
            response += qp.second.to_string() + "&";
        }

        response += " " + http_reverse_version_map.find(_http_version)->second + "\r\n";

        for (auto header : _headers) {
            response += header.second.to_string();
        }

        response += "\r\n";
        response += _body_content;

        return response;
    }
};

class http_response {
private:
    int _status_code;
    string _content;
    unordered_map<string, http_header> _headers; // kinda goofy, whatever

public:
    http_response(string content, int status_code = 200)
        : _content(std::move(content))
        , _status_code(status_code)
    {
    }

    int status_code() { return _status_code; }

    void add_header(http_header header, bool override_existing = true)
    {
        if (override_existing || _headers.find(header.name()) == _headers.end()) {
            _headers[header.name()] = header;
        }
    }

    string to_string()
    {
        string response = "";
        response += "HTTP/1.1 " + ::to_string(_status_code) + " " + http_status_map.find(_status_code)->second + "\r\n";

        add_header(http_header("Content-Type", "text/html"), false);
        add_header(http_header("Content-Length", ::to_string(_content.length())), false);
        add_header(http_header("Server", "Anthracite/0.0.1"), false);

        for (auto header : _headers) {
            response += header.second.to_string();
        }

        response += "\r\n";
        response += _content;

        return response;
    }
};
