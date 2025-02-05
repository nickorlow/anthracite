#include "request.hpp"
#include "../log/log.hpp"
#include "constants.hpp"
#include <map>
#include <cstring>
#include <stdio.h>

namespace anthracite::http {

void request::parse_header(std::string& raw_line) {
    auto delim_pos = raw_line.find_first_of(':');
    auto value_pos = raw_line.find_first_not_of(' ', delim_pos+1);

    std::string header_name = raw_line.substr(0,delim_pos);
    std::string header_val = raw_line.substr(value_pos);

    _headers[header_name] = header_val;
}

void request::parse_query_param(std::string& raw_param) {
    auto delim_pos = raw_param.find_first_of('=');
    auto value_pos = delim_pos+1;

    std::string query_name = raw_param.substr(0,delim_pos);
    std::string query_val = raw_param.substr(value_pos);

    _query_params[query_name] = query_val;
}

void request::parse_path(char* raw_path) {
    char* saveptr = nullptr;
    char* tok = strtok_r(raw_path, "?", &saveptr);

    if (tok){
        _path = tok;
    }

    tok = strtok_r(nullptr, "?", &saveptr);
    while(tok) {
        std::string rtok(tok);
        parse_query_param(rtok);
        tok = strtok_r(nullptr, "?", &saveptr);
    }
}

void request::parse_request_line(char* raw_line) {
        request_line_parser_state state = METHOD;

        char* saveptr = nullptr;
        char* tok = strtok_r(raw_line, " \r", &saveptr);

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
                    std::string str_tok(tok);
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
            tok = strtok_r(nullptr, " \r", &saveptr);
        }
}

request::request(std::string& raw_data, const std::string& client_ip)
    : _path("")
    , _client_ipaddr(client_ip)
{

    parser_state state = REQUEST_LINE;

    char* saveptr = nullptr;
    char* tok = strtok_r(raw_data.data(), "\r\n", &saveptr);

    while(tok && state != BODY_CONTENT){
        switch(state) {
            case REQUEST_LINE: {
                                   parse_request_line(tok); 
                                   state = HEADERS;
                                    tok = strtok_r(nullptr, "\n", &saveptr);
                                   break;
                               };
            case HEADERS: {
                if (tok[0] == '\r') {
                    state = BODY_CONTENT;
                } else {
                    std::string rtok(tok);
                    rtok.pop_back();
                    parse_header(rtok); 
                tok = strtok_r(nullptr, "\n", &saveptr);
                }
                break;
            };
            case BODY_CONTENT: break;
        }
    }

    tok = strtok_r(nullptr, "", &saveptr);
    if (tok) {
        _body_content = std::string(tok);
    }
    //if (getline(line_stream, line, '\0')) {
    //    _body_content = line;
    //}
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

    if (found && header->second == "keep-alive") {
        return false;
    }

    return true;
}

std::string request::to_string()
{
    std::string response = "";
    response += reverse_method_map.find(_method)->second + " " + _path;

    if (_query_params.size() > 0) {
        response += "?";
    }

    auto qp_map = std::map(_query_params.begin(), _query_params.end());
    auto qp = qp_map.begin();
    while (qp != qp_map.end()) {
        response += qp->first + "=" + qp->second;
        if (++qp != qp_map.end()) {
            response += "&";
        }
    }

    response += " " + reverse_version_map.find(_http_version)->second + "\r\n";

    if (_headers.size() == 0) {
        response += "\r\n";
    }

    auto hd_map = std::map(_headers.begin(), _headers.end());
    auto hd = hd_map.begin();
    while (hd != hd_map.end()) {
        response += hd->first + ": " + hd->second + "\r\n";
        if (++hd == hd_map.end()) {
            response += "\r\n";
        }
    }

    response += _body_content;

    return response;
}

};
