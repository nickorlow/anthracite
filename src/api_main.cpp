#include "../lib/anthracite.hpp"
#include "../lib/backends/backend.hpp"
#include "../lib/http/constants.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <span>

using namespace anthracite;

using CallbackType = std::unique_ptr<http::response> (*)(http::request&);
class api_backend : public backends::backend {

    class RouteNode {
    public:
        std::optional<CallbackType> callback;

        RouteNode()
            : callback(std::nullopt)
        {
        }
        std::unordered_map<std::string, RouteNode> routes;
    };

    RouteNode root;

    std::unique_ptr<http::response> default_route(http::request& req)
    {
        std::unique_ptr<http::response> resp = std::make_unique<http::response>();

        resp->add_body("Not Found");
        resp->add_header(http::header("Content-Type", "application/json"));
        resp->add_status(http::status_codes::NOT_FOUND);

        return resp;
    }

    std::unique_ptr<http::response> find_handler(http::request& req)
    {
        std::string filename = req.path().substr(1);
        std::vector<std::string> result;
        std::stringstream ss(filename);
        std::string item;

        RouteNode* cur = &root;
        while (getline(ss, item, '/')) {
            if (cur->routes.find(item) == cur->routes.end()) {
                if (cur->routes.find("*") == cur->routes.end()) {
                    break;
                } else {
                    cur = &cur->routes["*"];
                }
            } else {
                cur = &cur->routes[item];
            }
        }

        if (cur->callback.has_value()) {
            return cur->callback.value()(req);
        } else {
            return default_route(req);
        }
    }

    std::unique_ptr<http::response> handle_request(http::request& req) override
    {
        return find_handler(req);
    }

public:
    api_backend()
    {
        root.routes = std::unordered_map<std::string, RouteNode>();
    }

    void register_endpoint(std::string pathspec, CallbackType callback)
    {
        std::vector<std::string> result;
        std::stringstream ss(pathspec);
        std::string item;

        RouteNode* cur = &root;
        while (getline(ss, item, '/')) {
            cur->routes[item] = RouteNode {};
            cur = &cur->routes[item];
        }

        cur->callback = callback;
    }
};

std::unique_ptr<http::response> handle_request(http::request& req)
{
    std::unique_ptr<http::response> resp = std::make_unique<http::response>();

    resp->add_body(R"({"user": "endpoint"}")");
    resp->add_header(http::header("Content-Type", "application/json"));
    resp->add_status(http::status_codes::OK);

    return resp;
}

int main(int argc, char** argv)
{
    auto args = std::span(argv, size_t(argc));
    api_backend ab;
    ab.register_endpoint("users/*", handle_request);
    //anthracite_main(argc, argv, ab);
}
