#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <deque>
#include <type_traits>

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
            // std::map<int, std::string> r_d;
            for(const auto& [data, val] : value) {
                int tmp = val.AsInt();
                // std::cout << "Name is: " << name << ", " << data << " " << tmp << std::endl;
                // r_d[tmp] = data;
                road_distances.push_back(std::pair<int, std::string>(tmp, data));
                // std::cout << r_d[tmp] << std::endl;
            }
            // std::cout << value.size() << std::endl;
            // for(const auto& [_, _s] : r_d) {
                // std::cout << _ << " " << _s << std::endl;
            // }
        }
        
        void clear() {
            type = "";
            name = "";
            stops = {};
            round = false;
            latitude = 0.0;
            longitude = 0.0;
            road_distances = {};
        }
    };
    
    void AddToCt(const Base& base) {
        if(base.type == "Stop") {
            ct.AddStop(base.name, base.latitude, base.longitude, base.road_distances);
            //for(const auto& [_, _s] : base.road_distances) {
              //  std::cout << _ << " " << _s << std::endl;
            //}
        }
        else if(base.type == "Bus") {
           ct.AddBus(base.name, base.round, base.stops);
        }
    }
        
    struct Stat {
        int id = 0;
        std::string type = "";
        std::string name = "";
        
        void set_id(int value) {
            id = value;
        }
        
        void set_type(const std::string& value) {
            type = value;
        }
        
        void set_name(const std::string& value) {
            name = value;
        }
        
        void clear() {
            id = 0;
            type = "";
            name = "";
        }
        
        bool operator== (const Stat& stat)
        {
            return id == stat.id && type == stat.type && name == stat.name;
        }
    };
    
    void ParseRequests(std::istream& input) {
        json::Node node = json::LoadNode(input);
        
        std::map<std::string, json::Node> request_handler = node.AsMap();
        
        std::vector<json::Node> base_req = request_handler["base_requests"].AsArray();
        
        std::vector<json::Node> stat_req = request_handler["stat_requests"].AsArray();
        
        for(const auto& b_r : base_req) {
            Base base;
            std::map<std::string, json::Node> request = b_r.AsMap();
            
            for(const auto& [data, value] : request) {
                if(data == "type") {
                    base.set_type(value.AsString());
                }
                else if(data == "name") {
                    base.set_name(value.AsString());
                }
                else if(data == "stops") {
                    base.set_stops(value.AsArray());
                }
                else if(data == "is_roundtrip") {
                    base.is_round(value.AsBool());
                }
                else if(data == "latitude") {
                    base.set_latitude(value.AsDouble());
                }
                else if(data == "longitude") {
                    base.set_longitude(value.AsDouble());
                }
                else if(data == "road_distances") {
                    base.set_road_distances(value.AsMap());
                }
            }
            
            AddToCt(base);
        }
        
        std::deque<Stat> stat_d;
        
        for(const auto& s_r : stat_req) {
            Stat stat;
            std::map<std::string, json::Node> request = s_r.AsMap();
            
            for(const auto& [data, value] : request) {
                if(data == "id") {
                    stat.set_id(value.AsInt());
                }
                else if(data == "type") {
                    stat.set_type(value.AsString());
                }
                else if(data == "name") {
                    stat.set_name(value.AsString());
                }
            }
            
            stat_d.push_back(stat);
        }
        
        std::cout << "[" << std::endl;
        
        for(const auto& stat : stat_d) {
            if (stat.type == "Stop") {
                auto info = ct.GetInfoAboutStop(stat.name);
                std::cout << "    {" << std::endl;
                if(info.name!=std::string{})
                {
                        std::cout << "        \"buses\": [" << std::endl;
                        if(!info.buses.empty()) {
                        std::cout << "            ";
                        SortByName(info.buses);
                            for (size_t i = 0; i < info.buses.size(); i++) {
                                if(i == info.buses.size() - 2 && info.buses.size() > 1)
                                    std::cout << "\"" << info.buses[i] << "\", ";
                                else
                                    std::cout << "\"" << info.buses[i] << "\"" << std::endl;
                            }
                        }
                        std::cout << "        ]," << std::endl;
                        std::cout << "        \"request_id\": " << stat.id << std::endl;
                }
                else
                {
                    std::cout << "        \"request_id\": " << stat.id << "," << std::endl;
                    std::cout << "        \"error_message\": \"not found\"" << std::endl;
                }
                ////////////////////////////////////////////////////
                
                if(stat_d[stat_d.size() - 1] == stat)
                    std::cout << "    }" << std::endl;
                else
                    std::cout << "    }," << std::endl;
            }
            else {
                auto info = ct.GetInfoAboutBus(stat.name);
                
                std::cout << "    {" << std::endl;
                if (info.name!=std::string{}) {
                    std::cout << "        \"curvature\": " << info.curvature << "," << std::endl;
                    std::cout << "        \"request_id\": " << stat.id << "," << std::endl;
                    std::cout << "        \"route_length\": " << info.route_lenght << "," << std::endl;
                    std::cout << "        \"stop_count\": " << info.stops_on_route << "," << std::endl;
                    std::cout << "        \"unique_stop_count\": " << info.unique_stops << std::endl;
                }
                else {
                    std::cout << "        \"request_id\": " << stat.id << "," << std::endl;
                    std::cout << "        \"error_message\": \"not found\"" << std::endl;
                }
                
                if(stat_d[stat_d.size() - 1] == stat)
                    std::cout << "    }" << std::endl;
                else
                    std::cout << "    }," << std::endl;
            }
        }
        
        std::cout << "]" << std::endl;
    }
    
private:
    Catalogue ct;
};
