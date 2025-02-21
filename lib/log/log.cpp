#include "./log.hpp"

namespace anthracite::log {
enum LOG_LEVEL Logger::_level = LOG_LEVEL_NONE;

// TODO: implement logger as a singleton to prevent duplicates
Logger::Logger() = default;
void Logger::initialize(enum LOG_LEVEL level)
{
    _level = level;
}

void Logger::log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros)
{
    log::info << "[" << resp->status_code() << " " + http::status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http::reverse_method_map.find(req.get_method())->second + " " + req.path() << " in " << micros << " usecs" << std::endl;
}

LogBuf::LogBuf(std::ostream& output_stream, const std::string& tag, enum LOG_LEVEL level)
    : _output_stream(output_stream)
    , _tag(tag)
    , _level(level)
{
}

int LogBuf::sync()
{
    if (this->_level <= logger._level) {
        std::cout << "[" << this->_tag << "] " << this->str();
        std::cout.flush();
    }
    this->str("");
    return 0;
}
};
