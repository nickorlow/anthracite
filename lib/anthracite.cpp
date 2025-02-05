#include "./anthracite.hpp"
#include "./log/log.hpp"
#include "./socket/socket.hpp"
#include "./socket/tls_socket.hpp"
#include "backends/file_backend.hpp"
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <span>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace anthracite;

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

void handle_client(socket::tls_socket s, backends::backend& b, backends::file_backend& fb, std::mutex& thread_wait_mutex, std::condition_variable& thread_wait_condvar, int& active_threads)
{
    while (true) {
        std::string raw_request = s.recv_message(http::HEADER_BYTES);
        
        // We're doing the start here even though it would ideally be done 
        // before the first line since if we leave the connection open for 
        // HTTP 1.1, we can spend a bit of time waiting
        auto start = high_resolution_clock::now();

        if (raw_request == "") {
            break;
        }

        continue;
    }
    s.close_conn();
    {
        std::lock_guard<std::mutex> lock(thread_wait_mutex);
        active_threads--;
    }
    thread_wait_condvar.notify_one();
}

int anthracite_main(int argc, char** argv, backends::backend& be)
{
    log::logger.initialize(log::LOG_LEVEL_DEBUG);
    auto args = std::span(argv, size_t(argc));
    int port_number = default_port;

    if (argc > 1) {
        port_number = atoi(args[1]);
    }

    socket::tls_socket s(port_number);
    backends::file_backend fb(argc > 2 ? args[2] : "./www");
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

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros)
{
    log::info << "[" << resp->status_code() << " " + http::status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http::reverse_method_map.find(req.get_method())->second + " " + req.path() << " in " << micros << " usecs" << std::endl;
}
