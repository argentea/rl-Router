#include "Parser.h"
#include "logger/src/Logger.h"

using namespace router::parser;
using namespace router;

int Parser::read(const std::string &lefFile, const std::string &defFile, const std::string &guideFile) {
    logger::Logger::info("Start reading benchmarks");
    _session.init();
    const Rsyn::Json params = {
            {"lefFile", lefFile},
            {"defFile", defFile},
            {"guideFile", guideFile},
    };
    Rsyn::ISPD2018Reader reader;
    reader.load(&_session, params);

    _physical_service = _session.getService("rsyn.physical");
    _routeGuide_service = _session.getService("rsyn.routingGuide");
    _physical_design = _physical_service->getPhysicalDesign();
    _design = _session.getDesign();
    _module = _design.getTopModule();

    logger::Logger::info("Finish reading benchmarks");
    return 0;
}