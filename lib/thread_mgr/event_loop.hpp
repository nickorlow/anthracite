#include "./thread_mgr.hpp"
#include "../socket/socket.hpp"
#include "../backends/file_backend.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace anthracite::thread_mgr {
    class event_loop : public virtual thread_mgr {
            class event {
                socket::anthracite_socket* _socket;
                std::chrono::time_point<std::chrono::high_resolution_clock> _ts; 
                public:
                    event(socket::anthracite_socket* socket, std::chrono::time_point<std::chrono::high_resolution_clock> timestamp);
                    socket::anthracite_socket* socket();
                    std::chrono::time_point<std::chrono::high_resolution_clock>& timestamp();    
            };

            std::vector<int> _epoll_fds;
            std::mutex _event_mtx;
            std::condition_variable _event_cv;
            std::queue<event> _events;
            backends::file_backend _error_backend;

            void worker_thread_loop(int threadno);
            void listener_thread_loop(config::http_config& http_config);
            void eventer_thread_loop();
            bool event_handler(socket::anthracite_socket*);

        public:
            event_loop(backends::backend& backend, config::config& config);
            void start() override;
            void stop() override;
    };
};
