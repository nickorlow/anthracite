#include "backends/file_backend.hpp"
#include "./anthracite_main.hpp"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <span>
#include "./log/log.hpp"
#include "./socket.hpp"

using namespace anthracite;

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

void handle_client(socket::anthracite_socket s, backends::backend& b, backends::file_backend& fb, std::mutex& thread_wait_mutex, std::condition_variable& thread_wait_condvar, int& active_threads)
{
    while (true) {
        std::string raw_request = s.recv_message(http::HEADER_BYTES);
        if (raw_request == "") {
            break;
        }
        http::request req(raw_request, s.get_client_ip());
        std::unique_ptr<http::response> resp = req.is_supported_version() ? b.handle_request(req) : fb.handle_error(http::status_codes::HTTP_VERSION_NOT_SUPPORTED);
        log_request_and_response(req, resp);
        std::string header = resp->header_to_string();
        s.send_message(header);
        s.send_message(resp->content());
        resp.reset();
        if (req.close_connection()) {
            break;
        }
    }
    s.close_conn();
    {
        std::lock_guard<std::mutex> lock(thread_wait_mutex);
        active_threads--;
    }
    thread_wait_condvar.notify_one();
}

//int main(int argc, char** argv)
int anthracite_main(int argc, char** argv, backends::backend& be)
{
    log::logger.initialize(log::LOG_LEVEL_INFO);
    auto args = std::span(argv, size_t(argc));
    int port_number = default_port;

    if (argc > 1) {
        port_number = atoi(args[1]);
    }
    log::verbose << "Initializing Anthracite" << std::endl;
    socket::anthracite_socket s(port_number);
    backends::file_backend fb(argc > 2 ? args[2] : "./www");
    log::verbose << "Initialization Complete" << std::endl;
    log::info << "Listening for HTTP connections on port " << port_number << std::endl;

    int active_threads = 0;
    std::mutex thread_wait_mutex;
    std::condition_variable thread_wait_condvar;

    while (true) {
        s.wait_for_conn();
        std::unique_lock<std::mutex> lock(thread_wait_mutex);
        thread_wait_condvar.wait(lock, [active_threads] { return active_threads < max_worker_threads; });
        active_threads++;
        std::thread(handle_client, s, std::ref(be), std::ref(fb), std::ref(thread_wait_mutex), std::ref(thread_wait_condvar), std::ref(active_threads)).detach();
    }

    exit(0);
}

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp)
{
    log::info << "[" << resp->status_code() << " " + http::status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http::reverse_method_map.find(req.get_method())->second + " " + req.path() << std::endl;
}
