#pragma once


#include <algorithm>
#include <iostream>
#include <vector>
#include "parser/src/Parser.h"
#include "utils/utils.h"
// STL libraries
#include <cstdlib>
#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <thread>
#include <mutex>

// Boost libraries
#include <boost/program_options.hpp>
#include <boost/icl/split_interval_map.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// Rsyn
#include "rsyn/session/Session.h"
#define RSYN_NO_GUI
#include "rsyn/core/Rsyn.h"
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/ispd18/RoutingGuide.h"
#include "rsyn/io/reader/ISPD2018Reader.h"

using boostPoint = bg::model::point<DBU, 2, bg::cs::cartesian>;
using boostBox = bg::model::box<boostPoint>;
using RTree = bgi::rtree<std::pair<boostBox, int>, bgi::rstar<32>>;
using RTrees = vector<bgi::rtree<std::pair<boostBox, int>, bgi::rstar<32>>>;
