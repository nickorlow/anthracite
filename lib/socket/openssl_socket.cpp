#include "./openssl_socket.hpp"
#include "../log/log.hpp"
#include <arpa/inet.h>
#include <array>
#include <exception>
#include <iostream>
#include <malloc.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

namespace anthracite::socket {

openssl_listener::openssl_listener(std::string& key_path, std::string& cert_path, int port, int max_queue, bool nonblocking)
    : listener(port, max_queue, nonblocking)
{
    const SSL_METHOD* method = TLS_server_method();

    _context = SSL_CTX_new(method);

    if (!_context) {
        log::err << "Unable to initialize SSL" << std::endl;
        throw std::exception();
    }

    if (SSL_CTX_use_certificate_file(_context, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        log::err << "Unable to open Cert file at: " << cert_path << std::endl;
        throw std::exception();
    }

    if (SSL_CTX_use_PrivateKey_file(_context, key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        log::err << "Unable to open Key file at: " << key_path << std::endl;
        throw std::exception();
    }
}

bool openssl_listener::wait_for_conn(server** client_sock_p)
{
    struct sockaddr_in client_addr {};
    socklen_t client_addr_len;

    int csock = accept(_sock_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);

    if (csock > 0) {
        std::array<char, INET6_ADDRSTRLEN> ip_str { 0 };

        if (inet_ntop(AF_INET, &client_addr.sin_addr, ip_str.data(), INET_ADDRSTRLEN) == NULL) {
            if (inet_ntop(AF_INET6, &client_addr.sin_addr, ip_str.data(), INET6_ADDRSTRLEN) == NULL) {
                log::warn << "Unable to decode client's IP address" << std::endl;
            }
        }

        SSL* ssl = SSL_new(_context);

        if (ssl == NULL) {
            for (int i = 0; i < 5 && close(csock) != 0; ++i)
                ;
            return false;
        }

        if (SSL_set_fd(ssl, csock) == 0) {
            SSL_free(ssl);
            for (int i = 0; i < 5 && close(csock) != 0; ++i)
                ;
            return false;
        }

        if (SSL_accept(ssl) <= 0) {
            log::warn << "Unable to open SSL connection with client" << std::endl;
            SSL_free(ssl);
            for (int i = 0; i < 5 && close(csock) != 0; ++i)
                ;
            return false;
        }

        std::string client_ip = std::string(ip_str.data());
        *client_sock_p = new openssl_server(csock, client_ip, _nonblocking, ssl);
        return true;
    } else {
        return false;
    }
}

openssl_listener::~openssl_listener() {}

openssl_server::openssl_server(int sock_fd, std::string client_ip, bool nonblocking, SSL* ssl)
    : server(sock_fd, client_ip, nonblocking)
    , _ssl(ssl)
{
}

openssl_server::~openssl_server()
{
    SSL_shutdown(_ssl);
    SSL_free(_ssl);
}

void openssl_server::send_message(const std::string& msg)
{
    SSL_write(_ssl, &msg[0], msg.length());
}

std::string openssl_server::recv_message(int buffer_size)
{
    // Ignored because it's nonfatal, just slower
    int nodelay_opt = 1;
    (void)setsockopt(_sock_fd, SOL_TCP, TCP_NODELAY, &nodelay_opt, sizeof(nodelay_opt));

    std::vector<char> response(buffer_size + 1);
    ssize_t result = SSL_read(_ssl, response.data(), buffer_size + 1);

    if (result < 1) {
        return "";
    }

    response[buffer_size] = '\0';
    return { response.data() };
}

};
