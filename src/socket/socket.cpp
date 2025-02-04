#include <arpa/inet.h>
#include <array>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

namespace anthracite::socket {

constexpr int MAX_QUEUE_LENGTH = 100;

class anthracite_socket {
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
    anthracite_socket(int port, int max_queue = MAX_QUEUE_LENGTH)
        : server_socket(::socket(AF_INET, SOCK_STREAM, 0))
        , client_ip("")
    {
        struct sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;

        int reuse_opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt));
        bind(server_socket, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));

        listen(server_socket, max_queue);
    }

    void wait_for_conn()
    {
        client_ip = "";
        client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
        std::array<char, INET_ADDRSTRLEN> ip_str { 0 };
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str.data(), INET_ADDRSTRLEN);
        client_ip = std::string(ip_str.data());
    }

    const std::string& get_client_ip()
    {
        return client_ip;
    }

    void close_conn()
    {
        close(client_socket);
        client_socket = -1;
    }

    void send_message(std::string& msg)
    {
        if (client_socket == -1) {
            return;
        }
        send(client_socket, &msg[0], msg.length(), 0);
    }

    std::string recv_message(int buffer_size)
    {
        if (client_socket == -1) {
            return "";
        }

        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof timeout_tv);
        std::vector<char> response(buffer_size + 1);
        ssize_t result = recv(client_socket, response.data(), buffer_size + 1, 0);

        if (result < 1) {
            return "";
        }

        response[buffer_size] = '\0';
        return { response.data() };
    }
};

};
