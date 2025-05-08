#pragma once

#include <string>

namespace pulse::net::proto {

class Metrics {
public:
    virtual void increment(const std::string& name, uint64_t amount = 1) = 0;
    virtual void gauge(const std::string& name, double value) = 0;
    virtual ~Metrics() = default;
};

class NoopMetrics : public Metrics {
public:
    void increment(const std::string&, uint64_t) override {}
    void gauge(const std::string&, double) override {}
};

} // namespace pulse::net::proto