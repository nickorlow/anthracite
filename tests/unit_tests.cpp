#include "../lib/http/request.hpp"
#include <boost/beast.hpp>
#include <fstream>
#include <gtest/gtest.h>

TEST(unit_tests, single_request_parse)
{
    std::ifstream t("./test_files/test_request.http");
    std::stringstream buffer;
    buffer << t.rdbuf();

    std::string raw_req = buffer.str();
    std::string expected = buffer.str();

    anthracite::http::request req(raw_req, "0.0.0.0");

    ASSERT_EQ(expected, req.to_string());
}
