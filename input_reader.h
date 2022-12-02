#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "transport_catalogue.h"

namespace catalogue{
namespace detail{
class InputReader {
public:
    
    void FillInQuery(std::string& query);
    std::vector<std::string> GetStops();
    std::vector<std::string> GetBuses();
    std::vector<std::string> ParseDistance();
    static std::vector<std::string> ParseBus(std::string& bus);

    static Transport::Bus FormBus(std::string& bus);
    static Transport::Stop FormStop(std::string& stop);
    static void FormPointers(std::string& ds,
                        std::unordered_map<std::string_view, Transport::Stop*>& stopname_to_stop,
                        std::unordered_map<std::pair<Transport::Stop*, Transport::Stop*>, double, Transport::StopHasher>& stop_distances);
    
private:
    std::vector<std::string> parse_stops_;
    std::vector<std::string> parse_buses_;
    std::vector<std::string> distances_;
};

}
}
