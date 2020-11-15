#include "Logger.h"
#include "iostream"

using namespace logger;

void Logger::info(const std::string &msg) const {
    if (_verbose) {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void Logger::error(const std::string &msg) const {
    std::cout << "[ERROR] " << msg << std::endl;
}
void Logger::warning(const std::string &msg) const {
    if (_verbose > 1) {
        std::cout << "[WARNING] " << msg << std::endl;
    }
}
