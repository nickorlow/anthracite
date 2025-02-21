#include "../lib/anthracite.hpp"
#include "../lib/backends/file_backend.hpp"
#include "../lib/config/config.hpp"

using namespace anthracite;

int main(int argc, char** argv)
{
    backends::file_backend fb("./www");
    config::config cfg(5);
    cfg.add_http_config(config::http_config(8080));
    cfg.add_https_config(config::https_config(8081, "", ""));

    anthracite_main(fb, cfg);
}
