#include <exception>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "./openssl_socket.hpp"
#include <arpa/inet.h>
#include <array>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "../log/log.hpp"

namespace anthracite::socket {

SSL_CTX* openssl_socket::_context = nullptr;

openssl_socket::openssl_socket(int port, int max_queue)
    : anthracite_socket(port, max_queue)
{
    const SSL_METHOD *method = TLS_server_method();

    if (_context == nullptr) {
        _context = SSL_CTX_new(method);
    }

    if (!_context) {
        log::err << "Unable to initialize SSL" << std::endl;
        throw std::exception();
    }

    if (SSL_CTX_use_certificate_file(_context, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        log::err << "Unable to open cert.pem" << std::endl;
        throw std::exception();
    }

    if (SSL_CTX_use_PrivateKey_file(_context, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        log::err << "Unable to open key.pem" << std::endl;
        throw std::exception();
    }
}

openssl_socket::~openssl_socket() = default;

void openssl_socket::wait_for_conn()
{
    client_ip = "";
    client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    std::array<char, INET_ADDRSTRLEN> ip_str { 0 };
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str.data(), INET_ADDRSTRLEN);
    client_ip = std::string(ip_str.data());
 
    _ssl = SSL_new(_context);
    SSL_set_fd(_ssl, client_socket);
    if (SSL_accept(_ssl) <= 0) {
        log::warn << "Unable to open SSL connection with client" << std::endl;
        client_ip = "";
        close(client_socket);
        client_socket = -1;
    }
}

void openssl_socket::close_conn()
{
    SSL_shutdown(_ssl);
    SSL_free(_ssl);
    close(client_socket);
    client_socket = -1;
}

void openssl_socket::send_message(std::string& msg)
{
    if (client_socket == -1) {
        return;
    }
    SSL_write(_ssl, &msg[0], msg.length());
}

std::string openssl_socket::recv_message(int buffer_size)
{
    if (client_socket == -1) {
        return "";
    }

    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof timeout_tv);
    std::vector<char> response(buffer_size + 1);
    ssize_t result = SSL_read(_ssl, response.data(), buffer_size+1);

    if (result < 1) {
        return "";
    }

    response[buffer_size] = '\0';
    return { response.data() };
}

};
