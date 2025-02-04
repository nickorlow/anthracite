#include "../lib/anthracite.hpp"
#include "../lib/backends/file_backend.hpp"

using namespace anthracite;

int main(int argc, char** argv)
{
    auto args = std::span(argv, size_t(argc));
    backends::file_backend fb(argc > 2 ? args[2] : "./www");
    anthracite_main(argc, argv, fb);
}
