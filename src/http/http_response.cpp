#include "http.hpp"

class http_response {
private:
    int _status_code;
    std::string& _content;
    std::string _filename;
    std::unordered_map<std::string, http_header> _headers; // kinda goofy, whatever

public:
    http_response(std::string& content, std::string filename, int status_code = 200)
        : _content(content)
        , _status_code(status_code)
        , _filename(std::move(filename))
    {
    }

    int status_code() { return _status_code; }

    void add_header(http_header header, bool override_existing = true)
    {
        if (override_existing || _headers.find(header.name()) == _headers.end()) {
            _headers[header.name()] = header;
        }
    }

    std::string& content()
    {
        return _content;
    }

    std::string header_to_string()
    {
        std::string response = "";
        response += "HTTP/1.1 " + std::to_string(_status_code) + " " + http_status_map.find(_status_code)->second + "\r\n";
        std::string content_type = "text/html";
        std::string file_extension = _filename.substr(_filename.rfind('.') + 1);
        auto mime_type = mime_types.find(file_extension);
        if (mime_type != mime_types.end()) {
            content_type = mime_type->second;
        }
        add_header(http_header("Content-Type", content_type), false);
        add_header(http_header("Content-Length", std::to_string(_content.length())), false);
        add_header(http_header("Server", "Anthracite/0.0.1"), false);
        add_header(http_header("Origin-Server", "Anthracite/0.0.1"), false);

        for (auto header : _headers) {
            response += header.second.to_string();
        }

        response += "\r\n";

        return response;
    }

    std::string to_string()
    {
        return header_to_string() + _content;
    }
};
