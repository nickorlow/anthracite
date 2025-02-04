#pragma once

#include <memory>
#include "../http/request.hpp"
#include "../http/response.hpp"

namespace anthracite::backends {

class backend {
public:
  backend() = default;
  virtual ~backend() = default;
  backend(backend const&) = delete;
  backend& operator = (backend const&) = delete;
  backend(backend&&) = delete;
  backend& operator=(backend&&) = delete;
  virtual std::unique_ptr<http::response> handle_request(http::request& req) = 0;
};

};
