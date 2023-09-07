#include "backend.cpp"

class file_backend : public backend {
public:
  http_response handle_request(http_request req) {
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
     return http_response(buffer.str(), status);
  }
};
