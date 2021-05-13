#include <Setting.h>
#include <iostream>
#include "database/src/Database.h"
#include <rsyn/session/Session.h>
#include <unistd.h>
using namespace db;
using std::cout;
int main() {
    std::string lefFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	
	cout << "Database test\n";

	Setting setting;
	globalDetails.numThreads = 20;
	setting.tat = 300;
	setting.outputFile = "home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ans";
	globalDetails.dbVerbose = db::VerboseLevelT::HIGH;


	RsynService service;
	Rsyn::Session& session = service.session;
	session.init();
    Rsyn::ISPD2018Reader reader;
    const Rsyn::Json params = {
        {"lefFile", lefFile},
        {"defFile", defFile},
        {"guideFile", guideFile},
    };
	reader.load(&session, params);
	service.init();
	log() << "som\n";

	Database database(setting, service);
//	database.init();
	if(globalDetails.dbVerbose >= +db::VerboseLevelT::LOW)
	{
		cout << "You should get this line.\n";
	}
//	std::cerr << "RL::database init end" << std::endl;
//  db::setting.adapt();
//	std::cerr << "RL::database setting adapt end" << std::endl;

    return 0;
}
