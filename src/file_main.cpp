#include "../lib/backends/file_backend.hpp"
#include "../lib/config/config.hpp"
#include "../lib/log/log.hpp"
#include "../lib/thread_mgr/event_loop.hpp"
#include "signal.h"
#include "string.h"
#include <memory>

std::shared_ptr<anthracite::thread_mgr::thread_mgr> server = nullptr;

extern "C" void signalHandler(int signum)
{
    //anthracite::log::warn << "Caught signal SIG" << sigabbrev_np(signum) << ", exiting Anthracite" << std::endl;
    if (server != nullptr) {
        server->stop();
    }
}

int main(int argc, char** argv)
{
    anthracite::log::logger.initialize(anthracite::log::LOG_LEVEL_INFO);
    anthracite::log::info << "Starting Anthracite, a higher performance web server" << std::endl;
    signal(SIGINT, signalHandler);

    anthracite::backends::file_backend fb("./www");
    anthracite::config::config cfg(1, 10);
    cfg.add_http_config(anthracite::config::http_config(8080));
    // cfg.add_https_config(config::https_config(8081, "", ""));

    server = std::make_shared<anthracite::thread_mgr::event_loop>(fb, cfg);

    server->start();

    anthracite::log::info << "Stopping Anthracite, a higher performance web server" << std::endl;
}
