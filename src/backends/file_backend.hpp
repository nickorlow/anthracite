#pragma once

#include "backend.hpp"

namespace anthracite::backends {

class file_backend : public backend {
private:
  std::unordered_map<std::string,std::string> file_cache;
  std::string file_dir;
  
  std::unique_ptr<http::response> handle_request_cache(http::request& req);
  void populate_cache_dir(std::string dir);
  void populate_cache();
public:
  file_backend(std::string dir = "./www") ;

  std::unique_ptr<http::response> handle_request(http::request& req) override;
  std::unique_ptr<http::response> handle_error(const http::status_codes& error);
};

};
