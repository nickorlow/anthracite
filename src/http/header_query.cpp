#include <string>

class name_value {
private:
  std::string _name;
  std::string _value;

protected:
    name_value() {}

public:
    name_value(std::string name, std::string value)
        : _name(std::move(name))
        , _value(std::move(value))
    {
    }
    virtual ~name_value() = default;
    name_value(const name_value&) = default;
    name_value& operator=(name_value const&) = default;
    name_value(name_value&&) = default;
    name_value& operator=(name_value&&) = default;

    std::string name() { return _name; }
    std::string value() { return _value; }

    virtual std::string to_string() { return ""; }
};

class http_header : public name_value {
public:
    http_header()
        : name_value()
    {
    }
    http_header(std::string name, std::string value)
        : name_value(name, value)
    {
    }

    std::string to_string() override { return name() + ": " + value() + "\r\n"; }
};

class query_param : public name_value {
public:
    query_param()
        : name_value()
    {
    }
    query_param(std::string name, std::string value)
        : name_value(name, value)
    {
    }

    std::string to_string() override { return name() + "=" + value(); }
};
