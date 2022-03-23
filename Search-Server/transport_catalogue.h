#pragma once

#include "geo.h"

#include <algorithm>
#include <iomanip>
#include <deque>
#include <functional>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct Stop
{
    std::string name{};
    Coordinates cords{};
};

struct Bus
{
    std::string name;
    bool is_round;
    std::vector<std::string> stops;
};

struct Stops_for_Bus
{
    std::string name;
    std::deque<std::string> buses;
};

struct InfoAboutBus 
{
    std::string name;
    size_t stops_on_route;
    size_t unique_stops;
    double route_lenght;
    double curvature;
};

void SortByName(std::deque<std::string>& container);

std::ostream& operator<<(std::ostream& out, const InfoAboutBus& bus);

std::ostream& operator<<(std::ostream& out, Stops_for_Bus& stop);

class Catalogue {
public:
    void AddBus (const std::string& name, const bool& type_of_move, const std::vector<std::string> stops)
    {
        buses_m.emplace(std::pair<std::string, Bus>(name, {name, type_of_move, stops}));
        
        std::deque<std::string> skip;
        
        for(const auto& stop : stops)
        {
            bool is_used = false;
            for(const auto& i : skip) 
            {
                if(i == stop) {is_used = true; break;}
            }
            if(!is_used) {
                stops_for_bus_m[stop].push_back(name);
                skip.push_back(stop);
            }
        }
    }
    
    void AddStop (const std::string& name, double x_cord, double y_cord, const std::vector<std::pair<int, std::string>>& distance = {})
    {
        stops_m.emplace(std::pair<std::string, Stop>(name, {name, {x_cord, y_cord}}));
        
        stops_for_bus_m[name];
        
        for(const auto& [len, name_stop] : distance)
        {
            AddDistanceBetweenStops(len, name_stop, name);
        }
    }
    
    void AddDistanceBetweenStops (const int& lenght, const std::string& name, const std::string& from_stop_name)
    {
        dist_btw_stops[from_stop_name].push_back({lenght, name});
    }
    
    int GetDistanceBetweenStops (const std::string& from, const std::string& to)
    {
        for(const auto& i : dist_btw_stops[from])
        {
            if(i.second == to) return i.first;
        }
        for(const auto& i : dist_btw_stops[to])
        {
            if(i.second == from) return i.first;
        }
        return 0;
    }
     
    InfoAboutBus GetInfoAboutBus (const std::string& name)
    {
        Bus this_bus;
        InfoAboutBus empty_info;
        
        this_bus = buses_m[name];
        
        if(this_bus.name != std::string{}) {
            ////////////////////////////////////////////////
            std::unordered_set<std::string_view> uniq_stops;

            for (const auto& stop : this_bus.stops) {
                uniq_stops.insert(stops_m[stop].name);
            }
            ////////////////////////////////////////////////
            size_t stops_route = this_bus.stops.size();

            if (!this_bus.is_round) {
                stops_route = this_bus.stops.size() * 2 - 1;
            }
            ////////////////////////////////////////////////
            double rout_len = 0.0;
            bool is_first = true;
            Coordinates prev;
            for(const auto& stop : this_bus.stops)
            {
                if(is_first) {prev = stops_m[stop].cords; is_first = false;}
                else {
                    rout_len += ComputeDistance(prev, stops_m[stop].cords);
                    prev = stops_m[stop].cords;
                }
            }

            if (!this_bus.is_round) {
                rout_len *= 2.0;
            }

            double real_rout_len = 0.0;
            bool is_frst = true;
            std::string prev_stop;
            
            for(const auto& it : this_bus.stops)
            {
                if(is_frst) {is_frst = false; prev_stop = it; continue;}
                real_rout_len += GetDistanceBetweenStops(prev_stop, it);
                prev_stop = it;
            }

            if (!this_bus.is_round) {
                is_frst = true;
                prev_stop.clear();
                for(auto it = this_bus.stops.rbegin(); it != this_bus.stops.rend(); it++)
                {
                    if(is_frst) {is_frst = false; prev_stop = *it; continue;}
                    real_rout_len += GetDistanceBetweenStops(prev_stop, *it);
                    prev_stop = *it;
                }
            }
            
            double curvature = real_rout_len / rout_len;
            
            return {
                    name,
                    stops_route,
                    uniq_stops.size(),
                    real_rout_len,
                    curvature
                   };
        }
        return empty_info;
    }
    
    Stops_for_Bus GetInfoAboutStop (const std::string& name)
    {
        return {(stops_m.count(name) != 0) ? name : std::string{}, stops_for_bus_m[name]};
    }
    
private:
    std::unordered_map<std::string, Bus> buses_m;
    
    std::unordered_map<std::string, Stop> stops_m;
    
    std::unordered_map<std::string, std::deque<std::string>> stops_for_bus_m;
    
    std::unordered_map<std::string, std::deque<std::pair<int, std::string>>> dist_btw_stops;
};
