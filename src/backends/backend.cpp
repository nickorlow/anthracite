#include <memory>

#include "../http/http_request.cpp"
#include "../http/http_response.cpp"

class backend {
public:
  virtual std::unique_ptr<http_response> handle_request(http_request& req) = 0;
};
