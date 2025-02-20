#include "./anthracite.hpp"
#include "./log/log.hpp"
#include "./socket/socket.hpp"
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
#include "./socket/openssl_socket.hpp"
#include <future>

using namespace anthracite;

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros);

constexpr int default_port = 80;
constexpr int max_worker_threads = 128;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

void handle_client(socket::anthracite_socket* s, backends::backend& b, backends::file_backend& fb, std::mutex& thread_wait_mutex, std::condition_variable& thread_wait_condvar, int& active_threads)
{
    while (true) {
        std::string raw_request = s->recv_message(http::HEADER_BYTES);
        
        // We're doing the start here even though it would ideally be done 
        // before the first line since if we leave the connection open for 
        // HTTP 1.1, we can spend a bit of time waiting
        auto start = high_resolution_clock::now();

        if (raw_request == "") {
            break;
        }

        http::request req(raw_request, s->get_client_ip());
        std::unique_ptr<http::response> resp = req.is_supported_version() ? b.handle_request(req) : fb.handle_error(http::status_codes::HTTP_VERSION_NOT_SUPPORTED);
        std::string header = resp->header_to_string();
        s->send_message(header);
        s->send_message(resp->content());

        auto end = high_resolution_clock::now();
        auto ms_int = duration_cast<std::chrono::microseconds>(end-start);
        log_request_and_response(req, resp , ms_int.count());
        
        resp.reset();
        if (req.close_connection()) {
            break;
        }
    }
    s->close_conn();
    delete s;
    {
        std::lock_guard<std::mutex> lock(thread_wait_mutex);
        active_threads--;
    }
    thread_wait_condvar.notify_one();
}

int listen_loop(int port_number, backends::backend& be, bool tls) {
    socket::anthracite_socket* socket;

    if (tls){
        socket = new socket::openssl_socket(port_number);
    } else {
        socket = new socket::anthracite_socket(port_number);
    }

    backends::file_backend fb("./www");
    log::info << "Listening for " << (tls ? "HTTPS" : "HTTP" ) << " connections on port " << port_number << std::endl;
     
    int active_threads = 0;
    std::mutex thread_wait_mutex;
    std::condition_variable thread_wait_condvar;

    while (true) {
        socket->wait_for_conn();
        std::unique_lock<std::mutex> lock(thread_wait_mutex);
        thread_wait_condvar.wait(lock, [active_threads] { return active_threads < max_worker_threads; });
        active_threads++;

        socket::anthracite_socket* client_sock;
    
        if (tls){
            socket::openssl_socket* ssl_sock = dynamic_cast<socket::openssl_socket*>(socket);
            client_sock = new socket::openssl_socket(*ssl_sock);
        } else {
            client_sock = new socket::anthracite_socket(*socket);
        }

        std::thread(handle_client, client_sock, std::ref(be), std::ref(fb), std::ref(thread_wait_mutex), std::ref(thread_wait_condvar), std::ref(active_threads)).detach();
    }

    delete socket;
}

int anthracite_main(int argc, char** argv, backends::backend& be)
{
    log::logger.initialize(log::LOG_LEVEL_INFO);
    auto args = std::span(argv, size_t(argc));

    std::vector<std::thread> threads;

    if (argc > 1) {
        auto thread = std::thread(listen_loop, atoi(argv[1]), std::ref(be), false);
        thread.detach();
        threads.push_back(std::move(thread));
    } if (argc > 2) {
        auto thread = std::thread(listen_loop, atoi(argv[2]), std::ref(be), true);
        thread.detach();
        threads.push_back(std::move(thread));
    }
    std::promise<void>().get_future().wait();
    return 0;
}

void log_request_and_response(http::request& req, std::unique_ptr<http::response>& resp, uint32_t micros)
{
    log::info << "[" << resp->status_code() << " " + http::status_map.find(resp->status_code())->second + "] " + req.client_ip() + " " + http::reverse_method_map.find(req.get_method())->second + " " + req.path() << " in " << micros << " usecs" << std::endl;
}
