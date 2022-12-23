#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"
#include "svg.h"

JsonReader::JsonReader(json::Node& node, catalogue::Transport& catalogue)
    : node_(node), catalogue_(catalogue) {}


void JsonReader::AddStops() {
    Stop stop;
    auto requests_map = node_.AsMap();
    if (requests_map.count("base_requests") == 0) {
        throw std::invalid_argument("JSON is invalid");
    }
    auto requests = requests_map.at("base_requests").AsArray();
    for(auto node: requests) {
        auto info = node.AsMap();
        if (info.at("type").AsString() == "Bus") {
            continue;
        }
        // формируем структуру остановки и добавляем в контейнер класса каталога
        stop.stop_name = info.at("name").AsString();
        stop.coords.lat = info.at("latitude").AsDouble();
        stop.coords.lng = info.at("longitude").AsDouble();
        catalogue_.AddStop(stop);
    }
}

void JsonReader::AddDistances() {
    auto requests_map = node_.AsMap();
    if (requests_map.count("base_requests") == 0) {
        throw std::invalid_argument("JSON is invalid");
    }
    auto requests = requests_map.at("base_requests").AsArray();
    for(auto node: requests) {
        auto info = node.AsMap();
        if (info.at("type").AsString() == "Bus") {
            continue;
        }
        auto stop_name = info.at("name").AsString();
        for (auto& [distance_to, distance]: info.at("road_distances").AsMap()) {
            catalogue_.AddDistance(stop_name, distance_to, distance.AsInt());
        }
    }
}

void JsonReader::AddBuses() {
    auto requests_map = node_.AsMap();
    if (requests_map.count("base_requests") == 0) {
        throw std::invalid_argument("JSON is invalid");
    }
    auto requests = requests_map.at("base_requests").AsArray();
    for(auto node: requests) {
        bool round_trip = true;
        auto info = node.AsMap();
        if (info.at("type").AsString() == "Stop") {
            continue;
        }
        std::vector<std::string> bus_stops;
        auto bus_name = info.at("name").AsString();
        
        for(auto stop: info.at("stops").AsArray()) {
            bus_stops.push_back(stop.AsString());
        }
        if(!info.at("is_roundtrip").AsBool()) {
            round_trip = false;
            for(int i = bus_stops.size()-2; i >= 0; --i) {
                bus_stops.push_back(bus_stops[i]);
            }
        }
        catalogue_.AddBus(bus_name, bus_stops, round_trip);
    }
}

