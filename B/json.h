#pragma once
#include "transport_catalogue.h"
#include "geo.h"
#include "map_renderer.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <deque>
#include <type_traits>
#include <algorithm>
#include <unordered_map>
#include <set>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Data = variant;
        Node() = default;

        Node(Array array) : data_(std::move(array)) {}
        Node(Dict map) : data_(std::move(map)) {}
        Node(int value) : data_(value) {}
        Node(std::string value) : data_(std::move(value)) {}
        Node(double value) : data_(value) {}
        Node(bool value) : data_(value) {}

        const Array& AsArray() const
        {
            if(std::holds_alternative<Array>(data_)) {
                return std::get<Array>(data_);
            }
            else throw std::logic_error("Array Parsing Error");
        }
        const Dict& AsMap() const
        {
            if(std::holds_alternative<Dict>(data_)) {
                return std::get<Dict>(data_);
            }
            else throw std::logic_error("Map Parsing Error");
        }
        int AsInt() const
        {
            if(std::holds_alternative<int>(data_)) {
                return std::get<int>(data_);
            }
            else throw std::logic_error("Int Parsing Error");
        }
        const std::string& AsString() const
        {
            if(std::holds_alternative<std::string>(data_)) {
                return std::get<std::string>(data_);
            }
            else throw std::logic_error("String Parsing Error");
        }
        double AsDouble() const
        {
            if (std::holds_alternative<double>(data_)) return std::get<double>(data_);
            else if (std::holds_alternative<int>(data_)) return std::get<int>(data_);
            else throw std::logic_error("Int/Double Parsing Error");
        }
        bool AsBool() const
        {
            if(std::holds_alternative<bool>(data_)) {
                return std::get<bool>(data_);
            }
            else throw std::logic_error("Bool Parsing Error");
        }

        bool IsInt() const
        {
            return std::holds_alternative<int>(data_);
        }
        bool IsNull() const
        {
            return std::holds_alternative<std::nullptr_t>(data_);
        }
        bool IsDouble() const
        {
            if(IsInt() || IsPureDouble()) { return true; }
            return false;
        }
        bool IsPureDouble() const
        {
            return std::holds_alternative<double>(data_);
        }
        bool IsString() const
        {
            return std::holds_alternative<std::string>(data_);
        }
        bool IsArray() const
        {
            return std::holds_alternative<Array>(data_);
        }
        bool IsMap() const
        {
            return std::holds_alternative<Dict>(data_);
        }
        bool IsBool() const
        {
            return std::holds_alternative<bool>(data_);
        }

        bool operator== (const Node& node) const
        {
            return data_ == node.data_;
        }
        bool operator!= (const Node& node) const
        {
            return data_ != node.data_;
        }

    private:
        Data data_;
    };

    Node LoadNode(std::istream& input);

    Node LoadArray(std::istream& input);

    using Number = std::variant<int, double>;

    Number LoadNumber(std::istream& input);

    Node LoadString(std::istream& input);

    Node LoadDict(std::istream& input);

    Node LoadNull(std::istream& input);

    Node LoadBool(std::istream& input);

    Node LoadNode(std::istream& input);

    class Document {
    public:
        explicit Document(Node root) : root_(std::move(root)) {}

        const Node& GetRoot() const
        {
            return root_;
        }

        bool operator== (const Document& doc)
        {
            return root_ == doc.root_;
        }

        bool operator!= (const Document& doc)
        {
            return root_ != doc.root_;
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);
}  // namespace json

class FromMain {
public:
    struct Base {
    public:
        std::string type = "";
        std::string name = "";
        std::vector<std::string> stops = {};
        bool round = false;
        double latitude = 0.0;
        double longitude = 0.0;
        std::vector<std::pair<int, std::string>> road_distances = {};
        
        void set_type(const std::string& value) {
            type = value;
        }
        
        void set_name(const std::string& value) {
            name = value;
        }
        
        void set_stops(const std::vector<json::Node>& value) {
            for(const auto& val : value) {
                std::string tmp = val.AsString();
                stops.push_back(tmp);
            }
        }
        
        void is_round(bool value) {
            round = value;
        }
        
        void set_latitude(const double& value) {
            latitude = value;
        }
        
        void set_longitude(const double& value) {
            longitude = value;
        }
        
