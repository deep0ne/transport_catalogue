#include <iostream>
#include <istream>

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    catalogue::Transport catalogue;
    catalogue::detail::InputReader reader;
    int n;
    std::cin >> n;
    std::cin >> std::ws;
    for (int i = 0; i < n; ++i) {
        std::string input_query;
        std::getline(std::cin, input_query);
        reader.FillInQuery(input_query);
    }

    for (auto& stop: reader.GetStops()) {
        catalogue.AddStop(stop);
    }
    
    for (auto& distance: reader.ParseDistance()) {
        catalogue.AddDistance(distance);
    }
    
    for (auto& bus: reader.GetBuses()) {
        catalogue.AddBus(bus);
    }
    
    int k;
    std::cin >> k;
    std::cin >> std::ws;
    for (int i = 0; i < k; ++i) {
        std::string output_query;
        std::getline(std::cin, output_query);
        catalogue.GetAllInfo(output_query);
    }
}
