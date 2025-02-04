#include "response.hpp"
#include "../build_supp/version.cpp"

namespace anthracite::http {

    response::response() {};

    int response::status_code() { return _status_code; }

    void response::add_body(const std::string body) {
        _content_noref = body;
        _content = &_content_noref;
    }
    
    void response::add_body_ref(std::string* body) {
        _content = body; 
    }

    void response::add_header(header header, bool override_existing)
    {
        if (override_existing || _headers.find(header.name()) == _headers.end()) {
            _headers[header.name()] = header;
        }
    }

    void response::add_status(int  code) {
        _status_code = code;
    }

    std::string& response::content()
    {
        return *_content;
    }

    std::string response::header_to_string()
    {
        std::string response = "";
        response += "HTTP/1.1 " + std::to_string(_status_code) + " " + status_map.find(_status_code)->second + "\r\n";

        add_header(header("Content-Length", std::to_string(_content->length())), false);
        add_header(header("Server", ANTHRACITE_FULL_VERSION_STRING), false);
        add_header(header("Origin-Server", ANTHRACITE_FULL_VERSION_STRING), false);

        for (auto header : _headers) {
            response += header.second.to_string();
        }

        response += "\r\n";

        return response;
    }

    std::string response::to_string()
    {
        return header_to_string() + *_content;
    }

};
