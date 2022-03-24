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
#include <cmath>

struct Coordinates {
    double lat;
    double lng;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

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

void SortByName(std::deque<std::string>& container) {
	sort(container.begin(), container.end(),
			[](const std::string& lhs, const std::string& rhs) {
				return lhs < rhs;
			}
	);
}

std::ostream& operator<<(std::ostream& out, const InfoAboutBus& bus) {
    out << "Bus " << bus.name << ": ";
    out << bus.stops_on_route << " " << "stops on route" << ", ";
    out << bus.unique_stops << " " << "unique stops" << ", ";
    out << std::setprecision(6) << bus.route_lenght << " " << "route length" << ", ";
    out << std::setprecision(6) << bus.curvature << " " << "curvature";

    return out;
}

std::ostream& operator<<(std::ostream& out, Stops_for_Bus& stop) {
    out << "Stop " << stop.name << ": buses";
    SortByName(stop.buses);
    for(auto it = stop.buses.begin(); it!=stop.buses.end(); it++)
    {
        out << " " << *it;
    }
    
    return out;
}

    class ParserAndInfo {
    public:
        ParserAndInfo() = default;
        
        size_t FindNextPos (const std::string& str, char to_find) {
            size_t counter = 0;
            
            for (size_t i = 0; i < str.size(); i++) {
                if (str[i] == to_find) {
                    counter++;
                    if(counter == 2) {
                        return i;
                    }
                }
            }
            return 0;
        }

        std::string RemoveSpacesInBeginMiddleAndEnd (const std::string& str) {  
            std::string answer = str;
            answer.erase(0, answer.find_first_not_of(' '));
            size_t j = 0;

            for (size_t i = answer.size() - 1; i > 0; i--) {
                if (answer[i] == ' ')  j++;
                else break;
            }

            answer.erase(answer.size() - j);
            j = 0;

            for (size_t i = answer.find(':') - 1; i > 0; i--) {
                if (answer[i] == ' ')  j++;
                else break;
            }

            if(j != 0)
            answer.erase(answer.find(':') - j, j);
            j = 0;

            for (size_t i = answer.find(':') + 1; i < answer.size(); i++) {
                if (answer[i] == ' ')  j++;
                else break;
            }

            if(j != 0)
            answer.erase(answer.find(':') + 1, j);

            return answer;
        }

        size_t FindLastSymbol (const std::string& str, char c) {  
            for (size_t i = str.size() - 1; i != 0; i--) {
                if (str[i] == c) {
                    return i;
                }
            }

            return 0;
        }

        size_t FindStringInString (const std::string& str, const std::string& to_find) {
            size_t answer = 0;
            for (size_t i = 0; i < str.size(); i++) {
                if (str[i] == to_find[0]) { 
                    answer = i;

                    for (size_t j = 0; j < to_find.size(); j++) {
                        if (to_find[j] != str[i+j]) {answer = 0; break;}
                        if(j == to_find.size()-1) {return answer;}
                    }
                }
            }

            return answer;
        }

        std::string RemoveSymbols (const std::string& str, char ch) {
            std::string answer;
            for (const auto& s : str) {
                if(s != ch) answer += s;
            }
            return answer;
        }

