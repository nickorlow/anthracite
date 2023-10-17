#include "backend.cpp"
#include <filesystem>

class file_backend : public backend {
private:
  unordered_map<string, string> file_cache;
  string file_dir;

  unique_ptr<http_response> handle_request_cache(http_request& req) {
     string filename = req.path() == "/" ? "/index.html" : req.path();
     filename = file_dir + filename;
     auto file_info = file_cache.find(filename);

     int status = 200;
     if (file_info == file_cache.end()) {
         status = 404;
         filename = "./error_pages/404.html";
         file_info = file_cache.find(filename);
     }

     return make_unique<http_response>(file_info->second, filename, status);
  }

  void populate_cache_dir(string dir) {
    filesystem::recursive_directory_iterator cur = begin(filesystem::recursive_directory_iterator(dir));
    filesystem::recursive_directory_iterator fin = end(filesystem::recursive_directory_iterator(dir));

    while (cur != fin) {
        auto p = cur->path();
        string filename = p.string();
        stringstream buffer;
        ifstream stream(filename);
        buffer << stream.rdbuf();
        file_cache[filename] = buffer.str();
        cout << "File at " << filename << " cached (" << file_cache[filename].size() << " bytes)" << endl;
        ++cur;
    }
  }

  void populate_cache() {
    populate_cache_dir(file_dir);
    populate_cache_dir("./error_pages/");
  }

public:
  file_backend(string dir = "./www") : file_dir(dir) {
    populate_cache();
  }

  unique_ptr<http_response> handle_request(http_request& req) override {
    return handle_request_cache(req);
  }
};
