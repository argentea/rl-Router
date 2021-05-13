#include <Setting.h>
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
		db::Setting bsetting;
		db::Setting& setting = bsetting;
		db::Database db(db::Setting& setting);
	}
	return 0;
}
