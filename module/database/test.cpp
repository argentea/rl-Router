#include "parser/src/Parser.h"
#include "database/src/Database.h"
#include <unistd.h>
using namespace router;
using namespace db;
int main() {
    std::string lefFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	router::parser::Parser parser;
    parser.read(lefFile, defFile, guideFile);
	db::Database database;
	database.init(parser);
	
	std::cout << "database test\n";
    return 0;
}
