#include "./socket.hpp"
#include <arpa/inet.h>
#include <array>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "assert.h"
#include <netinet/in.h>
#include <netinet/tcp.h>


namespace anthracite::socket {

const struct timeval anthracite_socket::timeout_tv = { .tv_sec = 5, .tv_usec = 0 };

anthracite_socket::anthracite_socket(int port, int max_queue, bool nonblocking)
    : server_socket(::socket(AF_INET, SOCK_STREAM, 0))
    , client_ip("")
    , _nonblocking(nonblocking)
{
    struct sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    int reuse_opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt));
    bind(server_socket, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));

    if (_nonblocking) {
        fcntl(server_socket, F_SETFL, O_NONBLOCK);
    }

    listen(server_socket, max_queue);
}

bool anthracite_socket::wait_for_conn()
{
    client_ip = "";
    client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    if (client_socket > 0) {
        if (_nonblocking) {
            fcntl(client_socket, F_SETFL, O_NONBLOCK);
        }
        std::array<char, INET_ADDRSTRLEN> ip_str { 0 };
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str.data(), INET_ADDRSTRLEN);
        client_ip = std::string(ip_str.data());
        return true;
    } else {
        return false;
    }
}

const std::string& anthracite_socket::get_client_ip()
{
    return client_ip;
}

void anthracite_socket::close_conn() {
    close(client_socket);
    client_socket = -1;
}

void anthracite_socket::send_message(std::string& msg)
{
    if (client_socket == -1) {
        return;
    }
    send(client_socket, &msg[0], msg.length(), 0);
}

bool anthracite_socket::has_client() {
    return client_socket > 0;
}

std::string anthracite_socket::recv_message(int buffer_size)
{
    if (client_socket == -1) {
        return "";
    }

    //setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof timeout_tv);

    int nodelay_opt = 1;
    assert(setsockopt(client_socket, SOL_TCP, TCP_NODELAY, &nodelay_opt, sizeof(nodelay_opt)) == 0);

    std::vector<char> response(buffer_size + 1);
    ssize_t result = recv(client_socket, response.data(), buffer_size + 1, 0);

    if (result < 1) {
        return "";
    }

    response[buffer_size] = '\0';
    return { response.data() };
}

};
