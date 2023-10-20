#include <filesystem>
#include "backend.cpp"

class file_backend : public backend {
private:
  std::unordered_map<std::string,std::string> file_cache;
  std::string file_dir;

  std::unique_ptr<http_response> handle_request_cache(http_request& req) {
     std::string filename = req.path() == "/" ? "/index.html" : req.path();
     filename = file_dir + filename;
     auto file_info = file_cache.find(filename);

     int status = http_status_codes::OK;
     if (file_info == file_cache.end()) {
         status = http_status_codes::NOT_FOUND;
         filename = "./error_pages/404.html";
         file_info = file_cache.find(filename);
     }

     return std::make_unique<http_response>(file_info->second, filename, status);
  }

  void populate_cache_dir(std::string dir) {
    std::filesystem::recursive_directory_iterator cur = begin(std::filesystem::recursive_directory_iterator(dir));
    std::filesystem::recursive_directory_iterator fin = end(std::filesystem::recursive_directory_iterator(dir));

    while (cur != fin) {
        auto p = cur->path();
        std::string filename = p.string();
        std::stringstream buffer;
        std::ifstream stream(filename);
        buffer << stream.rdbuf();
        file_cache[filename] = buffer.str();
        std::cout << "File at " << filename << " cached (" << file_cache[filename].size() << " bytes)" << std::endl;
        ++cur;
    }
  }

  void populate_cache() {
    populate_cache_dir(file_dir);
    populate_cache_dir("./error_pages/");
  }

public:
  file_backend(std::string dir = "./www") : file_dir(std::move(dir)) {
    populate_cache();
  }

  ~file_backend() = default;

  std::unique_ptr<http_response> handle_request(http_request& req) override {
    return handle_request_cache(req);
  }
};
