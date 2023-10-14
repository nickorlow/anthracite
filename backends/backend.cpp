#include "../http.cpp"

class backend {
public:
  virtual http_response handle_request(http_request req) = 0;
};
