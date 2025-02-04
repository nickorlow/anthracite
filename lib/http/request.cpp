#include "request.hpp"
#include "constants.hpp"
#include "../log/log.hpp"
#include <stdio.h>

namespace anthracite::http {

    request::request(std::string& raw_data, const std::string& client_ip)
        : _path(""), _client_ipaddr(client_ip)
    {

        parser_state state = METHOD;

        std::string scratch = "";
        std::string scratch_2 = "";
        for (int i = 0; i < raw_data.length(); i++) {
            switch (state) {
            case METHOD: {
                if (raw_data[i] == ' ') {
                    if (method_map.find(scratch) == method_map.end()) {
                        _method = method::UNKNOWN;
                    } else {
                        _method = method_map.find(scratch)->second;
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
                    _http_version = version_map.find(scratch)->second;
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
                    _headers[scratch] = header(scratch, scratch_2);
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

    std::string request::path() { return _path; }

    method request::get_method() { return _method; }

    std::string request::client_ip() { return _client_ipaddr; }

    version request::get_http_version() {
      return _http_version;
    }

    bool request::is_supported_version() {
        //log::err << reverse_version_map.find(_http_version)->second << std::endl;
        return _http_version == HTTP_1_1 || _http_version == HTTP_1_0;
    }

    bool request::close_connection() {
      const auto& header = _headers.find("Connection");
      const bool found = header != _headers.end();

      if(found && header->second.value() == "keep-alive") {
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
