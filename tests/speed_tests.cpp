#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "../lib/http/request.hpp"

#ifdef SPEEDTEST_COMPARE_BOOST
#include <boost/beast.hpp>
#endif


using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

constexpr uint32_t num_requests = 10000000;

TEST(speed_tests, request_parse) {
    std::ifstream t("./test_files/test_request.http");
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string raw_req = buffer.str();

    auto start = high_resolution_clock::now();

    for(int i = 0; i < num_requests; ++i) {
        volatile anthracite::http::request req (raw_req, "0.0.0.0");
    }

    auto end = high_resolution_clock::now();

    auto ms_int = duration_cast<milliseconds>(end-start);

    double m_rps = ((1000.0 / ms_int.count()) * num_requests) / 1000000;

    std::cout << "Parsed " << (num_requests/1000000) << " Million requests in " << ms_int << " ms";
    std::cout << " at " << m_rps << " Million RPS " << std::endl;

    ASSERT_LT(ms_int.count(), 2000);
}

#ifdef SPEEDTEST_COMPARE_BOOST
TEST(speed_tests, boost) {
    std::ifstream t("./test_files/test_request.http");
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string raw_req = buffer.str();

    auto start = high_resolution_clock::now();

    for(int i = 0; i < num_requests; ++i) {
        boost::system::error_code ec;
        boost::beast::http::request_parser<boost::beast::http::string_body> p;
        p.put(boost::asio::buffer(raw_req), ec);
        boost::beast::http::request<boost::beast::http::string_body> r = p.get();
    }

    auto end = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(end-start);

    double m_rps = ((1000.0 / ms_int.count()) * num_requests) / 1000000;

    std::cout << "Parsed " << (num_requests/1000000) << " Million requests in " << ms_int << " ms";
    std::cout << " at " << m_rps << " Million RPS " << std::endl;
}
#endif
