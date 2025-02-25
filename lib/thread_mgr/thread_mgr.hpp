#pragma once

#include "../backends/backend.hpp"

namespace anthracite::thread_mgr {
    class thread_mgr {
    protected:
        bool _run;
        backends::backend& _backend;
    public:
        thread_mgr(backends::backend& backend): _backend(backend) {} 
        virtual ~thread_mgr() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
    };
};
