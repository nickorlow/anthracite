#include <exception>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

class Socket {
private:
  int server_socket;
  int client_socket;

public:
  Socket(int port, int max_queue = 10) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int x = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
    bind(server_socket, (struct sockaddr *)&address, sizeof(address));

    ::listen(server_socket, max_queue);
  }

  void wait_for_conn() { client_socket = accept(server_socket, NULL, NULL); }

  void close_conn() {
    close(client_socket);
    client_socket = -1;
  }

  void send_message(string msg) {
    if (client_socket == -1) {
      return;
    }
    send(client_socket, &msg[0], msg.length(), 0);
  }

  string recv_message(int buffer_size = 1024) {
    if (client_socket == -1) {
      return "";
    }

    char response[buffer_size + 1];
    recv(client_socket, response, sizeof(response), 0);
    response[buffer_size] = '\0';
    return string(response);
  }
};

enum http_method { GET, POST, PUT, PATCH, UNKNOWN };
static unordered_map<string, http_method> const http_method_map = {
    {"GET", http_method::GET}};

static unordered_map<int, string> const http_status_map = {
    {200, "OK"},
    {404, "NOT FOUND"},
};

class http_request {
private:
  http_method _method;
  string _path;

public:
  http_request(string raw_data) {
    int state = 0;
    string method = "";
    for (int i = 0; i < raw_data.length() && state != 2; i++) {

      if (raw_data[i] == ' ') {
        state++;
        continue;
      }

      if (state == 0) {
        method += raw_data[i];
      } else if (state == 1) {
        _path += raw_data[i];
      }
    }

    if (http_method_map.find(method) == http_method_map.end()) {
      _method = http_method::UNKNOWN;
    } else {
      _method = http_method_map.find(method)->second;
    }
  }

  string path() { return _path; }

  http_method method() { return _method; }
};

class http_response {
private:
  int _status_code;
  string _content;

public:
  http_response(string content, int status_code = 200) {
    _content = content;
    _status_code = status_code;
  }

  string to_string() {
    string response = "";
    response += "HTTP/1.1 " + ::to_string(_status_code) + " " +
                http_status_map.find(_status_code)->second + " \n";
    response += "Content-Type: text/html\n";
    response += "Content-Length: " + ::to_string(_content.length()) + "\n";
    response += "Server: Anthracite/0.0.1\n\n";
    response += _content;
    return response;
  }
};

int main() {
  cout << "Initializing Server...\n";
  Socket s(8099);
  cout << "Initialized, Awaiting Connections...\n";

  while (true) {
    s.wait_for_conn();
    cout << "Received Request...\n";
    http_request req(s.recv_message(8190));

    string response = "";

    switch (req.method()) {
    case http_method::GET: {
      string filename = req.path() == "/" ? "index.html" : req.path();
      filename = "./www/" + filename;
      ifstream stream(filename);

      int status = 200;
      if (!stream.is_open()) {
        status = 404;
        filename = "./error_pages/404.html";
        stream = ifstream(filename);
      }

      stringstream buffer;
      buffer << stream.rdbuf();
      http_response res(buffer.str(), status);
      response = res.to_string();
    } break;

    default:
      break;
    }
    cout << "Responding with: \n" << response << "\n";
    s.send_message(response);
    s.close_conn();
  }

  return 0;
}
