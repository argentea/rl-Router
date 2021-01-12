#include "parser/src/Parser.h"
using namespace router;
int main() {
	std::cout << "parser test\n";
    std::string lefFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	router::parser::Parser parser;
    parser.read(lefFile, defFile, guideFile);
    return 0;
}
