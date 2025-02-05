#pragma once

#include <string>
#include <unordered_map>
#include "./header_query.hpp"
#include "./constants.hpp"

namespace anthracite::http {

class request {
private:
    enum request_line_parser_state {
        METHOD, PATH, VERSION
    };

    enum parser_state {
        REQUEST_LINE,
        HEADERS,
        BODY_CONTENT
    };

    method _method;
    version _http_version;
    std::string _path;
    std::string _client_ipaddr;
    std::string _body_content;
    std::unordered_map<std::string, std::string> _headers; 
    std::unordered_map<std::string, std::string> _query_params;

    void parse_request_line(char* raw_line);
    void parse_header(std::string& raw_line);
    void parse_path(char* raw_path);
    void parse_query_param(std::string& raw_param);

public:
    request(std::string& raw_data, const std::string& client_ip);
    std::string path();
    method get_method();
    std::string client_ip();
    version get_http_version();
    bool is_supported_version();
    bool close_connection();
    std::string to_string();
};

};
