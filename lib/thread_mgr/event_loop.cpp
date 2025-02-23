#include "./event_loop.hpp"
#include "../log/log.hpp"
#include "../socket/openssl_socket.hpp"
#include "assert.h"
#include "sys/epoll.h"
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <syncstream>
#include "signal.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

namespace anthracite::thread_mgr {
event_loop::event::event(socket::anthracite_socket* socket, std::chrono::time_point<std::chrono::high_resolution_clock> timestamp)
    : _socket(socket)
    , _ts(timestamp)
{
}

socket::anthracite_socket* event_loop::event::socket()
{
    return _socket;
}

std::chrono::time_point<std::chrono::high_resolution_clock>& event_loop::event::timestamp()
{
    return _ts;
}

event_loop::event_loop(backends::backend& backend, config::config& config)
    : thread_mgr(backend, config)
    , _error_backend("./www")
{
}

bool event_loop::event_handler(socket::anthracite_socket* sock)
{
    std::string raw_request = sock->recv_message(http::HEADER_BYTES);

    if (raw_request == "") {
        return false;
    }

    http::request req(raw_request, sock->get_client_ip());
    std::unique_ptr<http::response> resp = req.is_supported_version() ? _backend.handle_request(req) : _error_backend.handle_error(http::status_codes::HTTP_VERSION_NOT_SUPPORTED);
    std::string header = resp->header_to_string();
    sock->send_message(header);
    sock->send_message(resp->content());

    if (req.close_connection()) {
        return false;
    }

    return true;
}

void event_loop::worker_thread_loop(int threadno)
{
    std::stringstream ss;
    ss << "worker " << threadno;
    pthread_setname_np(pthread_self(), ss.str().c_str());

    struct epoll_event* events = new struct epoll_event[_config.max_clients()];
    int timeout_ms = 1000;

    if (_nonblocking) {
        timeout_ms = 0;
    }

    log::info << "Starting worker thread " << threadno << std::endl;

    while (_run) {
        int ready_fds = epoll_wait(_epoll_fd, events, _config.max_clients(), timeout_ms);

        if (ready_fds > 0) {
            for (int i = 0; i < ready_fds; i++) {
                socket::anthracite_socket* sockptr = reinterpret_cast<socket::anthracite_socket*>(events[i].data.ptr);

                if (!event_handler(sockptr)) {
                    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, sockptr->csock(), &events[i]);
                    sockptr->close_conn();
                    delete sockptr;
                }
            }
        }
    }

    log::info << "Stopping worker thread " << threadno << std::endl;
}

void event_loop::listener_thread_loop(config::http_config& http_config)
{
    socket::anthracite_socket* socket;

    config::http_config* http_ptr = &http_config;
    config::https_config* https_ptr = dynamic_cast<config::https_config*>(http_ptr);

    bool is_tls = https_ptr != nullptr;

    if (is_tls) {
        socket = new socket::openssl_socket(https_ptr->port());
    } else {
        socket = new socket::anthracite_socket(http_ptr->port());
    }

    std::osyncstream(log::info) << "Listening for " << (is_tls ? "HTTPS" : "HTTP") << " connections on port " << http_ptr->port() << std::endl;

    while (_run) {
        if (socket->wait_for_conn()) {
            socket::anthracite_socket* client_sock;

            if (is_tls) {
                socket::openssl_socket* ssl_sock = dynamic_cast<socket::openssl_socket*>(socket);
                client_sock = new socket::openssl_socket(*ssl_sock);
            } else {
                client_sock = new socket::anthracite_socket(*socket);
            }

            struct epoll_event event;
            event.events = EPOLLIN | EPOLLEXCLUSIVE;// | EPOLLET;
            event.data.ptr = client_sock;
            epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_sock->csock(), &event);
        }
    }

    std::osyncstream(log::info) << "Stopping listening for " << (is_tls ? "HTTPS" : "HTTP") << " connections on port " << http_ptr->port() << std::endl;

    delete socket;
}

void event_loop::start()
{
    signal(SIGPIPE, SIG_IGN);
    log::info << "Starting event_loop Thread Manager" << std::endl;

    _run = true;
    _epoll_fd = epoll_create(1);

    std::vector<std::thread> listener_threads;
    std::vector<std::thread> worker_threads;

    for (int i = 0; i < _config.worker_threads(); i++) {
        auto thread = std::thread(&event_loop::worker_thread_loop, this, i);
        worker_threads.push_back(std::move(thread));
    }

    if (_config.http_cfg().has_value()) {
        auto thread = std::thread(&event_loop::listener_thread_loop, this, std::ref(_config.http_cfg().value()));
        listener_threads.push_back(std::move(thread));
    }

    if (_config.https_cfg().has_value()) {
        auto thread = std::thread(&event_loop::listener_thread_loop, this, std::ref(_config.https_cfg().value()));
        listener_threads.push_back(std::move(thread));
    }

    for (std::thread& t : worker_threads) {
        t.join();
    }

    for (std::thread& t : listener_threads) {
        t.join();
    }
}

void event_loop::stop()
{
    _run = false;
}
}
