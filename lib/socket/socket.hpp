#pragma once 

#include <arpa/inet.h>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace anthracite::socket {


class anthracite_socket {

protected:
    struct timeval wait_timeout = { .tv_sec = 1, .tv_usec = 0};
    int server_socket;
    int client_socket {};
    std::string client_ip;
    struct sockaddr_in client_addr {};
    socklen_t client_addr_len {};
    static const struct timeval timeout_tv;
    static const int MAX_QUEUE_LENGTH = 100;

public:
    anthracite_socket(int port, int max_queue = MAX_QUEUE_LENGTH);

    virtual const std::string& get_client_ip() final;

    virtual bool wait_for_conn();
    virtual void close_conn();
    virtual void send_message(std::string& msg);
    virtual std::string recv_message(int buffer_size);

    int csock() { return client_socket; }
};

};
