#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"

namespace catalogue {


Bus* Transport::FindPath(std::string_view name) const {
    if (busname_to_bus_.count(name)) {
        return busname_to_bus_.at(name);
    }
    return nullptr;
}

Stop* Transport::FindStop(std::string_view stop) const {
    if (stopname_to_stop_.count(stop)) {
        return stopname_to_stop_.at(stop);
    }
    return nullptr;
}


void Transport::AddStop(Stop& stop) {
    stops_.push_back(std::move(stop));
    Stop* stop_pointer = &stops_.back();
    stopname_to_stop_[stop_pointer->stop_name] = stop_pointer;
}

void Transport::AddDistance(const std::string& stop1, const std::string& stop2, double distance) {
    Stop* first_ptr = stopname_to_stop_[stop1];
    Stop* second_ptr = stopname_to_stop_[stop2];
    std::pair<Stop*, Stop*> ptr_pair = std::make_pair(first_ptr, second_ptr);
    stop_distances_[ptr_pair] = distance;
}

void Transport::AddBus(std::string& bus_name, std::vector<std::string>& bus_stops, bool& round_trip) {
    Bus bus_to_add;
    bus_to_add.bus_name = bus_name;
    bus_to_add.round_trip = round_trip;
    for (const auto& stop: bus_stops) {
        if (stopname_to_stop_.count(stop)) {
            bus_to_add.paths.push_back(stopname_to_stop_[stop]);
        }
    }

    for (int i = 0; i < bus_to_add.paths.size() - 1; ++i) {
            bus_to_add.distance += geo::ComputeDistance(bus_to_add.paths[i]->coords, bus_to_add.paths[i+1]->coords);
            std::pair<Stop*, Stop*> pair_to_find = std::make_pair(bus_to_add.paths[i], bus_to_add.paths[i+1]);
            if(stop_distances_.count(pair_to_find)) {
                bus_to_add.curvature += stop_distances_[pair_to_find];
            } else {
                std::swap(pair_to_find.first, pair_to_find.second);
                bus_to_add.curvature += stop_distances_[pair_to_find];
            }
        }
    buses_.push_back(std::move(bus_to_add));
    Bus* bus_pointer = &buses_.back();
    for (const auto& bus_name: bus_stops) {
        if(stopname_to_stop_.count(bus_name)) {
            stopname_to_stop_[bus_name]->buses.insert(bus_pointer);
        }
    }
    busname_to_bus_[bus_pointer->bus_name] = bus_pointer;
}

std::deque<Bus> Transport::GetBuses() const {
    return buses_;
}

std::deque<Stop> Transport::GetStops() const {
    return stops_;
}

}
