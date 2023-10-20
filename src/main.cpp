#include "backends/file_backend.cpp"
#include <condition_variable>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <span>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

void log_request_and_response(http_request& req, std::unique_ptr<http_response>& resp);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

void handle_client(anthracite_socket s, backend& b, std::mutex& thread_wait_mutex, std::condition_variable& thread_wait_condvar, int& active_threads)
{
    http_request req(s);
    std::unique_ptr<http_response> resp = b.handle_request(req);
    log_request_and_response(req, resp);
    std::string header = resp->header_to_string();
    s.send_message(header);
    s.send_message(resp->content());
    resp.reset();
    s.close_conn();
    {
        std::lock_guard<std::mutex> lock(thread_wait_mutex);
        active_threads--;
    }
    thread_wait_condvar.notify_one();
}

int main(int argc, char** argv)
{
    auto args = std::span(argv, size_t(argc));
    int port_number = default_port;

    if (argc > 1) {
        port_number = atoi(args[1]);
    }

    std::cout << "Initializing Anthracite" << std::endl;
    anthracite_socket s(port_number);
    file_backend fb(argc > 2 ? args[2] : "./www");
    std::cout << "Initialization Complete" << std::endl;
    std::cout << "Listening for HTTP connections on port " << port_number << std::endl;

    int active_threads = 0;
    std::mutex thread_wait_mutex;
    std::condition_variable thread_wait_condvar;

    while (true) {
        s.wait_for_conn();
        std::unique_lock<std::mutex> lock(thread_wait_mutex);
        thread_wait_condvar.wait(lock, [active_threads] { return active_threads < max_worker_threads; });
        active_threads++;
        std::thread(handle_client, s, std::ref(fb), std::ref(thread_wait_mutex), std::ref(thread_wait_condvar), std::ref(active_threads)).detach();
    }

    exit(0);
}

void log_request_and_response(http_request& req, std::unique_ptr<http_response>& resp)
{
    std::cout << "[" << resp->status_code() << " " + http_status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http_reverse_method_map.find(req.method())->second + " " + req.path() << std::endl;
}
