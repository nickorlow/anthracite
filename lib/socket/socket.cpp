#include "./socket.hpp"
#include "../log/log.hpp"
#include "assert.h"
#include <arpa/inet.h>
#include <array>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <malloc.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

namespace anthracite::socket {

socket::socket(bool nonblocking)
    : _nonblocking(nonblocking)
{
}

listener::listener(int port, int max_queue, bool nonblocking)
    : socket(nonblocking)
    , _port(port)
{
    _sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (_sock_fd == -1) {
        log::err << "Listener was unable to open a socket" << std::endl;
        throw std::exception();
    }

    struct sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);
    address.sin_addr.s_addr = INADDR_ANY;

    int reuse_opt = 1;
    if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt)) < 0) {
        log::err << "Listener was unable to set SO_REUSEADDR" << std::endl;
        throw std::exception();
    }

    if (bind(_sock_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0) {
        log::err << "Listener was unable to bind to address" << std::endl;
        throw std::exception();
    }

    if (fcntl(_sock_fd, F_SETFL, O_NONBLOCK) == -1) {
        log::err << "Listener was unable to fcntl(O_NONBLOCK)" << std::endl;
        throw std::exception();
    }

    if (listen(_sock_fd, max_queue) == -1) {
        log::err << "Listener was unable to begin listening" << std::endl;
        throw std::exception();
    }
}

bool listener::wait_for_conn(server** client_sock_p)
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

        std::string client_ip = std::string(ip_str.data());
        *client_sock_p = new server(csock, client_ip, _nonblocking);

        return true;
    } else {
        return false;
    }
}

server::server(int sock_fd, std::string client_ip, bool nonblocking)
    : _sock_fd(sock_fd)
    , _client_ip(std::move(client_ip))
    , socket(nonblocking)
{
    if (_nonblocking) {
        if (fcntl(_sock_fd, F_SETFL, O_NONBLOCK) == -1) {
            log::err << "Server was unable to fcntl(O_NONBLOCK)" << std::endl;
            throw std::exception();
        }
    }
}

void server::send_message(const std::string& msg)
{
    // Ignored because if we fail to send, it probably means
    // a HUP will occur and it'll be closed. TODO: Just close
    // it here and add a return value
    (void)send(_sock_fd, &msg[0], msg.length(), 0);
}

std::string server::recv_message(int buffer_size)
{
    // Ignored because it's nonfatal, just slower
    int nodelay_opt = 1;
    (void)setsockopt(_sock_fd, SOL_TCP, TCP_NODELAY, &nodelay_opt, sizeof(nodelay_opt));

    std::vector<char> response(buffer_size + 1);
    ssize_t result = recv(_sock_fd, response.data(), buffer_size + 1, 0);

    if (result < 1) {
        return "";
    }

    response[buffer_size] = '\0';
    return { response.data() };
}

server::~server()
{
    for (int i = 0; i < 5 && close(_sock_fd) != 0; ++i)
        ;
}

listener::~listener()
{
    for (int i = 0; i < 5 && close(_sock_fd) != 0; ++i)
        ;
}

const std::string& server::client_ip()
{
    return _client_ip;
}

};
