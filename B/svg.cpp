#include "svg.h"

#include <iomanip>
#include <algorithm>
#include <cmath>

namespace svg {

    using namespace std::literals;
    
    void Object::Render(const RenderContext& context) const {
        //context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << "\n"sv;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        std::ostream& out = context.out;
        out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" fill=\"white" << "\""sv;
        out << "/>"sv;
    }


    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }
    
    Polyline& Polyline::SetFill(const std::string& data) {
        fill = data;
        return *this;
    }
    
    Polyline& Polyline::SetStroke(const std::string& data) {
        stroke = data;
        return *this;
    }
    
    Polyline& Polyline::SetWidth(double data) {
        width = data;
        return *this;
    }
    
    Polyline& Polyline::SetCap(const std::string& data) {
        cap = data;
        return *this;
    }
    
    Polyline& Polyline::SetJoin(const std::string& data) {
        join = data;
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        std::ostream& out = context.out;
        out << "  <polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ","sv << points_[i].y;
            if (i != points_.size() - 1) {
                out << " "sv;
            }
        }
        out << "\""sv;
        out << " fill=\"" << fill << "\"" << " stroke=\"" << stroke << "\"" << " stroke-width=\"" << width << "\"" << " stroke-linecap=\"" << cap << "\"" << " stroke-linejoin=\"" << join << "\"";
        out << "/>"sv;
    }

    // ---------- Text ------------------
    Text& Text::SetFill(const std::string& data) {
        fill = data;
        return *this;
    }
    
    Text& Text::SetStroke(const std::string& data) {
        stroke = data;
        return *this;
    }
    
    Text& Text::SetWidth(double data) {
        width = data;
        return *this;
    }
    
    Text& Text::SetCap(const std::string& data) {
        cap = data;
        return *this;
    }
    
    Text& Text::SetJoin(const std::string& data) {
        join = data;
        return *this;
    }
    
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
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
        std::ostream& out = context.out;
        out << "  <text"sv;
        out << " fill=\"" << fill << "\"";
        if(width != 0.0 && !cap.empty()) {
            out << " stroke=\"" << stroke << "\"" << " stroke-width=\"" << width << "\"" << " stroke-linecap=\"" << cap << "\"" << " stroke-linejoin=\"" << join << "\"";
        }
        
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (!font_family_.empty()) {
            out << "font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv << data_ << "</text>"sv;
    }

    // ---------- Document ------------------
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv ;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv ;
        for (size_t i = 0; i < objects_.size(); ++i) {
            objects_[i]->Render({ out, 2, 2 });
        }
        out << "</svg>"sv;
    }
}  // namespace svg
