#include "request.hpp"
#include "../log/log.hpp"
#include "constants.hpp"
#include <stdio.h>

namespace anthracite::http {

void request::parse_header(std::string& raw_line) {
    auto delim_pos = raw_line.find_first_of(':');
    auto value_pos = raw_line.find_first_not_of(' ', delim_pos+1);

    std::string header_name = raw_line.substr(0,delim_pos);
    std::string header_val = raw_line.substr(value_pos);

    _headers[header_name] = header(header_name, header_val);
}

void request::parse_query_param(std::string& raw_param) {
    auto delim_pos = raw_param.find_first_of('=');
    auto value_pos = delim_pos+1;

    std::string query_name = raw_param.substr(0,delim_pos);
    std::string query_val = raw_param.substr(value_pos);

    _query_params[query_name] = query_param(query_name, query_val);
}

void request::parse_path(std::string& raw_path) {
    std::stringstream ss(raw_path);
    std::string tok;

    if (getline(ss, tok, '?')){
        _path = tok;
    }
    
    while(getline(ss, tok, '&')) {
        parse_query_param(tok);
    }
}

void request::parse_request_line(std::string& raw_line) {
        request_line_parser_state state = METHOD;
        std::stringstream ss(raw_line);
        std::string tok;
        while(getline(ss, tok, ' ')){
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
                    parse_header(line); 
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
