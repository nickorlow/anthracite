#include "http.hpp"

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
    std::string _path;
    std::string _client_ipaddr;
    std::string _body_content;
    std::unordered_map<std::string, http_header> _headers; // kinda goofy, whatever
    std::unordered_map<std::string, query_param> _query_params; // kinda goofy, whatever

public:
    http_request(anthracite_socket& s)
        : _path(""), _client_ipaddr(s.get_client_ip())
    {
      std::string raw_data = s.recv_message(HTTP_HEADER_BYTES);

        parser_state state = METHOD;

        std::string scratch = "";
        std::string scratch_2 = "";
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

    std::string path() { return _path; }

    http_method method() { return _method; }

    std::string client_ip() { return _client_ipaddr; }

    std::string to_string()
    {
      std::string response = "";
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
