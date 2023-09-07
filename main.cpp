#include "backends/file_backend.cpp"
#include <exception>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

void log_request_an_response(http_request req, http_response resp);

constexpr int default_port = 80;

int main(int argc, char** argv)
{
    int port_number = default_port;

    if (argc > 1) {
        port_number = atoi(argv[1]);
    }

    cout << "Initializing Anthracite\n";
    anthracite_socket s(port_number);
    cout << "Initialization Complete\n";
    cout << "Listening for HTTP connections on port " << port_number << "\n";
    file_backend fb;

    while (true) {
        s.wait_for_conn();
        http_request req(s);
        http_response resp = fb.handle_request(req);
        log_request_an_response(req, resp);
        s.send_message(resp.to_string());
        s.close_conn();
    }

    return 0;
}

void log_request_an_response(http_request req, http_response resp)
{
    cout << "[" << resp.status_code() << " " + http_status_map.find(resp.status_code())->second + "] " + req.client_ip() + " " + http_reverse_method_map.find(req.method())->second + " " + req.path() + "\n";
}
