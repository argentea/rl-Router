#include "database/src/Net.h"
#include "logger/Logger.h"
#include "parser/Parser.h"
#include <iostream>

int main() {
    std::string lefFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
    parser::Parser parser;
    parser.read(lefFile, defFile, guideFile);
    database::NetList netlist;
    parser.initNetlist(netlist);
    return 0;
}
