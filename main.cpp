#include <Setting.h>
#include<iostream>
#include"module/router/src/Router.h"
#include"module/database/src/Database.h"
#include"module/parser/src/Parser.h"
using namespace std;
using namespace db;
using namespace router;
int main(int argc, char* argv[]) {
    std::string lefFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.lef";
    std::string defFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.def";
    std::string guideFile = "/home/kunpengjiang/project/rl-router/toys/ispd2018/ispd18_sample/ispd18_sample.input.guide";
	
	cout << "Database test\n";

	Setting setting;
	RrrIterSetting rrrIterSetting(setting);
	
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
	log() << "service inited\n";

	Database database(setting, service);
    auto dieBound = service.physicalDesign.getPhysicalDie().getBounds();
    auto dieRegion = getBoxFromRsynBounds(dieBound);
    if (globalDetails.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
		std::cout << "Die region (in DBU): " << dieRegion << std::endl;
        std::cout << std::endl;
    }

	database.init();
	setting.adapt(database);
//
	if(globalDetails.dbVerbose >= +db::VerboseLevelT::LOW)
	{
		cout << "You should get this line.\n";
	}
	std::cerr << "RL::database init end" << std::endl;
	std::cerr << "RL::database setting adapt end" << std::endl;


    Router router(database, setting, rrrIterSetting);
	router.run();
	database.writeNetTopo(setting.outputFile+".topo");
	database.clear();
	database.writeDEF(setting.outputFile);
    log() << "Finish writing def" << std::endl;
    log() << "MEM: cur=" << utils::mem_use::get_current() << "MB, peak=" << utils::mem_use::get_peak() << "MB"
          << std::endl;
    log() << std::endl;

    return 0;
}
