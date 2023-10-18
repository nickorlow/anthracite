#include <string>
#include <unordered_map>

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

static std::unordered_map<std::string, http_method> const http_method_map = {
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

static std::unordered_map<http_method, std::string> const http_reverse_method_map = {
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

static std::unordered_map<int, std::string> const http_status_map = {
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

static std::unordered_map<std::string, std::string> const mime_types = {
    { "html", "text/html" },
    { "css", "text/css" },

    { "js", "application/javascript" },
    { "pdf", "application/pdf" },

    { "ico",  "image/x-icon" },
    { "jpg",  "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "png",  "image/png" },
    { "gif",  "image/gif" },
    { "bmp",  "image/bmp" },

    { "mp4", "video/mp4" },
    { "avi", "video/x-msvideo" },
    { "mkv", "video/x-matroska" },
    { "mov", "video/quicktime" },
    { "wmv", "video/x-ms-wmv" },
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




