#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>

#include "transport_catalogue.h"
#include "stat_reader.h"
#include "geo.h"
#include "input_reader.h"

namespace catalogue {


Transport::Bus* Transport::FindPath(std::string_view name) {
    if (busname_to_bus_.count(name)) {
        return busname_to_bus_[name];
    }
    return nullptr;
}

Transport::Stop* Transport::FindStop(std::string_view stop) {
    if (stopname_to_stop_.count(stop)) {
        return stopname_to_stop_[stop];
    }
    return nullptr;
}


void Transport::AddStop(std::string& stop) {
    Transport::Stop stop_to_add = detail::InputReader::FormStop(stop);
    stops_.push_back(std::move(stop_to_add));
    Transport::Stop* stop_pointer = &stops_.back();
    stopname_to_stop_[stop_pointer->stop_name] = stop_pointer;
}

void Transport::AddBus(std::string& bus) {
    Transport::Bus bus_to_add = detail::InputReader::FormBus(bus);
    std::vector<std::string> buses = detail::InputReader::ParseBus(bus);
    for (const auto& bus_name: buses) {
        if (stopname_to_stop_.count(bus_name)) {
            bus_to_add.paths.push_back(stopname_to_stop_[bus_name]);
        }
    }

    for (int i = 0; i < bus_to_add.paths.size() - 1; ++i) {
            bus_to_add.distance += detail::ComputeDistance(bus_to_add.paths[i]->coords, bus_to_add.paths[i+1]->coords);
            std::pair<Transport::Stop*, Transport::Stop*> pair_to_find = std::make_pair(bus_to_add.paths[i], bus_to_add.paths[i+1]);
            if(stop_distances_.count(pair_to_find)) {
                bus_to_add.curvature += stop_distances_[pair_to_find];
            } else {
                std::swap(pair_to_find.first, pair_to_find.second);
                bus_to_add.curvature += stop_distances_[pair_to_find];
            }
        }

    buses_.push_back(std::move(bus_to_add));
    Transport::Bus* bus_pointer = &buses_.back();
    for (const auto& bus_name: buses) {
        if(stopname_to_stop_.count(bus_name)) {
            stopname_to_stop_[bus_name]->buses.insert(bus_pointer);
        }
    }
    busname_to_bus_[bus_pointer->bus_name] = bus_pointer;
}

void Transport::AddDistance(std::string& ds) {
    detail::InputReader::FormPointers(ds, stopname_to_stop_, stop_distances_);
}

void Transport::GetAllInfo(std::string_view query) {
    detail::GetInfo(*this, query);
}
}
