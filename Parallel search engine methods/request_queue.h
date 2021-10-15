#pragma once
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

class RequestQueue {
public:
    RequestQueue(const SearchServer& search_server) : server(search_server) {
    }
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        QueryResult result;
        result.found_documents=server.FindTopDocuments(raw_query, document_predicate);
        if(!requests_.empty()){           
           while(requests_.size() >= sec_in_day_){
                if(requests_.front().IsEmpty){               
                    Empty_Count--;
                }
                requests_.pop_front();
            }
        }
        if(result.found_documents.empty()){
            result.IsEmpty=true;
            Empty_Count++;
        }
        requests_.push_back(result);
        return result.found_documents;
    }
 
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
    }
 
    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }
 
    int GetNoResultRequests() const {
        return Empty_Count;
    }
private:
    struct QueryResult {
        std::vector<Document> found_documents; 
        bool IsEmpty=false;
    };
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    const SearchServer& server;
    int Empty_Count=0;
};
