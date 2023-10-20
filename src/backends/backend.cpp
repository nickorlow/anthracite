#include <memory>

#include "../http/http_request.cpp"
#include "../http/http_response.cpp"

class backend {
public:
  backend() = default;
  virtual ~backend() = default;
  backend(backend const&) = delete;
  backend& operator = (backend const&) = delete;
  backend(backend&&) = delete;
  backend& operator=(backend&&) = delete;
  virtual std::unique_ptr<http_response> handle_request(http_request& req) = 0;
};
