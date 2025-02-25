#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <inttypes.h>
#include <memory>
#include <string>
#include "../http/request.hpp"
#include "../http/response.hpp"

namespace anthracite::log {
    enum LOG_LEVEL {
        LOG_LEVEL_NONE = 0,
        LOG_LEVEL_ERROR = 1,
        LOG_LEVEL_WARN = 2,
        LOG_LEVEL_INFO = 3,
        LOG_LEVEL_VERBOSE = 4,
        LOG_LEVEL_DEBUG = 5
    };

    class Logger {
        friend class LogBuf;
        static enum LOG_LEVEL _level;
        public:
        
        Logger();
        void initialize(enum LOG_LEVEL level);
        void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros);
    };
    
    class LogBuf : public std::stringbuf
    {
            std::string _tag;
            std::ostream& _output_stream;
            enum LOG_LEVEL _level;
        public:
            LogBuf(std::ostream& output_stream, const std::string& tag, enum LOG_LEVEL level);
        int sync() override;
    };
    
    static class Logger logger{};

    static class LogBuf errBuf{std::cerr, "EROR", LOG_LEVEL_ERROR};
    static std::ostream err(&errBuf);

    static class LogBuf warnBuf{std::cerr, "WARN", LOG_LEVEL_WARN};
    static std::ostream warn(&warnBuf);

    static class LogBuf infoBuf{std::cout, "INFO", LOG_LEVEL_INFO};
    static std::ostream info(&infoBuf);

    static class LogBuf verboseBuf{std::cout, "VERB", LOG_LEVEL_VERBOSE};
    static std::ostream verbose(&verboseBuf);

    static class LogBuf debugBuf{std::cout, "DEBG", LOG_LEVEL_DEBUG};
    static std::ostream debug(&debugBuf);


};
