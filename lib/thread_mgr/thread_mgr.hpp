#pragma once

#include "../backends/backend.hpp"
#include "../config/config.hpp"

namespace anthracite::thread_mgr {
    class thread_mgr {
    protected:
        bool _run;
        backends::backend& _backend;
        config::config& _config;
    public:
        thread_mgr(backends::backend& backend, config::config& config): _backend(backend), _config(config) {} 
        virtual ~thread_mgr() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
    };
};
