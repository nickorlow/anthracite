#include "./thread_mgr.hpp"
#include "../socket/socket.hpp"
#include "../backends/file_backend.hpp"
#include <mutex>
#include <vector>
#include "../socket/socket.hpp"

namespace anthracite::thread_mgr {
    class event_loop : public virtual thread_mgr {
            std::mutex _event_mtx;
            backends::file_backend _error_backend;
            std::vector<socket::listener*>& _listen_sockets;
            bool _nonblocking;
            std::mutex _run_lock;
            int _max_threads;
            int _max_clients;

            void worker_thread_loop(int threadno);
            bool event_handler(socket::server*);

        public:
            event_loop(std::vector<socket::listener*>&, backends::backend& backend, int max_workers, int max_clients);
            void start() override;
            void stop() override;
    };
};
