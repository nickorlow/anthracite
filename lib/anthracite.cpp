#include "./anthracite.hpp"
#include "./log/log.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "./config/config.hpp"
#include "./thread_mgr/event_loop.hpp"
#include <signal.h>

using namespace anthracite;

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

thread_mgr::event_loop* elp = nullptr;


extern "C" void signalHandler(int signum) {
    log::warn << "Caught signal #" << signum << ", exiting Anthracite" << std::endl;
    elp->stop();
}


int anthracite_main(backends::backend& be, config::config& config)
{
    signal(SIGTERM, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGABRT, signalHandler);

    log::logger.initialize(log::LOG_LEVEL_INFO);
    thread_mgr::event_loop el(be, config);
    elp = &el;
    el.start();
    return 0;
}

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros)
{
    log::info << "[" << resp->status_code() << " " + http::status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http::reverse_method_map.find(req.get_method())->second + " " + req.path() << " in " << micros << " usecs" << std::endl;
}