        void set_road_distances(const std::map<std::string, json::Node>& value) {
            for(const auto& [data, val] : value) {
                int tmp = val.AsInt();
                road_distances.push_back(std::pair<int, std::string>(tmp, data));
            }
        }
    };
    
    void ParseRequests(std::istream& input) {
        std::unordered_map<std::string, Bus> index_buses_;
        json::Node node = std::move(json::LoadNode(input));
        std::map<std::string, json::Node> request_handler = node.AsMap();
        
        std::vector<json::Node> base_req = request_handler["base_requests"].AsArray();
        std::map<std::string, json::Node> render_settings = request_handler["render_settings"].AsMap();
        
        std::unordered_map<std::string, Coordinates> geo_coords;
        std::deque<Coordinates> geo_coords_v;
        std::deque<std::string> buses_v;
        std::deque<std::string> bus_stops;
        std::deque<std::string> stops_d;
        
        for(const auto& b_r : base_req) {
            Base base;
            std::map<std::string, json::Node> request = b_r.AsMap();
            
            base.set_type(request["type"].AsString());
            
            base.set_name(request["name"].AsString());
            
            if(base.type == "Stop") {
                base.set_latitude(request["latitude"].AsDouble());

                base.set_longitude(request["longitude"].AsDouble());
                
                geo_coords[base.name] = {base.latitude, base.longitude};
                
                //stops_d.push_back(base.name);
            }
            else {
                base.set_stops(request["stops"].AsArray());
                
                base.is_round(request["is_roundtrip"].AsBool());
                
                for(const auto& stop : base.stops) {
                    bus_stops.push_back(stop);
                }
                
                buses_v.push_back(base.name);
                
                index_buses_[base.name] = {base.name, base.round, base.stops};
                //ct.AddBus(base.name, base.round, base.stops);
            }
                
            // AddToCt(base);
        }
        //////////////////////////////////////////////////////////
        double width = render_settings["width"].AsDouble(); // for renderer
        double height = render_settings["height"].AsDouble(); // for renderer
        double padding = render_settings["padding"].AsDouble(); // for renderer
        double line_width = render_settings["line_width"].AsDouble(); // stroke-width
        double font_size = render_settings["stop_label_font_size"].AsDouble();
        double underlayer_width = render_settings["underlayer_width"].AsDouble();
        double stop_radius = render_settings["stop_radius"].AsDouble();
        std::vector<json::Node> stop_offset = render_settings["stop_label_offset"].AsArray();
        std::vector<json::Node> underlayer_color = render_settings["underlayer_color"].AsArray();
        std::vector<json::Node> bus_offset = render_settings["bus_label_offset"].AsArray();
        std::vector<json::Node> color_palette = render_settings["color_palette"].AsArray(); // stroke="rgb(255,160,0)"
        
        /*<text fill="rgba(255,255,255,0.85)" stroke="rgba(255,255,255,0.85)" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" x="550" y="190.051" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">14</text>
  <text fill="rgb(255,160,0)" x="550" y="190.051" dx="7" dy="15" font-size="20" font-family="Verdana" font-weight="bold">14</text>*/
        
        svg::Document d;
        
        for(const auto& b_s : bus_stops) {
            geo_coords_v.push_back(geo_coords[b_s]); // Delete useless stops
            // circles.Add(svg::Circle().SetRadius(stop_radius).SetCenter({}));
            // names.Add(svg::Text());
            
        }
        
        std::vector<std::string> result; // FOR RGBA
        
        //////////////////////////////////////////////////////////////
        for(const auto& node : color_palette) {
            try {
                std::vector<json::Node> tmp_arr = node.AsArray();
                std::stringstream strm;
                bool first = true;
                
                strm << "rgb";
                if(tmp_arr.size() != 3) strm << "a";
                strm << "(";
                    
                for(const auto& data : tmp_arr) {
                    if(first) {strm << data.AsInt(); first = false;} /////////////////////////////// RGBA
                    else {
                        if(data.IsInt()) strm << "," << data.AsInt();
                        else strm << "," << data.AsDouble();
                    }
                }
                strm << ")";
                
                result.push_back(strm.str());
            } catch(...) {}
            
            try {
                std::string tmp = node.AsString();
                result.push_back(tmp);
            } catch(...) {}
        }
        
        std::string result_f = "";
            try {
                std::stringstream strm;
                bool first = true;
                
                strm << "rgb";
                if(underlayer_color.size() != 3) strm << "a";
                strm << "(";
                    
                for(const auto& data : underlayer_color) {
                    if(first) {strm << data.AsInt(); first = false;} /////////////////////////////// RGBA
                    else {
                        if(data.IsInt()) strm << "," << data.AsInt();
                        else strm << "," << data.AsDouble();
                    }
                }
                strm << ")";
                
                result_f = strm.str();
            } catch(...) {}
        ///////////////////////////////////////////////////////////////
        
        
        const SphereProjector sp{geo_coords_v.begin(), geo_coords_v.end(), width, height, padding};
        size_t color_s = 0;
        
        SortByName(buses_v);
        SortByName(bus_stops);
        //bus_stops
        // std::reverse(stops_d.begin(), stops_d.end());
        
        for(const auto& bus : buses_v) {
            auto info = index_buses_[bus];
            
            if(info.stops.size() != 0) {
                svg::Polyline polyline;
                polyline.SetFill("none").SetStroke(result[color_s]).SetWidth(line_width).SetCap("round").SetJoin("round");
                std::vector<svg::Point> point_handler;
                
                for(const auto& stop : info.stops) {
                    point_handler.push_back(sp(geo_coords[stop]));
                    // stops_d.push_back(stop);
                }
                
                for(const auto& p_h : point_handler) {
                    polyline.AddPoint(p_h);
                }
                
                if(!info.is_round) {
                    point_handler.pop_back();
                    std::reverse(point_handler.begin(), point_handler.end());
                    
                    for(const auto& p_h : point_handler) {
                        polyline.AddPoint(p_h);
                    }
                }
                
                d.Add(polyline);
                
                if(result.size() - 1 <= color_s) color_s = 0;
                else color_s++;
            }
            
        }
        
        color_s = 0;
        std::set<std::string> itog;
        
        for(const auto& i : bus_stops) {
            itog.insert(i);
        }
        
        for(const auto& bus : buses_v) {
            auto info = index_buses_[bus];
            
            if(info.stops.size() != 0) {
                std::vector<svg::Point> point_handler;
                
                for(const auto& stop : info.stops) {
                    point_handler.push_back(sp(geo_coords[stop]));
                }
                
                if(info.is_round) {
                     d.Add(svg::Text().SetPosition(point_handler[0]).SetFontWeight("bold").SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({bus_offset[0].AsDouble(), bus_offset[1].AsDouble()}).SetData(info.name).SetWidth(underlayer_width).SetFill(result_f).SetStroke(result_f).SetCap("round"));
                    d.Add(svg::Text().SetPosition(point_handler[0]).SetFontWeight("bold").SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({bus_offset[0].AsDouble(), bus_offset[1].AsDouble()}).SetData(info.name).SetFill(result[color_s]));
                }
                
                if(!info.is_round) {
                    for(const auto& p_h : point_handler) {
                        d.Add(svg::Text().SetPosition(p_h).SetFontWeight("bold").SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({bus_offset[0].AsDouble(), bus_offset[1].AsDouble()}).SetData(info.name).SetWidth(underlayer_width).SetFill(result_f).SetStroke(result_f).SetCap("round"));
                        d.Add(svg::Text().SetPosition(p_h).SetFontWeight("bold").SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({bus_offset[0].AsDouble(), bus_offset[1].AsDouble()}).SetData(info.name).SetFill(result[color_s]));
                    }
                }
                
                if(result.size() - 1 <= color_s) color_s = 0;
                else color_s++;
            }
            
        }
        
        for(const auto& stop : itog) {
            d.Add(svg::Circle().SetRadius(stop_radius).SetCenter(sp(geo_coords[stop])));
        }
        
        for(const auto& stop : itog) {
            d.Add(svg::Text().SetPosition(sp(geo_coords[stop])).SetFill(result_f).SetWidth(underlayer_width).SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({stop_offset[0].AsDouble(), stop_offset[1].AsDouble()}).SetData(stop).SetCap("round").SetStroke(result_f));
            d.Add(svg::Text().SetPosition(sp(geo_coords[stop])).SetFill("black").SetWidth(underlayer_width).SetFontFamily("Verdana").SetFontSize(font_size).SetOffset({stop_offset[0].AsDouble(), stop_offset[1].AsDouble()}).SetData(stop));
        }
        
        d.Render(std::cout);
    }
};
