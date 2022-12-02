#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>

#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

namespace catalogue{
namespace detail{
void GetInfo(Transport& transport, std::string_view query, std::ostream& out) {
    std::string_view type_of_query = query.substr(0, query.find_first_of(' '));
    if (type_of_query == "Bus") {
        auto bus_to_find = transport.FindPath(query.substr(query.find_first_of(' ')+1));
        if (bus_to_find == nullptr) {
            out << query << ": not found" << '\n';
            return;
        }
        
        double route_length = bus_to_find->curvature;
        double curvature = bus_to_find->curvature / bus_to_find->distance;
        std::vector<Transport::Stop*> unique_paths = bus_to_find->paths;
        std::sort(unique_paths.begin(), unique_paths.end());
        int uniqueCount = std::unique(unique_paths.begin(), unique_paths.end()) - unique_paths.begin();
        out << std::setprecision(6);
        out << query << ": " << bus_to_find->paths.size() << " stops on route, "
        << uniqueCount << " unique stops, " << route_length << " route length, " << curvature << " curvature" << '\n';
            
    } else {
        std::string_view stop_query = query.substr(query.find_first_of(' ') + 1);
        auto stop_pointer = transport.FindStop(stop_query);

        if(stop_pointer == nullptr) {
            out << query << ": not found" << '\n';
            return;
        }

        auto buses_from_stop = stop_pointer->buses;
        
        if (buses_from_stop.size() == 0) {
            out << query << ": no buses" << std::endl;
        } else {
            out << query << ": buses ";
            for (auto bus: buses_from_stop) {
                out << bus->bus_name << " ";
            }
            out << '\n';
        }
    }
    
 }
}
}
