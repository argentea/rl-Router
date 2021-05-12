#include<iostream>
#include"module/router/src/Router.h"
#include"module/database/src/Database.h"
#include"module/parser/src/Parser.h"
using namespace std;
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << "bad argument\n";
	}
	else {
		router::parser::Parser p;
		p.read(argv[1], argv[2], argv[3]);
		db::Database db;
		db.init(p);
	}
	return 0;
}
