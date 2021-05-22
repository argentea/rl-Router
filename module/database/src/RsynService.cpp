#include "RsynService.h"
namespace db {

void RsynService::init() {
        physicalService = session.getService("rsyn.physical");
        routeGuideService = session.getService("rsyn.routingGuide");

        physicalDesign = physicalService->getPhysicalDesign();
        design = session.getDesign();
        module = design.getTopModule();
    }
}
