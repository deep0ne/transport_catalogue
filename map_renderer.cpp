#include "map_renderer.h"

#include "geo.h"

namespace renderer{

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

svg::Polyline FormPolyLine(const SphereProjector& proj, std::vector<Stop*>& stops, ColorType color, double width) {
    svg::Polyline polyline;
    for(auto path: stops) {
        polyline.AddPoint(proj(path->coords));
    }
    
    if(std::holds_alternative<std::string>(color)) {
        polyline.SetStrokeColor(std::get<std::string>(color));
    } else if (std::holds_alternative<svg::Rgb>(color)) {
        polyline.SetStrokeColor(std::get<svg::Rgb>(color));
    } else {
        polyline.SetStrokeColor(std::get<svg::Rgba>(color));
    }
    
    polyline.SetStrokeWidth(width)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
    .SetFillColor("none");
    
    return polyline;   
}

std::pair<svg::Text, svg::Text> FormRoute(const SphereProjector& proj, std::vector<Stop*>& stops, const Settings& renderer,
                                          std::unordered_map<std::string, renderer::ColorType>& bus_to_color, std::string& bus_name, bool first) {
    
    if(first) {
        auto first_path = stops.front();
        svg::Text under_first, first_stop;
        if(std::holds_alternative<std::string>(renderer.underlayer_color)) {
            std::string color = std::get<std::string>(renderer.underlayer_color);
            under_first.SetFillColor(color).SetStrokeColor(color);
        } else if (std::holds_alternative<svg::Rgb>(renderer.underlayer_color)) {
            under_first.SetFillColor(std::get<svg::Rgb>(renderer.underlayer_color))
            .SetStrokeColor(std::get<svg::Rgb>(renderer.underlayer_color));
        } else {
            under_first.SetFillColor(std::get<svg::Rgba>(renderer.underlayer_color))
            .SetStrokeColor(std::get<svg::Rgba>(renderer.underlayer_color));
        }
            
        under_first.SetStrokeWidth(renderer.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetOffset(svg::Point{renderer.bus_label_offset.first, renderer.bus_label_offset.second})
        .SetFontSize(renderer.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus_name)
        .SetPosition(proj(first_path->coords));

        
        if(std::holds_alternative<std::string>(bus_to_color[bus_name])) {
            first_stop.SetFillColor(std::get<std::string>(bus_to_color[bus_name]));
        } else if (std::holds_alternative<svg::Rgb>(bus_to_color[bus_name])) {
            first_stop.SetFillColor(std::get<svg::Rgb>(bus_to_color[bus_name]));
        } else {
            first_stop.SetFillColor(std::get<svg::Rgba>(bus_to_color[bus_name]));
        }
        first_stop.SetPosition(proj(first_path->coords))
        .SetOffset(svg::Point{renderer.bus_label_offset.first, renderer.bus_label_offset.second})
        .SetFontSize(renderer.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus_name);
        return std::make_pair(under_first, first_stop);
    } else {
        const int last = static_cast<int>(stops.size()) / 2;

        auto second_path = stops[last];
        svg::Text under_second, second_stop;
        
        if(std::holds_alternative<std::string>(renderer.underlayer_color)) {
            under_second.SetFillColor(std::get<std::string>(renderer.underlayer_color))
            .SetStrokeColor(std::get<std::string>(renderer.underlayer_color));
        } else if (std::holds_alternative<svg::Rgb>(renderer.underlayer_color)) {
            under_second.SetFillColor(std::get<svg::Rgb>(renderer.underlayer_color))
            .SetStrokeColor(std::get<svg::Rgb>(renderer.underlayer_color));
        } else {
            under_second.SetFillColor(std::get<svg::Rgba>(renderer.underlayer_color))
            .SetStrokeColor(std::get<svg::Rgba>(renderer.underlayer_color));
        }
        
        under_second.SetStrokeWidth(renderer.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetOffset(svg::Point{renderer.bus_label_offset.first, renderer.bus_label_offset.second})
        .SetFontSize(renderer.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus_name)
        .SetPosition(proj(second_path->coords));

    
        if(std::holds_alternative<std::string>(bus_to_color[bus_name])) {
            second_stop.SetFillColor(std::get<std::string>(bus_to_color[bus_name]));
        } else if (std::holds_alternative<svg::Rgb>(bus_to_color[bus_name])) {
            second_stop.SetFillColor(std::get<svg::Rgb>(bus_to_color[bus_name]));
        } else {
            second_stop.SetFillColor(std::get<svg::Rgba>(bus_to_color[bus_name]));
        }
        second_stop.SetPosition(proj(second_path->coords))
        .SetOffset(svg::Point{renderer.bus_label_offset.first, renderer.bus_label_offset.second})
        .SetFontSize(renderer.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(bus_name);
        return std::make_pair(under_second, second_stop);
    }
}

svg::Circle FormCircle(const SphereProjector& proj, Stop* unique_path, const Settings& renderer) {
    svg::Circle circle;
    circle.SetCenter(proj(unique_path->coords)).SetRadius(renderer.stop_radius).SetFillColor("white");
    return circle;
}

std::pair<svg::Text, svg::Text> FormStop(const SphereProjector& proj, const Settings& renderer, Stop* path) {
    svg::Text under, stop;

    if(std::holds_alternative<std::string>(renderer.underlayer_color)) {
        under.SetFillColor(std::get<std::string>(renderer.underlayer_color))
        .SetStrokeColor(std::get<std::string>(renderer.underlayer_color));
    } else if (std::holds_alternative<svg::Rgb>(renderer.underlayer_color)) {
        under.SetFillColor(std::get<svg::Rgb>(renderer.underlayer_color))
        .SetStrokeColor(std::get<svg::Rgb>(renderer.underlayer_color));
    } else {
        under.SetFillColor(std::get<svg::Rgba>(renderer.underlayer_color))
        .SetStrokeColor(std::get<svg::Rgba>(renderer.underlayer_color));
    }
    under.SetStrokeWidth(renderer.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
    .SetPosition(proj(path->coords)).SetOffset({renderer.stop_label_offset.first, renderer.stop_label_offset.second})
    .SetFontSize(renderer.stop_label_font_size).SetFontFamily("Verdana").SetData(path->stop_name);

    stop.SetFillColor("black").SetPosition(proj(path->coords)).SetOffset({renderer.stop_label_offset.first, renderer.stop_label_offset.second})
    .SetFontSize(renderer.stop_label_font_size).SetFontFamily("Verdana").SetData(path->stop_name);
    return std::make_pair(under, stop);
}

}

