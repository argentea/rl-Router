#include "parser/src/Parser.h"
#include "database/layer/src/Layer.h"
#include <unistd.h>
using namespace router;
int main() {
	std::cout << "layer test\n";
    std::string lefFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	router::parser::Parser parser;
    parser.read(lefFile, defFile, guideFile);
	db::MetalLayer metalTest =  db::MetalLayer(parser.getDatabase()._metal_layers[0], parser.getDatabase()._metal_layers[0]._tracks);

	std::cout << metalTest.idx << " " << metalTest.name << "\n";
    return 0;
}
