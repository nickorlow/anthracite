#include "./file_backend.hpp"
#include "../log/log.hpp"
#include <fstream>
#include <filesystem>


namespace anthracite::backends {

  
  std::unique_ptr<http::response> file_backend::handle_request_cache(http::request& req) {
     std::string filename = req.path() == "/" ? "/index.html" : req.path();
     filename = file_dir + filename;
     auto file_info = file_cache.find(filename);

     int status = http::status_codes::OK;
     if (file_info == file_cache.end()) {
       return handle_error(http::status_codes::NOT_FOUND);
     }

     std::unique_ptr<http::response> resp = std::make_unique<http::response>();
        
     std::string file_extension = file_info->first.substr(file_info->first.rfind('.') + 1);
     std::string content_type = "text/html";

     auto mime_type = http::mime_types.find(file_extension);
     if (mime_type != http::mime_types.end()) {
         content_type = mime_type->second;
     }

     resp->add_body_ref(&file_info->second);
     resp->add_status(http::status_codes::OK);
     resp->add_header(http::header("Content-Type", content_type), false);

     return resp;
  }

  void file_backend::populate_cache_dir(std::string dir) {
    std::filesystem::recursive_directory_iterator cur = begin(std::filesystem::recursive_directory_iterator(dir));
    std::filesystem::recursive_directory_iterator fin = end(std::filesystem::recursive_directory_iterator(dir));

    while (cur != fin) {
        auto p = cur->path();
        std::string filename = p.string();
        std::stringstream buffer;
        std::ifstream stream(filename);
        buffer << stream.rdbuf();
        file_cache[filename] = buffer.str();
        log::verbose << "File at " << filename << " cached (" << file_cache[filename].size() << " bytes)" << std::endl;
        ++cur;
    }
  }

  void file_backend::populate_cache() {
    populate_cache_dir(file_dir);
    populate_cache_dir("./error_pages/");
  }

  file_backend::file_backend(std::string dir) : file_dir(std::move(dir)) {
    populate_cache();
  }


  std::unique_ptr<http::response> file_backend::handle_request(http::request& req) {
    return handle_request_cache(req);
  }
  
  std::unique_ptr<http::response> file_backend::handle_error(const http::status_codes& error) {
     std::string filename = "./error_pages/" + std::to_string(error) + ".html";
     auto file_info = file_cache.find(filename);

     http::status_codes status = error;
     if (file_info == file_cache.end()) {
         status = http::status_codes::NOT_FOUND;
         filename = "./error_pages/404.html";
         file_info = file_cache.find(filename);
     }

     std::unique_ptr<http::response> resp = std::make_unique<http::response>();
        
     resp->add_body_ref(&file_info->second);
     resp->add_status(error);
     resp->add_header(http::header("Content-Type", "text/html"), false);

     return resp;
  }

};
