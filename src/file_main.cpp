#include "../lib/anthracite.hpp"
#include "../lib/backends/file_backend.hpp"

using namespace anthracite;

int main(int argc, char** argv)
{
    backends::file_backend fb("./www");
    anthracite_main(argc, argv, fb);
}
