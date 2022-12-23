#include "svg.h"

#include <iomanip>
#include <cmath>
namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineCap> line_cap) {
    if (line_cap == StrokeLineCap::BUTT) {
        out << "butt";
    }
    if (line_cap == StrokeLineCap::ROUND) {
        out << "round";
    }
    if(line_cap == StrokeLineCap::SQUARE) {
        out << "square";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineJoin> line_join) {
    if (line_join == StrokeLineJoin::ARCS) {
        out << "arcs";
    }
    if (line_join == StrokeLineJoin::BEVEL) {
        out << "bevel";
    }
    if(line_join == StrokeLineJoin::MITER) {
        out << "miter";
    }
    if(line_join == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip";
    }
    if (line_join == StrokeLineJoin::ROUND) {
        out << "round";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    if(std::holds_alternative<std::monostate>(color)) {
        out << "none";
    }
    if(std::holds_alternative<std::string>(color)) {
        out << std::get<std::string>(color);
    }
    if(auto colors = std::get_if<svg::Rgb>(&color)) {
        out << "rgb(" << unsigned(colors->red) << "," << unsigned(colors->green) << "," << unsigned(colors->blue) << ")";
    }
    if(auto colors = std::get_if<svg::Rgba>(&color)) {
        out << "rgba(" << unsigned(colors->red) << "," << unsigned(colors->green) << "," << unsigned(colors->blue) << "," << colors->opacity << ")";
    }
    return out;
}

// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------


Polyline& Polyline::AddPoint(Point point) {
    std::ostringstream ss;
  
    ss << point.x << " " << point.y;

    auto x = ss.str().substr(0, ss.str().find_last_of(' '));
    auto y = ss.str().substr(ss.str().find_first_of(' ') + 1);
    if(points_.empty()) {
        points_ += x + ',' + y;
    } else {
        points_ += ' ' + x + ',' + y;
    }
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv << points_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if(!font_family_.empty()) {
        if(font_weight_.empty()) {
            out << "font-family=\""sv << font_family_ << "\""sv;
        } else {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
    }
    if(!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << '>';
    for (char ch : data_) {
        switch (ch) {
            case '&': out << "&amp;"; break;
            case '\'': out << "&apos;"; break;
            case '"': out << "&quot;"; break;
            case '<': out << "&lt;"; break;
            case '>': out << "&gt;"; break;
            default: out << ch; break;
        }
    }
    out << "</text>"sv;
}


// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
    for (const auto& obj: objects_) {
        obj->Render(ctx);
    }
    out << "</svg>";
}

}  // namespace svg
