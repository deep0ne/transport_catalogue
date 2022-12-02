#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include <functional>

#include "geo.h"

namespace catalogue{
class Transport {
public:
    struct Bus;

    struct PtrComp {
        bool operator()(const Bus* lhs, const Bus* rhs) const {
            return lhs->bus_name < rhs->bus_name;
        }
    };

    struct Stop {
        std::string stop_name;
        detail::Coordinates coords;
        std::set<Bus*, PtrComp> buses;

    };

    class StopHasher {
    public:
        size_t operator() (std::pair<Stop*, Stop*> val) const {
            return std::hash<const void*>()(val.first) ^ std::hash<const void*>()(val.second);
        }
    };
    
    struct Bus {
        std::string bus_name;
        std::vector<Stop*> paths;
        double distance = 0;
        double curvature = 0;
    };
    
    void AddBus(std::string& bus);
    void AddStop(std::string& stop);
    void AddDistance(std::string& stop);
    
    Bus* FindPath(std::string_view name);
    Stop* FindStop(std::string_view stop);

    void GetAllInfo(std::string_view bus);

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, StopHasher> stop_distances_;
};
}
