#include <arpa/inet.h>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace anthracite::socket {


class anthracite_socket {
    static const int MAX_QUEUE_LENGTH = 100;
private:
    int server_socket;
    int client_socket {};
    std::string client_ip;
    struct sockaddr_in client_addr {};
    socklen_t client_addr_len {};
    static constexpr struct timeval timeout_tv {
        .tv_sec = 5, .tv_usec = 0
    };

public:
    anthracite_socket(int port, int max_queue = MAX_QUEUE_LENGTH);

    void wait_for_conn();
    const std::string& get_client_ip();
    void close_conn();
    void send_message(std::string& msg);
    std::string recv_message(int buffer_size);
};

};
