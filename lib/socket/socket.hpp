#pragma once 

#include <arpa/inet.h>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace anthracite::socket {

class socket {
    protected:
        bool _nonblocking;
        socket(bool nonblocking);
    public:
        socket(){}
        virtual ~socket(){}
};

class server : public socket {
    protected:
        int _sock_fd;
        std::string _client_ip;
    public:
        server(int sock_fd, std::string client_ip, bool nonblocking);
        ~server();

        virtual void send_message(const std::string& msg);
        virtual std::string recv_message(int buffer_size);
        const std::string& client_ip();

        int fd() { return _sock_fd; }
};

class listener : public socket {
    protected:
        uint16_t _port;
        int _sock_fd;
    public:
        listener(int port, int max_queue_length, bool nonblocking);
        ~listener();

        virtual bool wait_for_conn(server** client_sock_p);

        int fd() { return _sock_fd; }
        int port() { return _port; }
};

};
