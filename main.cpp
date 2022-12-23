#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <fstream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

using namespace json;
using namespace std::literals;

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

int main() {
    // считываем JSON из stdin
    catalogue::Transport catalogue;
    const json::Document requests = json::Load(std::cin);
    auto node = requests.GetRoot();
    JsonReader json_reader(node, catalogue);

    json_reader.AddStops();
    json_reader.AddDistances();
    json_reader.AddBuses();
    
    renderer::Settings settings = json_reader.GetSettings();
    RequestHandler handler(catalogue, settings);
    auto doc = handler.RenderMap();
    
    std::ostringstream ss;
    doc.Render(ss);
    json_reader.GetOutputJson(ss);
}
