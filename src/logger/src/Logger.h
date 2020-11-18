#ifndef RL_ROUTER_LOGGER1_H
#define RL_ROUTER_LOGGER1_H

#include <string>

namespace router {
namespace logger {
class Logger {
public:
    static void info(const std::string &msg);

    static void warning(const std::string &msg);

    static void error(const std::string &msg);

private:
    const static bool _printInfo = true;
    const static bool _printWarning = true;
    const static bool _printError = true;
};
}// namespace logger
}// namespace router
#endif//RL_ROUTER_LOGGER_H
