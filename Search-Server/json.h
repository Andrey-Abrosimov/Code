#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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
