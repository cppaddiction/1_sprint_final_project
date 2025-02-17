#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        for(const string& s:words)
        {
            double tf=double(count(words.begin(), words.end(), s))/double(words.size());
            documents_[s].insert({document_id, tf});
        }
        document_count_++;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    map<string, set<pair<int, double>>> documents_;
    set<string> stop_words_;
    int document_count_ = 0;
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
	    
	struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    double ComputeIdf(const double all_documents_amount, const double word_found_documents_amount) const	
    {
        return log(all_documents_amount/word_found_documents_amount);
    }		
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
	
    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> id_and_relevance;
        vector<Document> matched_documents;
        for (const string& word:query_words.plus_words)
        {
            if(documents_.find(word) != documents_.end())
            {
                double idf=ComputeIdf(double(document_count_), double(documents_.at(word).size()));
                for (auto [id, relevance] : documents_.at(word)) {
                    id_and_relevance[id]+=idf*relevance;
                } 
            }
        }
        for (const string& word:query_words.minus_words)
        { 
            if(documents_.find(word) != documents_.end())
            {
                for (auto [id, relevance] : documents_.at(word)) {
                    id_and_relevance.erase(id);
                }
            }
        }
        for (auto [id, relevance] : id_and_relevance) {
            matched_documents.push_back({id, relevance});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}