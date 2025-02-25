#include "./event_loop.hpp"
#include "../log/log.hpp"
#include "assert.h"
#include "signal.h"
#include "sys/epoll.h"
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <syncstream>
#include <thread>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

namespace anthracite::thread_mgr {

event_loop::event_loop(std::vector<socket::listener*>& listen_sockets, backends::backend& backend, int max_threads, int max_clients)
    : thread_mgr(backend)
    , _error_backend("./www")
    , _max_threads(max_threads)
    , _listen_sockets(listen_sockets)
    , _max_clients(max_clients)
    , _nonblocking(false)
{
}

bool event_loop::event_handler(socket::server* sock)
{
    std::string raw_request = sock->recv_message(http::HEADER_BYTES);

    if (raw_request == "") {
        return false;
    }

    http::request req(raw_request, sock->client_ip());
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

    struct epoll_event* events = new struct epoll_event[_max_clients];
    int timeout_ms = 1000;

    if (_nonblocking) {
        timeout_ms = 0;
    }

    std::osyncstream(log::info) << "Starting worker thread " << threadno << std::endl;

    int epoll_fd = epoll_create(1);

    for (socket::listener* sl : _listen_sockets) {
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLEXCLUSIVE;
        event.data.ptr = sl;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sl->fd(), &event);
        if (threadno == 0) {
            std::osyncstream(log::info) << "Listening started on port " << sl->port() << std::endl;
        }
    }

    while (_run) {
        int ready_fds = epoll_wait(epoll_fd, events, _max_clients, timeout_ms);

        if (ready_fds > 0) {
            for (int i = 0; i < ready_fds; i++) {
                socket::socket* sockptr = reinterpret_cast<socket::socket*>(events[i].data.ptr);
                socket::server* server_ptr = dynamic_cast<socket::server*>(sockptr);

                if (server_ptr != nullptr) {
                    if (!event_handler(server_ptr)) {
                        delete server_ptr;
                    }
                } else {
                    socket::listener* listen_ptr = dynamic_cast<socket::listener*>(sockptr);
                    if (listen_ptr != nullptr) {
                        socket::server* server_sock;
                        while (listen_ptr->wait_for_conn(&server_sock)) {
                            struct epoll_event event;
                            event.events = EPOLLIN | EPOLLEXCLUSIVE;
                            event.data.ptr = server_sock;
                            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock->fd(), &event);
                        }
                    } else {
                        std::osyncstream(log::err) << "Had socket type that wasn't listener or server" << std::endl;
                    }
                }
            }
        }
    }

    delete[] events;

    std::osyncstream(log::info) << "Stopping worker thread " << threadno << std::endl;
}

void event_loop::start()
{
    std::lock_guard<std::mutex> lg(_run_lock);

    signal(SIGPIPE, SIG_IGN);
    log::info << "Starting event_loop Thread Manager" << std::endl;

    _run = true;

    std::vector<std::thread> worker_threads;

    for (int i = 0; i < _max_threads; i++) {
        auto thread = std::thread(&event_loop::worker_thread_loop, this, i);
        worker_threads.push_back(std::move(thread));
    }

    for (std::thread& t : worker_threads) {
        t.join();
    }
}

void event_loop::stop()
{
    _run = false;
}
}
