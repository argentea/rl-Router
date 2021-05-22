#pragma once

#include "../../global.h"

namespace db {

class RsynService {
public:
    Rsyn::Session session;
    Rsyn::PhysicalService* physicalService;
    Rsyn::RoutingGuide* routeGuideService;
    Rsyn::PhysicalDesign physicalDesign;
    Rsyn::Design design;
    Rsyn::Module module;

    void init();
};

}
