#include <string>
#include <algorithm>
#include <vector>

#include "input_reader.h"
#include "transport_catalogue.h"

namespace catalogue{
namespace detail{
void InputReader::FillInQuery(std::string& query) {
    query = query.substr(query.find_first_not_of(' '));
    std::string type_of_query = query.substr(query.find_first_not_of(' '), query.find_first_of(' '));
    std::string text = query.substr(query.find_first_of(' ') + 1);
    std::string text_to_add, distance_to_add;
    if (type_of_query == "Stop") {
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        text_to_add += text.substr(0, text.find_first_of(':') + 2);
        text = text.substr(text.find_first_of(':') + 2);
        distance_to_add = text_to_add;
        if(text.find_first_of(delimiter) != std::string::npos) {
            for (int i = 0; i < 2; ++i) {
                pos = text.find(delimiter);
                token = text.substr(0, pos+1);
                text_to_add += token;
                text.erase(0, pos + delimiter.length());
            }
        }
        if(text.find_first_of(delimiter) == std::string::npos) {
            text_to_add += text;
        } else {
            text_to_add = text_to_add.substr(0, text_to_add.find_last_of(','));
            distances_.push_back(std::move(distance_to_add + text));
        }
        parse_stops_.push_back(std::move(text_to_add));
    } else {
        text = query.substr(query.find_first_of(' ') + 1);
        parse_buses_.push_back(std::move(text));
    }
}

std::vector<std::string> InputReader::GetStops() {
    return parse_stops_;
}

std::vector<std::string> InputReader::GetBuses() {
    return parse_buses_;
}

std::vector<std::string> InputReader::ParseDistance() {
    return distances_;
}
std::vector<std::string> InputReader::ParseBus(std::string& bus) {
    bus = bus.substr(bus.find_first_of(':') + 2);
    std::vector<std::string> buses;
    std::string delimiter;
    size_t pos = 0;
    std::string token;
    if(bus.find_first_of('>') != std::string::npos) {
        delimiter = " > ";
        while ((pos = bus.find(delimiter)) != std::string::npos) {
            token = bus.substr(0, pos);
            buses.push_back(std::move(token));
            bus.erase(0, pos + delimiter.length());
        }
        buses.push_back(std::move(bus));  
    } else {
        delimiter = " - ";
        while ((pos = bus.find(delimiter)) != std::string::npos) {
            token = bus.substr(0, pos);
            buses.push_back(std::move(token));
            bus.erase(0, pos + delimiter.length());
        }
        buses.push_back(std::move(bus));
        
        for (int i = buses.size() - 2; i >= 0; --i) {
            buses.push_back(buses[i]);
        }
    }
        
    return buses;
}
Transport::Bus InputReader::FormBus(std::string& bus) {
        Transport::Bus bus_to_add;
        bus_to_add.bus_name = bus.substr(0, bus.find_first_of(':'));
        return bus_to_add;
    }
    
Transport::Stop InputReader::FormStop(std::string& stop) {
    Transport::Stop stop_to_add;
    stop_to_add.stop_name = stop.substr(0, stop.find_first_of(':'));
    stop_to_add.coords.lat = std::stod(stop.substr(stop.find_first_of(':') + 2, stop.find_first_of(',')));
    stop_to_add.coords.lng = std::stod(stop.substr(stop.find_first_of(',') + 2));
    return stop_to_add;
}

void InputReader::FormPointers(std::string& ds,
                    std::unordered_map<std::string_view, Transport::Stop*>& stopname_to_stop,
                    std::unordered_map<std::pair<Transport::Stop*, Transport::Stop*>, double, Transport::StopHasher>& stop_distances) {
    Transport::Stop* first_ptr = stopname_to_stop[ds.substr(0, ds.find_first_of(':'))];
    ds = ds.substr(ds.find_first_of(':') + 2);
    std::string delimiter = "m to ";
    size_t pos = 0;
    std::string stop;
    double distance = 0;
    while((pos = ds.find(delimiter)) != std::string::npos) {
        distance = std::stod(ds.substr(0, pos));
        ds.erase(0, pos + delimiter.length());
        if (ds.find(',') != std::string::npos) {
            stop = ds.substr(0, ds.find_first_of(','));
            ds.erase(0, ds.find_first_of(',') + 2);
        } else {
            stop = ds;
        }
        Transport::Stop* second_ptr = stopname_to_stop[stop];
        std::pair<Transport::Stop*, Transport::Stop*> ptr_pair = std::make_pair(first_ptr, second_ptr);
        stop_distances[ptr_pair] = distance;
    }
}
}
}
