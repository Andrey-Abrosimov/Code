#pragma once

#define _USE_MATH_DEFINES

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cmath>

namespace svg {
    using namespace std::literals;
    
    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };
    
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };
    
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    
    class Circle final : public Object {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0.0, 0.0 };
        double radius_ = 1.0;
    };
    
    class Polyline final : public Object {
    public:        
        Polyline& AddPoint(Point point);
        
        Polyline& SetFill(const std::string& data);
    
        Polyline& SetStroke(const std::string& data);
    
        Polyline& SetWidth(double data);
    
        Polyline& SetCap(const std::string& data);
    
        Polyline& SetJoin(const std::string& data);

        
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
        std::string fill = "none";
        std::string stroke = "none";
        double width = 0.0;
        std::string cap = "round";
        std::string join = "round";
    };
    
    class Text final : public Object {
    public:     
        Text& AddPoint(Point point);
        
        Text& SetFill(const std::string& data);
    
        Text& SetStroke(const std::string& data);
    
        Text& SetWidth(double data);
    
        Text& SetCap(const std::string& data);
    
        Text& SetJoin(const std::string& data);
        
        Text& SetPosition(Point pos);
        
        Text& SetOffset(Point offset);
        
        Text& SetFontSize(uint32_t size);
        
        Text& SetFontFamily(std::string font_family);
        
        Text& SetFontWeight(std::string font_weight);
        
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::string fill = "none";
        std::string stroke = "none";
        double width = 0.0;
        std::string cap;
        std::string join = "round";
        
        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";
    };

    class ObjectContainer {
    public:        
        template <typename T>
        void Add(T obj) {
            objects_.emplace_back(std::make_unique<T>(std::move(obj)));
        }

        virtual ~ObjectContainer() = default;
    protected:        
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }        

        void Render(std::ostream& out) const;
    };
    
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };
}
