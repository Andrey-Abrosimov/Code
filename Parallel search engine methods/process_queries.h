#pragma once
#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <functional>
#include <execution>
#include <list>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> answ;
    answ.resize(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), answ.begin(), [&search_server] (std::string hlp){
                   return search_server.FindTopDocuments(hlp);});
    return answ;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> hlp = ProcessQueries(search_server, queries);
 
    std::vector<Document> answ;
    for (const auto& h : hlp)
    {
        answ.insert(answ.end(), h.begin(), h.end());
    }
    return answ;
}