renderer::Settings JsonReader::GetSettings() {
    renderer::Settings settings_struct;
    auto requests_map = node_.AsMap();
    if(requests_map.count("render_settings") == 0) {
        throw std::invalid_argument("JSON does not contain render settings");
    }
    auto settings = requests_map.at("render_settings").AsMap();
    for (auto& [setting, value]: settings) {
        if(setting == "width") {
            settings_struct.width = value.AsDouble();
        } else if (setting == "height") {
            settings_struct.height = value.AsDouble();
        } else if (setting == "padding") {
            settings_struct.padding = value.AsDouble();
        } else if (setting == "line_width") {
            settings_struct.line_width = value.AsDouble();
        } else if (setting == "stop_radius") {
            settings_struct.stop_radius = value.AsDouble();
        } else if (setting == "bus_label_font_size") {
            settings_struct.bus_label_font_size = value.AsInt();
        } else if (setting == "bus_label_offset") {
            std::pair<double, double> dxdy;
            auto dxdyarray = value.AsArray();
            dxdy.first = dxdyarray[0].AsDouble();
            dxdy.second = dxdyarray[1].AsDouble();
            settings_struct.bus_label_offset = dxdy;
        } else if (setting == "stop_label_font_size") {
            settings_struct.stop_label_font_size = value.AsInt();
        } else if (setting == "stop_label_offset") {
            std::pair<double, double> dxdy;
            auto dxdyarray = value.AsArray();
            dxdy.first = dxdyarray[0].AsDouble();
            dxdy.second = dxdyarray[1].AsDouble();
            settings_struct.stop_label_offset = dxdy;
        } else if (setting == "underlayer_color") {
            if(value.IsString()) {
                settings_struct.underlayer_color = value.AsString();
            }
            if(value.IsArray()) {
                if (value.AsArray().size() == 3) {
                    auto colors = value.AsArray();
                    settings_struct.underlayer_color = svg::Rgb(colors[0].AsInt(), colors[1].AsInt(), colors[2].AsInt());
                } else {
                    auto colors = value.AsArray();
                    settings_struct.underlayer_color = svg::Rgba(colors[0].AsInt(), colors[1].AsInt(), colors[2].AsInt(), colors[3].AsDouble());
                }
            }
        } else if (setting == "underlayer_width") {
            settings_struct.underlayer_width = value.AsDouble();
        } else if (setting == "color_palette") {
            auto color_palette = value.AsArray();
            for (auto color: color_palette) {
                if(color.IsString()) {
                    settings_struct.color_palette.push_back(color.AsString());
                } else {
                    auto rgb_colors = color.AsArray();
                    if(rgb_colors.size() == 3) {
                        settings_struct.color_palette.push_back(svg::Rgb(rgb_colors[0].AsInt(),
                                                                         rgb_colors[1].AsInt(),
                                                                         rgb_colors[2].AsInt()));
                    } else {
                        settings_struct.color_palette.push_back(svg::Rgba(rgb_colors[0].AsInt(),
                                                                         rgb_colors[1].AsInt(),
                                                                         rgb_colors[2].AsInt(),
                                                                         rgb_colors[3].AsDouble()));
                    }
                }
          }
      }
    }
    return settings_struct;
}

void JsonReader::GetOutputJson(std::ostringstream& stream) {
    auto requests_map = node_.AsMap();
    if (requests_map.count("stat_requests") == 0) {
        throw std::invalid_argument("There are no stat requests");
    }
    json::Array answers;
    auto stat_requests = requests_map.at("stat_requests").AsArray();
    for (auto node: stat_requests) {
        auto request = node.AsMap();
        if (request.at("type").AsString() == "Stop") {
            json::Dict answer;
            json::Array buses;
            auto stop_name = request.at("name").AsString();
            auto stop_pointer = catalogue_.FindStop(stop_name);
            if(stop_pointer == nullptr) {
                answer["request_id"] = request.at("id");
                answer["error_message"] = json::Node(std::string("not found"));
                answers.push_back(answer);
                continue;
            }
            auto buses_from_stop = stop_pointer->buses;
            for(auto bus: buses_from_stop) {
                buses.push_back(json::Node(bus->bus_name));
            }
            answer["buses"] = buses;
            answer["request_id"] = request.at("id");
            answers.push_back(json::Node(answer));
        
        } else if (request.at("type").AsString() == "Bus") {
            auto bus_name = request.at("name").AsString();
            auto bus_to_find = catalogue_.FindPath(bus_name);
            json::Dict answer;
            if (bus_to_find == nullptr) {
                answer["request_id"] = request.at("id");
                answer["error_message"] = json::Node(std::string("not found"));
                answers.push_back(answer);
                continue;
            }
            double curvature = bus_to_find->curvature / bus_to_find->distance;
            answer["curvature"] = json::Node(curvature);
            answer["request_id"] = request.at("id");
            answer["route_length"] = json::Node(bus_to_find->curvature);
            answer["stop_count"] = json::Node(static_cast<int>(bus_to_find->paths.size()));
            std::vector<Stop*> unique_paths = bus_to_find->paths;
            std::sort(unique_paths.begin(), unique_paths.end());
            int uniqueCount = std::unique(unique_paths.begin(), unique_paths.end()) - unique_paths.begin();
            answer["unique_stop_count"] = json::Node(uniqueCount);
            answers.push_back(answer);
        } else {
            json::Dict answer;
            answer["map"] = json::Node(stream.str());
            answer["request_id"] = request.at("id");
            answers.push_back(answer);
        }
    }
    std::ostringstream out;
    Print(json::Document{answers}, out);
    std::cout << out.str();
}
