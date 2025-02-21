#pragma once

#include <optional>
#include <inttypes.h>
#include <string>

namespace anthracite::config {
    class http_config {
        uint16_t _port;
    public:
        http_config(uint16_t port) : _port(port) {};
        virtual ~http_config() {};

        uint16_t port() { return _port; }
    };

    class https_config : public http_config {
        std::string _cert_path;
        std::string _key_path;
    public:
        https_config(uint16_t port, std::string cert_path, std::string key_path) :
            http_config(port), _cert_path(cert_path), _key_path(key_path) {};
    };

    class config {
        int _worker_threads;
        int _max_clients;
        std::optional<http_config> _http_config;
        std::optional<https_config> _https_config;

    public:
        config(int worker_threads, int max_clients) : _worker_threads(worker_threads), _max_clients(max_clients) {
        }

        void add_http_config(http_config config) {
            _http_config = config;
        }

        void add_https_config(https_config config) {
            _https_config = config;
        }

        int worker_threads() {
            return _worker_threads;
        }
        
        int max_clients() {
            return _max_clients;
        }

        std::optional<http_config>& http_cfg() {
            return _http_config;
        }
        
        std::optional<https_config>& https_cfg() {
            return _https_config;
        }
    };
};
