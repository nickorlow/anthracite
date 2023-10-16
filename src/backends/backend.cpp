#include "../http.cpp"
#include <memory>

class backend {
public:
  virtual unique_ptr<http_response> handle_request(http_request& req) = 0;
};
