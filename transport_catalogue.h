#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include <functional>

#include "geo.h"
#include "domain.h"

namespace catalogue{
class Transport {
public:
    
    void AddBus(std::string& bus_name, std::vector<std::string>& bus_stops, bool& round_trip);
    void AddStop(Stop& stop);
    void AddDistance(const std::string& stop1, const std::string& stop2, double distance);
    
    Bus* FindPath(std::string_view name) const;
    Stop* FindStop(std::string_view stop) const;

    std::deque<Stop> GetStops() const;
    std::deque<Bus> GetBuses() const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, StopHasher> stop_distances_;
};
}
