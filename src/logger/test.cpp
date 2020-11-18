#include "logger/src/Logger.h"

using namespace router::logger;
int main() {
    Logger::info("info");
    Logger::warning("warning");
    Logger::error("error");
    return 0;
}
