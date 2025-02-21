#pragma once

#include "./socket.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace anthracite::socket {
class openssl_socket : public anthracite_socket {
    private:
        static SSL_CTX* _context;
        SSL* _ssl;

    public:
        openssl_socket(int port, int max_queue = MAX_QUEUE_LENGTH);
        ~openssl_socket();

        bool wait_for_conn() override;
        void close_conn() override;
        void send_message(std::string& msg) override;
        std::string recv_message(int buffer_size) override;
};
};
