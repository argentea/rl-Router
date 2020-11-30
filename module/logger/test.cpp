#include "fmt/format.h"
#include "fmt/ranges.h"
#include "src/Logger.h"
#include <vector>

using namespace router::logger;
int main() {
    std::vector vec{1, 2, 3};
    for (int i = 0; i <= ASSERT; i++) {
        router::logger::print(MessageType(i), "this is %d \n", 23);
        router::logger::print(MessageType(i), fmt::format("fmt: {}\n", 23));
        router::logger::print(MessageType(i), fmt::format("fmt: {}\n", vec));
    }
    return 0;
}
