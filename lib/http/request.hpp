#pragma once

#include <string>
#include <unordered_map>
#include "./header_query.hpp"
#include "./constants.hpp"

namespace anthracite::http {

class request {
private:
    enum parser_state { METHOD,
        PATH,
        QUERY_PARAM_NAME,
        QUERY_PARAM_VALUE,
        VERSION,
        HEADER_NAME,
        HEADER_VALUE,
        BODY_CONTENT };
    method _method;
    version _http_version;
    std::string _path;
    std::string _client_ipaddr;
    std::string _body_content;
    std::unordered_map<std::string, header> _headers; // kinda goofy, whatever
    std::unordered_map<std::string, query_param> _query_params; // kinda goofy, whatever

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
