#pragma once
#include <string>
#include <set>
#include <vector>

#include "geo.h"

struct Bus;

struct PtrComp {
    bool operator()(const Bus* lhs, const Bus* rhs) const;
};

struct Stop {
    std::string stop_name;
    geo::Coordinates coords;
    std::set<Bus*, PtrComp> buses;

};

struct Bus {
    std::string bus_name;
    std::vector<Stop*> paths;
    double distance = 0;
    double curvature = 0;
    bool round_trip = false;
};

class StopHasher {
public:
    size_t operator() (std::pair<Stop*, Stop*> val) const;
};
