#ifndef RL_ROUTER_LOGGER_H
#define RL_ROUTER_LOGGER_H

#include <string>

namespace logger {
class Logger {
public:
    void info(const std::string &msg) const;

    void error(const std::string &msg) const;

    void warning(const std::string &msg) const;

private:
    // 1: print info
    // 2: print info warning
    int _verbose{2};
};
}// namespace logger
#endif//RL_ROUTER_LOGGER_H
