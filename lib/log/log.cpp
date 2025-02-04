#include "./log.hpp"

namespace anthracite::log {
enum LOG_LEVEL Logger::_level = LOG_LEVEL_NONE;

// TODO: implement logger as a singleton to prevent duplicates
Logger::Logger() = default;
void Logger::initialize(enum LOG_LEVEL level)
{
    _level = level;
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
