#include "./event_loop.hpp"
#include "../log/log.hpp"
#include "../socket/openssl_socket.hpp"
#include <mutex>
#include <chrono>
#include <syncstream>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

namespace anthracite::thread_mgr {
    event_loop::event::event(socket::anthracite_socket* socket) :
                    _socket(socket) {}

    socket::anthracite_socket* event_loop::event::socket() {
        return _socket;
    }

    event_loop::event_loop(backends::backend& backend, config::config& config) : thread_mgr(backend, config), _error_backend("./www") {}

    bool event_loop::event_handler(event& event) {
        std::string raw_request = event.socket()->recv_message(http::HEADER_BYTES);
        
        // We're doing the start here even though it would ideally be done 
        // before the first line since if we leave the connection open for 
        // HTTP 1.1, we can spend a bit of time waiting
        auto start = high_resolution_clock::now();
        
        if (raw_request == "") {
            event.socket()->close_conn();
            delete event.socket();
            return false;
        }
        
        http::request req(raw_request, event.socket()->get_client_ip());
        std::unique_ptr<http::response> resp = req.is_supported_version() ? _backend.handle_request(req) : _error_backend.handle_error(http::status_codes::HTTP_VERSION_NOT_SUPPORTED);
        std::string header = resp->header_to_string();
        event.socket()->send_message(header);
        event.socket()->send_message(resp->content());
        
        auto end = high_resolution_clock::now();
        auto ms_int = duration_cast<std::chrono::microseconds>(end-start);
        //log_request_and_response(req, resp , ms_int.count());
        
        resp.reset();
        if (req.close_connection()) {
            event.socket()->close_conn();
            delete event.socket();
            return false;
        }

        return true;
    }

    void event_loop::worker_thread_loop(int threadno) {
        unsigned char buf[sizeof(class event)];

        std::osyncstream(log::info) << "Starting worker thread " << threadno << std::endl;
        while(_run) {
            // Get event from queue
            std::unique_lock lock(_event_mtx);


            event* ev = nullptr;

            if (_events.size() > 0) {
                ev = new (buf) event(_events.back());
                _events.pop();
                lock.unlock();
            } else {
                _event_cv.wait(lock, [this]{ return this->_events.size() > 0 || !_run; });

                if (!_run) {
                    break;
                }

                ev = new (buf) event(_events.back());
                _events.pop();
                lock.unlock();
            }


            // process
            bool requeue = event_handler(*ev);

            // if necessary, requeue
            if (requeue) {
                {
                    std::lock_guard<std::mutex> lg(_event_mtx);
                    _events.push(*ev);
                }
                _event_cv.notify_one();
            }
        }

        std::osyncstream(log::info) << "Stopping worker thread " << threadno << std::endl;
    }

    void event_loop::listener_thread_loop(config::http_config& http_config) {
        socket::anthracite_socket* socket;

        config::http_config* http_ptr = &http_config;
        config::https_config* https_ptr = dynamic_cast<config::https_config*>(http_ptr);

        bool is_tls = https_ptr != nullptr;

        if (is_tls){
            socket = new socket::openssl_socket(https_ptr->port());
        } else {
            socket = new socket::anthracite_socket(http_ptr->port());
        }

        std::osyncstream(log::info) << "Listening for " << (is_tls ? "HTTPS" : "HTTP" ) << " connections on port " << http_ptr->port() << std::endl;
         
        while (_run) {
            if(socket->wait_for_conn()) {
                socket::anthracite_socket* client_sock;
        
                if (is_tls){
                    socket::openssl_socket* ssl_sock = dynamic_cast<socket::openssl_socket*>(socket);
                    client_sock = new socket::openssl_socket(*ssl_sock);
                } else {
                    client_sock = new socket::anthracite_socket(*socket);
                }

                std::lock_guard<std::mutex> lg(_event_mtx);
                _events.push(event(client_sock));
                _event_cv.notify_one();
            }
        }

        std::osyncstream(log::info) << "Stopping listening for " << (is_tls ? "HTTPS" : "HTTP") << " connections on port " << http_ptr->port() << std::endl;

        delete socket;
    }

    void event_loop::start() {
        log::info << "Starting event_loop Thread Manager" << std::endl;

        _run = true;

        std::vector<std::thread> listener_threads;
        std::vector<std::thread> worker_threads;
        
        for(int i = 0; i < _config.worker_threads(); i++) {
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

        for(std::thread& t : worker_threads) {
            t.join();
        }
        
        for(std::thread& t : listener_threads) {
            t.join();
        }
    }

    void event_loop::stop() {
        _run = false;
        std::lock_guard<std::mutex> lg(_event_mtx);
        _event_cv.notify_all();
    }
}
