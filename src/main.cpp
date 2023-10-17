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

using namespace std;

void log_request_and_response(http_request& req, unique_ptr<http_response>& resp);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

int active_threads = 0;
mutex mtx;
condition_variable cv;

void handle_client(anthracite_socket s, file_backend& fb)
{
    http_request req(s);
    unique_ptr<http_response> resp = fb.handle_request(req);
    log_request_and_response(req, resp);
    string header = resp->header_to_string();
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

    cout << "Initializing Anthracite" << endl;
    anthracite_socket s(port_number);
    file_backend fb(argc > 2 ? argv[2] : "./www");
    cout << "Initialization Complete" << endl;
    cout << "Listening for HTTP connections on port " << port_number << endl;

    while (true) {
        s.wait_for_conn();
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return active_threads < max_worker_threads; });
        active_threads++;
        thread(handle_client, s, ref(fb)).detach();
    }

    exit(0);
}

void log_request_and_response(http_request& req, unique_ptr<http_response>& resp)
{
    cout << "[" << resp->status_code() << " " + http_status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http_reverse_method_map.find(req.method())->second + " " + req.path() << endl;
}
