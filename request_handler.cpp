#include "request_handler.h"


RequestHandler::RequestHandler(const catalogue::Transport& db, const renderer::Settings& renderer)
:db_(db), renderer_(renderer)
{
}

svg::Document RequestHandler::RenderMap() const {
    svg::Document doc;
    std::vector<geo::Coordinates> geo_coords;
    std::unordered_map<std::string, renderer::ColorType> bus_to_color;
    size_t index = 0;
    std::deque<Bus> buses = db_.GetBuses();

    for(auto bus: buses) {
        for(auto path: bus.paths) {
            geo_coords.push_back(path->coords);
        }
    }

    const renderer::SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), renderer_.width, renderer_.height, renderer_.padding
    };

    sort(buses.begin(), buses.end(), [] (const auto& b1, const auto& b2){
        return b1.bus_name < b2.bus_name;
    });

    for(auto bus: buses) {
        if(bus.paths.empty()) {
            continue;
        }
        if(index > renderer_.color_palette.size() - 1) {
            index = 0;
        }
        bus_to_color[bus.bus_name] = renderer_.color_palette[index];
        doc.Add(renderer::FormPolyLine(proj, bus.paths, renderer_.color_palette[index], renderer_.line_width));
        ++index;
    }

    for(auto bus: buses) {
        if(bus.paths.empty()) {
            continue;
        }
        auto [undertext, text] = FormRoute(proj, bus.paths, renderer_, bus_to_color, bus.bus_name);
        doc.Add(undertext);
        doc.Add(text);

        if(!bus.round_trip) {
            const int last = static_cast<int>(bus.paths.size()) / 2;
            if (bus.paths[0] == bus.paths[last]) {
                continue;
            }
            auto [undertext, text] = FormRoute(proj, bus.paths, renderer_, bus_to_color, bus.bus_name, false);
            doc.Add(undertext);
            doc.Add(text);   
        }
    }

    std::vector<Stop*> unique_paths;
    for(auto bus: buses) {
        for(auto path: bus.paths) {
            unique_paths.push_back(path);
        }
    }
    std::sort(unique_paths.begin(), unique_paths.end());
    unique_paths.erase( unique( unique_paths.begin(), unique_paths.end() ), unique_paths.end() );
    std::sort(unique_paths.begin(), unique_paths.end(), [](const auto stop1, const auto stop2){
        return stop1->stop_name < stop2->stop_name;
    });    

    for (auto path: unique_paths) {
        doc.Add(renderer::FormCircle(proj, path, renderer_));
    }

    for(auto path: unique_paths) {
        auto [undertext, text] = renderer::FormStop(proj, renderer_, path);
        doc.Add(undertext);
        doc.Add(text);
    }

    return doc;
}