        void ParseBaseRequest (const std::string& str) {
            if (FindStringInString(str, "\"Stop\"")) {
                std::string name{};
                if (FindStringInString(str, "\"name\"")) {
                    size_t pos_of_name = FindStringInString(str, "\"name\"");
                    std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_name, str.substr(pos_of_name).find_first_of(',')));
                    request = RemoveSymbols(request, '"');
                    name = request.erase(0, request.find_first_of(':') + 1);
                }
                double latitude = 0.0;
                if (FindStringInString(str, "\"latitude\"")) {
                    size_t pos_of_latitude = FindStringInString(str, "\"latitude\"");
                    std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_latitude, str.substr(pos_of_latitude).find_first_of(',')));
                    request = RemoveSymbols(request, '"');
                    latitude = std::stod(request.erase(0, request.find_first_of(':') + 1));
                }
                double longitude = 0.0;
                if (FindStringInString(str, "\"longitude\"")) {
                    size_t pos_of_longitude = FindStringInString(str, "\"longitude\"");
                    std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_longitude, str.substr(pos_of_longitude).find_first_of(',')));
                    request = RemoveSymbols(request, '"');
                    longitude = std::stod(request.erase(0, request.find_first_of(':') + 1));

                }
                std::vector<std::pair<int, std::string>> road_distances{};
                if (FindStringInString(str, "\"road_distances\"")) {
                    size_t pos_of_road_distances = FindStringInString(str, "\"road_distances\"");
                    std::string request = str.substr(pos_of_road_distances, str.substr(pos_of_road_distances).find_first_of('}') + 1);
                    std::string names_start = request.substr(request.find_first_of('{')  + 1, request.find_first_of('}') - request.find_first_of('{') - 1);
                    size_t n = std::count(names_start.begin(), names_start.end(), ',');
                    size_t o = (std::count(names_start.begin(), names_start.end(), '"') > 0) ? 1 : 0;
                    for (size_t i = 0; i < n + o; i++) {
                        if(i == n) {
                            road_distances.push_back(std::pair<int, std::string>(std::stoi(RemoveSymbols(names_start.substr(names_start.find_first_of(':') + 1), ' ')), names_start.substr(names_start.find_first_of('"') + 1, FindNextPos(names_start, '"') - names_start.find_first_of('"') - 1)));
                        }
                        else {
                            road_distances.push_back(std::pair<int, std::string>(std::stoi(RemoveSymbols(names_start.substr(names_start.find_first_of(':') + 1, names_start.find_first_of(',') - names_start.find_first_of(':') - 1), ' ')), names_start.substr(names_start.find_first_of('"') + 1, FindNextPos(names_start, '"') - names_start.find_first_of('"') - 1)));
                            names_start = names_start.substr(names_start.find_first_of(',') + 1);
                        }
                    }
                }
                
                ct.AddStop(name, latitude, longitude, road_distances);
            }
            else {
                std::string name;
                if (FindStringInString(str, "\"name\"")) {
                    size_t pos_of_name = FindStringInString(str, "\"name\"");
                    std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_name, str.substr(pos_of_name).find_first_of(',')));
                    request = RemoveSymbols(request, '"');
                    name = request.erase(0, request.find_first_of(':') + 1);
                }
                std::vector<std::string> stops;
                if (FindStringInString(str, "\"stops\"")) {
                    size_t pos_of_stops = FindStringInString(str, "\"stops\"");
                    std::string request = str.substr(pos_of_stops, str.substr(pos_of_stops).find_first_of(']') + 1);
                    std::string names_start = request.substr(request.find_first_of('[')  + 1, request.find_first_of(']') - request.find_first_of('[') - 1);
                    while (true) {
                        if (FindNextPos(names_start, '"') == 0) break;
                        else {
                            stops.push_back(names_start.substr(names_start.find_first_of('"') + 1, FindNextPos(names_start, '"') - names_start.find_first_of('"') - 1)); 
                            names_start = names_start.substr(FindNextPos(names_start, '"') + 1);
                        }
                    }
                }
                bool is_roundtrip;
                if (FindStringInString(str, "\"is_roundtrip\"")) {
                    if (FindStringInString(str, "false"))
                        is_roundtrip = false;
                    else if (FindStringInString(str, "true"))
                        is_roundtrip = true;
                }
                ct.AddBus(name, is_roundtrip, stops);
            }
        }

        void ParseBaseRequests (const std::string& str) {
            std::string tmp = str;

            while (true) {
                if(FindStringInString(tmp, "},") == 0) break;
                else {
                    ParseBaseRequest(tmp.substr(0, FindStringInString(tmp, "},") + 1));
                    tmp = tmp.substr(tmp.substr(FindStringInString(tmp, "},")).find_first_of('{') + FindStringInString(tmp, "},"));
                }
            }
            ParseBaseRequest(tmp);
        }
        
        void ParseStatRequest (const std::string& str, bool is_last = false) {
            int id = 0;
            
            if (FindStringInString(str, "\"id\"")) {
                size_t pos_of_id = FindStringInString(str, "\"id\"");
                std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_id, str.substr(pos_of_id).find_first_of(',')));
                request = RemoveSymbols(request, '"');
                id = std::stoi(request.erase(0, request.find_first_of(':') + 1));
            }
            std::string name{};
            if (FindStringInString(str, "\"name\"")) {
                size_t pos_of_name = FindStringInString(str, "\"name\"");
                std::string request = RemoveSpacesInBeginMiddleAndEnd(str.substr(pos_of_name, FindLastSymbol(str.substr(pos_of_name), '"')));
                request = RemoveSymbols(request, '"');
                name = request.erase(0, request.find_first_of(':') + 1);
            }
            if (FindStringInString(str, "\"Stop\"")) {
                auto info = ct.GetInfoAboutStop(name);
                std::cout << "    {" << std::endl;
                if(info.name!=std::string{})
                {
                        std::cout << "        \"buses\": [" << std::endl;
                        std::cout << "            ";
                        SortByName(info.buses);
                        for (size_t i = 0; i < info.buses.size(); i++) {
                            if(i == info.buses.size() - 2 && info.buses.size() > 1)
                                std::cout << "\"" << info.buses[i] << "\", ";
                            else
                                std::cout << "\"" << info.buses[i] << "\"" << std::endl;
                        }
                        std::cout << "        ]," << std::endl;
                        std::cout << "        \"request_id\": " << id << std::endl;
                }
                else
                {
                    std::cout << "        \"error_message\": \"not found\"," << std::endl;
                    std::cout << "        \"request_id\": " << id << std::endl;
                }
                ////////////////////////////////////////////////////
                
                if(is_last)
                    std::cout << "    }" << std::endl;
                else
                    std::cout << "    }," << std::endl;
            }
            else {
                auto info = ct.GetInfoAboutBus(name);
                
                std::cout << "    {" << std::endl;
                if (info.name!=std::string{}) {
                    std::cout << "        \"curvature\": " << info.curvature << "," << std::endl;
                    std::cout << "        \"request_id\": " << id << "," << std::endl;
                    std::cout << "        \"route_length\": " << info.route_lenght << "," << std::endl;
                    std::cout << "        \"stop_count\": " << info.stops_on_route << "," << std::endl;
                    std::cout << "        \"unique_stop_count\": " << info.unique_stops << std::endl;
                }
                else {
                    std::cout << "        \"error_message\": \"not found\"," << std::endl;
                    std::cout << "        \"request_id\": " << id << std::endl;
                }
                
                if(is_last)
                    std::cout << "    }" << std::endl;
                else
                    std::cout << "    }," << std::endl;
            }
        }
        
        void ParseStatRequests (const std::string& str) {
            std::cout << "[" << std::endl;
            std::string tmp = str;

            while (true) {
                if(FindStringInString(tmp, "},") == 0) break;
                else {
                    ParseStatRequest(tmp.substr(0, FindStringInString(tmp, "},") + 1));
                    tmp = tmp.substr(tmp.substr(FindStringInString(tmp, "},")).find_first_of('{') + FindStringInString(tmp, "},"));
                }
            }
            ParseStatRequest(tmp, true);
            std::cout << "]" << std::endl;
        }

        void InputBase (std::istream& input) {
            ////////// Transform to string ////////////
            std::string str;
            int r_c = 0;
            while (true) {
                char c;
                input.get(c);
                str += c;
                if (c == '{') r_c++;
                else if (c == '}') r_c--;
                if (r_c == 0) break;
            }
            ////////// Get requests ///////////////////
            std::string base_req = str.substr(str.substr(str.find_first_of('[')-1).find_first_of('{'), FindStringInString(str, "stat_requests"));
            std::string stat_req = str.substr(FindStringInString(str, "stat_requests"), FindLastSymbol(str, ']') - FindStringInString(str, "stat_requests"));
            base_req = base_req.substr(base_req.find_first_of('{'));
            base_req = base_req.substr(0, FindLastSymbol(base_req, '}') + 1);
            stat_req = stat_req.substr(stat_req.find_first_of('{'));

            ParseBaseRequests(base_req);
            ParseStatRequests(stat_req);
        }
        
    private:
        Catalogue ct;
    };


int main() {
    ParserAndInfo pai;
    pai.InputBase(std::cin);
    
    return 0;
}
