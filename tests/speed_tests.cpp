#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "../lib/http/request.hpp"


TEST(speed_tests, request_parse) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    std::ifstream t("./test_files/test_request.http");
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string raw_req = buffer.str();

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < 1000000; ++i) {
        volatile anthracite::http::request req (raw_req, "0.0.0.0");
    }
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    std::cout << "Parsed 1 Million requests in " << ms_int << "ms" << std::endl;
}
