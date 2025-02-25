#include "../lib/backends/file_backend.hpp"
#include "../lib/log/log.hpp"
#include "../lib/socket/openssl_socket.hpp"
#include "../lib/socket/socket.hpp"
#include "../lib/thread_mgr/event_loop.hpp"
#include "signal.h"
#include "getopt.h"
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct event_loop_config {
    int max_workers;
    int max_clients;
};

struct server_config {
    std::unordered_map<int, anthracite::socket::listener*> listeners;
    std::optional<event_loop_config> event_loop;
    std::string serve_dir = "./www";
    enum anthracite::log::LOG_LEVEL log_level = anthracite::log::LOG_LEVEL_INFO;
};

std::shared_ptr<anthracite::thread_mgr::thread_mgr> server = nullptr;

extern "C" void signalHandler(int signum)
{
    anthracite::log::warn << "Caught signal SIGIN, exiting Anthracite" << std::endl;
    if (server != nullptr) {
        server->stop();
    }
}

bool read_config(server_config& config, const std::string& config_path)
{
    std::ifstream config_stream(config_path);
    if (!config_stream.is_open()) {
        anthracite::log::err << "Unable to open configuration file at '" << config_path << "', ensure that the file exists and permissions are set correctly" << std::endl;
        return false;
    }

    std::string line;
    for (int lineno = 1; std::getline(config_stream, line); lineno++) {
        bool parse_failed = false;

        std::stringstream ss_line(line);
        std::string directive;
        ss_line >> directive;

        if (ss_line.fail()) {
            continue;
        }

        if (directive == "http" || directive == "https") {
            // http PORT QUEUE_LEN NONBLOCK
            // https PORT QUEUE_LEN NONBLOCK CRT_PATH KEY_PATH
            int port;
            int queue_len;
            std::string block;
            std::string crt_path;
            std::string key_path;
            ss_line >> port >> queue_len >> block;

            if (directive == "https") {
                ss_line >> crt_path >> key_path;
            }

            bool nonblocking = false;
            if (block == "blocking") {
                nonblocking = false;
            } else if (block == "nonblocking") {
                nonblocking = true;
            } else {
                anthracite::log::err << "BLOCK is not a string of value blocking or nonblocking";
                parse_failed = true;
            }

            parse_failed |= ss_line.fail();
            parse_failed |= !ss_line.eof();

            if (parse_failed) {
                anthracite::log::err << "Invalid http/https config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "Format is: " << std::endl;
                anthracite::log::err << "http PORT QUEUE_LENGTH BLOCK" << std::endl;
                anthracite::log::err << "https PORT QUEUE_LENGTH BLOCK CRT_PATH KEY_PATH" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "PORT, QUEUE_LENGTH are integers" << std::endl;
                anthracite::log::err << "BLOCK is a string of value blocking or nonblocking" << std::endl;
                anthracite::log::err << "CRT_PATH and KEY_PATH are strings for the path or the certificate and key files respectively" << std::endl;
                anthracite::log::err << std::endl
                                     << "Line was: " << std::endl
                                     << line << std::endl;
                anthracite::log::err << "Check for trailing whitespace!" << std::endl;
                return false;
            }

            if (config.listeners.contains(port)) {
                anthracite::log::err << "Invalid http/https config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << "Port " << port << " is already being used" << std::endl;
                return false;
            }

            if (directive == "https") {
                config.listeners[port] = new anthracite::socket::openssl_listener(key_path, crt_path, port, queue_len, nonblocking);
            } else {
                config.listeners[port] = new anthracite::socket::listener(port, queue_len, nonblocking);
            }
        } else if (directive == "log_level") {
            // log_level LEVEL
            std::string log_level;
            ss_line >> log_level;

            parse_failed |= ss_line.fail();
            parse_failed |= !ss_line.eof();

            if (log_level == "DEBUG") {
                config.log_level = anthracite::log::LOG_LEVEL::LOG_LEVEL_DEBUG;
            } else if (log_level == "VERBOSE") {
                config.log_level = anthracite::log::LOG_LEVEL::LOG_LEVEL_VERBOSE;
            } else if (log_level == "INFO") {
                config.log_level = anthracite::log::LOG_LEVEL::LOG_LEVEL_INFO;
            } else if (log_level == "WARN") {
                config.log_level = anthracite::log::LOG_LEVEL::LOG_LEVEL_WARN;
            } else if (log_level == "ERROR") {
                config.log_level = anthracite::log::LOG_LEVEL::LOG_LEVEL_ERROR;
            } else {
                parse_failed = true;
            }

            if (parse_failed) {
                anthracite::log::err << "Invalid log_level config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "Format is: " << std::endl;
                anthracite::log::err << "log_level LEVEL" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "LEVEL is string of value DEBUG, VERBOSE, INFO, WARN, ERROR" << std::endl;
                anthracite::log::err << std::endl
                                     << "Line was: " << std::endl
                                     << line << std::endl;
                anthracite::log::err << "Check for trailing whitespace!" << std::endl;
                return false;
            }
        } else if (directive == "event_loop") {
            // event_loop MAX_WORKERS MAX_CLIENS
            int max_workers;
            int max_clients;

            ss_line >> max_workers >> max_clients;

            parse_failed |= ss_line.fail();
            parse_failed |= !ss_line.eof();

            if (parse_failed) {
                anthracite::log::err << "Invalid event_loop config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "Format is: " << std::endl;
                anthracite::log::err << "event_loop MAX_WORKERS MAX_CLIENTS" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "MAX_WORKERS is the maximum number of worker threads" << std::endl;
                anthracite::log::err << "MAX_CLIENTS is the maximum number of concurrent clients" << std::endl;
                anthracite::log::err << std::endl
                                     << "Line was: " << std::endl
                                     << line << std::endl;
                anthracite::log::err << "Check for trailing whitespace!" << std::endl;
                return false;
            }

            if (max_workers <= 0) {
                anthracite::log::err << "Invalid event_loop config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << "MAX_WORKERS must be a positive, nonzero number" << std::endl;
                return false;
            }

            if (max_clients <= 0) {
                anthracite::log::err << "Invalid event_loop config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << "MAX_CLIENTS must be a positive, nonzero number" << std::endl;
                return false;
            }

            if (config.event_loop.has_value()) {
                anthracite::log::err << "Invalid event_loop config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << "A thread manager was already specified. You may only specify one at a time as of now." << std::endl;
                return false;
            }

            // Eww
            config.event_loop = { .max_workers = max_workers, .max_clients = max_clients };
        } else if (directive == "www_dir") {
            std::string www_dir;
            ss_line >> www_dir;

            parse_failed |= ss_line.fail();
            parse_failed |= !ss_line.eof();

            if (parse_failed) {
                anthracite::log::err << "Invalid www_dir config on line " << lineno << " of configuration file" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "Format is: " << std::endl;
                anthracite::log::err << "www_dir PATH" << std::endl;
                anthracite::log::err << std::endl;
                anthracite::log::err << "PATH is a path to a directory containing files to serve" << std::endl;
                anthracite::log::err << std::endl
                                     << "Line was: " << std::endl
                                     << line << std::endl;
                anthracite::log::err << "Check for trailing whitespace!" << std::endl;
                return false;
            }
        } else {
            anthracite::log::err << "Invalid configuration. Unknown directive " << directive << " on line " << lineno << std::endl;
            return false;
        }
    }

    if (!config.event_loop.has_value()) {
        anthracite::log::err << "Invalid configuration. Missing a thread manager. Try adding an event_loop directive." << std::endl;
        return false;
    }

    if (config.listeners.size() == 0) {
        anthracite::log::err << "Invalid configuration. Missing listeners. Try adding a http or https directive." << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);
    anthracite::log::logger.initialize(anthracite::log::LOG_LEVEL_INFO);
    if (pthread_setname_np(pthread_self(), "main") != 0) {
        anthracite::log::err << "Failed to set thread name via pthread_setname_np" << std::endl;
    }

    int opt_index = 0;
    option options[] = {
        { "help", no_argument, 0, 'h' },
        { "config", required_argument, 0, 'c' }
    };

    char c;
    std::string config_path = "./anthracite.cfg";
    bool config_set = false;
    while ((c = getopt_long(argc, argv, "hc:", options, &opt_index)) != -1) {
        switch (c) {
        case 'h': {
            std::cerr << "Anthracite Help" << std::endl;
            std::cerr << std::endl;
            std::cerr << "-h, --help                                 Prints this help menu " << std::endl;
            std::cerr << std::endl;
            std::cerr << "-c, --config string (optional)             Specifies the path of the  configuration" << std::endl;
            std::cerr << "                                           file. Default is './anthracite.cfg'" << std::endl;
            std::cerr << std::endl;
            return 0;
            break;
        };
        case 'c': {
            if (config_set) {
                anthracite::log::err << "You cannot specify multiple configuration files" << std::endl;
                return 1;
            }
            config_set = true;
            config_path = std::string(optarg);
            break;
        };
        }
    }

    anthracite::log::info << "Loading configuration file at path '" << config_path << "'" << std::endl;

    server_config cfg;
    if (!read_config(cfg, config_path)) {
        anthracite::log::err << "Failed reading configuration file at path '" << config_path << "'" << std::endl;
        return 1;
    }

    anthracite::log::logger.initialize(cfg.log_level);

    anthracite::log::info << "Serving files in directory " << cfg.serve_dir << std::endl;
    anthracite::backends::file_backend fb(cfg.serve_dir);

    std::vector<anthracite::socket::listener*> listeners;

    for (auto lp : cfg.listeners) {
        listeners.push_back(lp.second);
    }

    server = std::make_shared<anthracite::thread_mgr::event_loop>(listeners, fb, cfg.event_loop->max_workers, cfg.event_loop->max_clients);

    anthracite::log::info << "Starting Anthracite, a very high performance web server" << std::endl;

    server->start();

    for (auto listener : listeners) {
        delete listener;
    }

    anthracite::log::info << "Stopping Anthracite, a very high performance web server" << std::endl;
}
