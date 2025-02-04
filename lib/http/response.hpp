#pragma once

#include <string>
#include <unordered_map>
#include "header_query.hpp"
#include "constants.hpp"
#include <optional>

namespace anthracite::http {

class response {
private:
    int _status_code;
    std::string* _content;
    std::string _content_noref;
    std::unordered_map<std::string, header> _headers; // kinda goofy, whatever

public:
    response();
    int status_code();

    void add_body(const std::string body);
    void add_body_ref(std::string* body);
    void add_status(int);
    void add_header(header header, bool override_existing = true);
    std::string& content();
    std::string header_to_string();
    std::string to_string();
};

};
