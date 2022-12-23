#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <variant>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace renderer{

inline const double EPSILON = 1e-6;

bool IsZero(double value);

using ColorType = std::variant<std::string, svg::Rgb, svg::Rgba>;

struct Settings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    std::pair<double, double> bus_label_offset;
    int stop_label_font_size;
    std::pair<double, double> stop_label_offset;
    ColorType underlayer_color;
    double underlayer_width;
    std::vector<ColorType> color_palette;
};

class SphereProjector {
public:
    
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Polyline FormPolyLine(const SphereProjector& proj, std::vector<Stop*>& stops, ColorType color, double width);
std::pair<svg::Text, svg::Text> FormRoute(const SphereProjector& proj, std::vector<Stop*>& stops, const Settings& renderer,
                                          std::unordered_map<std::string, renderer::ColorType>& bus_to_color, std::string& bus_name, bool first=true);
svg::Circle FormCircle(const SphereProjector& proj, Stop* unique_path, const Settings& renderer);
std::pair<svg::Text, svg::Text> FormStop(const SphereProjector& proj, const Settings& renderer, Stop* path);
}

