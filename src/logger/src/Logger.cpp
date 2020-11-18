#include "Logger.h"
#include "iostream"

using namespace router::logger;

void Logger::info(const std::string &msg) {
    if (_printInfo) {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void Logger::error(const std::string &msg) {
    if (_printError) {
        std::cout << "[ERROR] " << msg << std::endl;
    }
}
void Logger::warning(const std::string &msg) {
    if (_printWarning) {
        std::cout << "[WARNING] " << msg << std::endl;
    }
}
