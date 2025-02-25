#pragma once

#include "./socket.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace anthracite::socket {
class openssl_server : public server{
    private:
        SSL* _ssl;
    public:
        openssl_server(int sock_fd, std::string client_ip, bool nonblocking, SSL* ssl);
        ~openssl_server();

        void send_message(const std::string& msg) override;
        std::string recv_message(int buffer_size) override;
};

class openssl_listener : public listener {
    private:
        SSL_CTX* _context;

    public:
        openssl_listener(std::string& key_path, std::string& cert_path, int port, int max_queue_length, bool nonblocking);
        ~openssl_listener();

        bool wait_for_conn(server** client_sock_) override;
};
};
