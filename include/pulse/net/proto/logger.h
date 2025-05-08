#pragma once

#include <string>

namespace pulse::net::proto {

class Logger {
public:
    enum class Level { Debug, Info, Warn, Error };
    virtual void log(Level level, const std::string& msg) = 0;
    virtual ~Logger() = default;
};

class NoopLogger : public Logger {
public:
    void log(Level, const std::string&) override {}
};

} // namespace pulse::net::proto