#include "backends/file_backend.cpp"
#include <condition_variable>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

void log_request_and_response(http_request& req, std::unique_ptr<http_response>& resp);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

int active_threads = 0;
std::mutex mtx;
std::condition_variable cv;

void handle_client(anthracite_socket s, file_backend& fb)
{
    http_request req(s);
    std::unique_ptr<http_response> resp = fb.handle_request(req);
    log_request_and_response(req, resp);
    std::string header = resp->header_to_string();
    s.send_message(header);
    s.send_message(resp->content());
    resp.reset();
    s.close_conn();
    {
        std::lock_guard<std::mutex> lock(mtx);
        active_threads--;
    }
    cv.notify_one();
}

int main(int argc, char** argv)
{
    int port_number = default_port;

    if (argc > 1) {
        port_number = atoi(argv[1]);
    }

    std::cout << "Initializing Anthracite" << std::endl;
    anthracite_socket s(port_number);
    file_backend fb(argc > 2 ? argv[2] : "./www");
    std::cout << "Initialization Complete" << std::endl;
    std::cout << "Listening for HTTP connections on port " << port_number << std::endl;

    while (true) {
        s.wait_for_conn();
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return active_threads < max_worker_threads; });
        active_threads++;
        std::thread(handle_client, s, std::ref(fb)).detach();
    }

    exit(0);
}

void log_request_and_response(http_request& req, std::unique_ptr<http_response>& resp)
{
  std::cout << "[" << resp->status_code() << " " + http_status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http_reverse_method_map.find(req.method())->second + " " + req.path() << std::endl;
}
