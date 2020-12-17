#include "parser/src/Parser.h"
using namespace router;
int main() {
    std::string lefFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	router::parser::Parser parser;
    parser.read(lefFile, defFile, guideFile);
    return 0;
}
