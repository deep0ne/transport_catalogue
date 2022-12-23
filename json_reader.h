#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <stdexcept>
#include <sstream>

class JsonReader {
public:

    JsonReader(json::Node& node, catalogue::Transport& catalogue);

    void AddStops();

    void AddDistances();

    void AddBuses();

    renderer::Settings GetSettings();

    void GetOutputJson(std::ostringstream& stream);

private:
    json::Node& node_;
    catalogue::Transport& catalogue_;
};
