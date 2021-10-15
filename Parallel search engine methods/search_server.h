#pragma once
#include "document.h"
#include "string_processing.h"

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
#include <string_view>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid");
        }
    }
    
    SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor
                                                         // from string container
    {
    }
    
    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw std::invalid_argument("Invalid document_id");
        }
        const auto words = SplitIntoWordsNoStop(static_cast<std::string>(document));

        const double inv_word_count = 1.0 / words.size();
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.push_back(document_id);
    }

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const {
        std::string str{raw_query};
        const auto query = ParseQuery(str);

        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                return lhs.rating > rhs.rating;
            } else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
    }

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const {
        return documents_.size();
    }
    
    std::vector<int>::const_iterator begin() const{
        return document_ids_.begin();
    }

    std::vector<int>::const_iterator end() const{
        return document_ids_.end();
    }
    
    const std::map<std::string_view, double> GetWordFrequencies(int document_id) const {
        std::map<std::string_view, double> answer;
        bool next=false;
        for(const auto& s:document_ids_)
        {
            if(s==document_id)
            {next=true;}
        }
        if(next)
        {
            std::vector<std::string> k;
            for(const auto& [x, z]:word_to_document_freqs_)
            {
                for(const auto& [zzz, _]:z)
                {
                    if(zzz==document_id)
                    {
                        k.push_back(x);
                    }
                }
            }
            for(const auto& f:k)
            {
                if(answer[f])
                {
                    ++answer[f];
                }
                else
                {
                    answer.insert(std::pair<std::string_view, double>(static_cast<std::string_view>(f), 1.0));
                }
            }
        }
        return answer;
    }
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const {
            std::string str{raw_query};
            Query query = ParseQuery(str);
 
            std::vector<std::string_view> matched_words;
            for (const std::string& word : query.plus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    matched_words.push_back(static_cast<std::string_view>(word));
                }
            }
            for (const std::string& word : query.minus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    matched_words.clear();
                    break;
                }
            }
            return {matched_words, documents_.at(document_id).status};
    }
    
    template <typename Execution>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(Execution&& policy, std::string_view raw_query, int document_id) const {
            std::string str{raw_query};
            Query query = ParseQuery(str);
    
            std::vector<std::string_view> matched_words;
            for (const std::string& word : query.plus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    matched_words.push_back(static_cast<std::string_view>(word));
                }
            }
            for (const std::string& word : query.minus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    matched_words.clear();
                    break;
                }
            }
            return {matched_words, documents_.at(document_id).status};
    }
    
    void RemoveDocument(int document_id)
    {
        int i=0;
        for(const auto& id:document_ids_)
        {
            if(id==document_id)
            {
                break;
            }
            i++;
        }
        document_ids_.erase(document_ids_.begin()+i);
        for(auto& [x, z]:word_to_document_freqs_)
        {
            z.erase(document_id);
        }
        documents_.erase(document_id);
    }
    
    template <typename Execution>
    void RemoveDocument(const Execution&, int document_id)
    {
        int i=0;
        for(const auto& id:document_ids_)
        {
            if(id==document_id)
            {
                break;
            }
            i++;
        }
        document_ids_.erase(document_ids_.begin()+i);
        for(auto& [x, z]:word_to_document_freqs_)
        {
            z.erase(document_id);
        }
        documents_.erase(document_id);
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;

    bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    }

    static bool IsValidWord(const std::string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }

    std::vector<std::string> SplitIntoWordsNoStop(const std::string text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(static_cast<std::string_view>(text))) {
            if (!IsValidWord(word)) {
                throw std::invalid_argument("Word " + word + " is invalid");
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    int ComputeAverageRating(const std::vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };
    QueryWord ParseQueryWord(const std::string& text) const {
        if (text.empty()) {
            throw std::invalid_argument("Query word is empty");
        }
        std::string word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw std::invalid_argument("Query word " + text + " is invalid");
        }

        return {word, is_minus, IsStopWord(word)};
    }

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const {
        Query result;
        for (const std::string& word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.insert(query_word.data);
                } else {
                    result.plus_words.insert(query_word.data);
                }
            }
        }
        return result;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};
