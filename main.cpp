#include<iostream>
#include"router/src/Router.h"
#include"database/src/Database.h"
#include"parser/src/Parser.h"
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
