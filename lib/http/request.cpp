#include "request.hpp"
#include "../log/log.hpp"
#include "constants.hpp"
#include <cstring>
#include <stdio.h>

namespace anthracite::http {

void request::parse_header(char* raw_line) {
    uint32_t delim_pos = strstr(raw_line, ":") - raw_line + 1;

    std::string query_val((char*)(raw_line + delim_pos));
    std::string query_name(raw_line, delim_pos - 1);

    _headers[query_name] = header(query_name, query_val);
}

void request::parse_query_param(char* raw_param) {
    uint32_t delim_pos = strstr(raw_param, "=") - raw_param + 1;

    std::string query_val((char*)(raw_param + delim_pos));
    std::string query_name(raw_param, delim_pos - 1);

    _query_params[query_name] = query_param(query_name, query_val);
}

void request::parse_path(char* raw_path) {
    char* saveptr = nullptr;
    char* tok = strtok_r(raw_path, "?", &saveptr);

    if (tok){
        _path = tok;
    }

    tok = strtok_r(nullptr, "&", &saveptr);
    while(tok) {
        parse_query_param(tok);
        tok = strtok_r(nullptr, "&", &saveptr);
    }
}

void request::parse_request_line(std::string& raw_line) {
        request_line_parser_state state = METHOD;
        std::stringstream ss(raw_line);

        char* saveptr = nullptr;
        char* tok = strtok_r(raw_line.data(), " ", &saveptr);

        while(tok){
            switch(state) {
                case METHOD: {
                    auto search = method_map.find(tok);
                    if (search != method_map.end()) {
                        _method = search->second;
                    } else {
                        _method = method::UNKNOWN;
                    }

                    state = PATH;
                    break;
                };
                
                case PATH: {
                    parse_path(tok);
                    state = VERSION;
                    break;
                };

                case VERSION: {
                    auto search = version_map.find(tok);
                    if (search != version_map.end()) {
                        _http_version = search->second;
                    } else {
                        _http_version = version::HTTP_1_0;
                    }
                    return;
                };
            }
            tok = strtok_r(nullptr, " ", &saveptr);
        }
}

request::request(std::string& raw_data, const std::string& client_ip)
    : _path("")
    , _client_ipaddr(client_ip)
{

    parser_state state = REQUEST_LINE;

    std::stringstream line_stream(raw_data);
    std::string line;

    while(getline(line_stream, line, '\n') && state != BODY_CONTENT){
        line.pop_back(); // HTTP requests do newline as \r\n, this removes the \r 
        switch(state) {
            case REQUEST_LINE: {
                                   parse_request_line(line); 
                                   state = HEADERS;
                                   break;
                               };
            case HEADERS: {
                if (line.length() == 0) {
                    state = BODY_CONTENT;
                } else {
                    parse_header(line.data()); 
                }
                break;
            };
            case BODY_CONTENT: break;
        }
    }

    if (getline(line_stream, line, '\0')) {
        _body_content = line;
    }
}

std::string request::path() { return _path; }

method request::get_method() { return _method; }

std::string request::client_ip() { return _client_ipaddr; }

version request::get_http_version()
{
    return _http_version;
}

bool request::is_supported_version()
{
    return _http_version == HTTP_1_1 || _http_version == HTTP_1_0;
}

bool request::close_connection()
{
    const auto& header = _headers.find("Connection");
    const bool found = header != _headers.end();

    if (found && header->second.value() == "keep-alive") {
        return false;
    }

    return true;
}

std::string request::to_string()
{
    std::string response = "";
    response += reverse_method_map.find(_method)->second + " " + _path + "?";

    for (auto qp : _query_params) {
        response += qp.second.to_string() + "&";
    }

    response += " " + reverse_version_map.find(_http_version)->second + "\r\n";

    for (auto header : _headers) {
        response += header.second.to_string();
    }

    response += "\r\n";
    response += _body_content;

    return response;
}

};
