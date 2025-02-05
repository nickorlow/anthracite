#include "./tls_socket.hpp"
#include <arpa/inet.h>
#include <array>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include "../log/log.hpp"


namespace anthracite::socket {

tls_socket::tls_socket(int port, int max_queue) : anthracite_socket(port, max_queue), _handshakeDone(false) {
}

void tls_socket::wait_for_conn()
{
    client_ip = "";
    client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    std::array<char, INET_ADDRSTRLEN> ip_str { 0 };
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str.data(), INET_ADDRSTRLEN);
    client_ip = std::string(ip_str.data());
}

void tls_socket::close_conn()
{
    close(client_socket);
    client_socket = -1;
}

void tls_socket::send_message(std::string& msg)
{
    if (client_socket == -1) {
        return;
    }
    send(client_socket, &msg[0], msg.length(), 0);
}

void tls_socket::perform_handshake() {
    struct tls_msg_hdr hdr{};
    ssize_t result = recv(client_socket, &hdr, sizeof(hdr), 0);

    if (result < 1) {
        return;
    }

    log::info << "MsgType " << unsigned(hdr.msg_type);
    log::info << " MsgLen " << hdr.length << std::endl;
    
    char hhdr[4];
    result = recv(client_socket, &hhdr, sizeof(hhdr), 0);

    if (result < 1) {
        return;
    }

    uint16_t msg_size = ClientHello::deserialize_uint16(hhdr + 2);

    log::debug << "TLS ClientHello Size: " << msg_size << std::endl;


    char* client_hello_data = (char*) malloc(msg_size);

    result = recv(client_socket, client_hello_data, msg_size, 0);

    std::cout << result << " Bytes rxd" << std::endl;

    ClientHello hello_msg(client_hello_data, result);

    char *ptr;
    ServerHello hello_retmsg(hello_msg.session_id);
    int size = hello_retmsg.get_buf(&ptr);
    log::debug << "Sending message of length " << size << std::endl;
    send(client_socket, ptr , size, 0);
    for(;;){}
    _handshakeDone = true;
}

std::string tls_socket::recv_message(int buffer_size)
{
    if (client_socket == -1) {
        return "";
    }

    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof timeout_tv);

    if (!_handshakeDone) {
        perform_handshake();
        return "";
    }

    std::vector<char> response(buffer_size + 1);
    ssize_t result = recv(client_socket, response.data(), buffer_size + 1, 0);

    if (result < 1) {
        return "";
    }

    response[buffer_size] = '\0';
    return { response.data() };
}

};
