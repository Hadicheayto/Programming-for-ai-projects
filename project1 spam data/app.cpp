// Import libraries are included to support various functionalities:

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <cctype>
#include <regex>
using namespace std;


// This Class for representing a message with label, original message, cleaned message, and tokens.
class MessageRecord {
public:
    string label;
    string message;
    string clean_message;
    vector<string> tokens;

    MessageRecord(const string& label, const string& message) : label(label), message(message) {}
};

// Abstract base class for message processors that perform transformations on messages.
class MessageProcessor {
public:
    virtual void process(MessageRecord& record) const = 0;  
};

// Class for removing punctuation from a message and inherits from the MessageProcessor class.
class PunctuationRemover : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
        record.clean_message = regex_replace(record.message, regex(R"([^\w\s])"), "");
    }
};

// Class for removing numbers from a message and inherits from the MessageProcessor class.
class NumberRemover : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
        string result;
        for (char ch : record.clean_message) {
            if (!isdigit(ch)) {
                result += ch;
            }
        }
        record.clean_message = result;
    }
};

// Class for normalizing whitespace by replacing multiple spaces with a single space and inherits from the MessageProcessor class.
class WhitespaceNormalizer : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
       
        record.clean_message = regex_replace(record.clean_message, regex(R"(\s+)", regex::optimize), " ");
    }
};

// Class for converting all characters in the message to lowercase and inherits from the MessageProcessor class.
class LowercaseConverter : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
        transform(record.clean_message.begin(), record.clean_message.end(), record.clean_message.begin(), ::tolower);
    }
};

// Class for tokenizing the cleaned message into individual words and inherits from the MessageProcessor class.
class Tokenizer : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
        stringstream ss(record.clean_message);
        string word;
        while (ss >> word) {
            record.tokens.push_back(word);
        }
    }
};

// Class for removing common stopwords from the tokens and inherits from the MessageProcessor class.
class StopwordRemover : public MessageProcessor {
    unordered_set<string> stopwords = {"i", "me", "my", "myself", "we", "our", "ours", "ourselves", 
                                       "you", "your", "yours", "yourself", "yourselves", "he", "she",
                                       "it", "they", "them", "his", "her", "its", "ourselves", "the", 
                                       "is", "in", "at", "so", "to", "for", "and", "but", "until","there"};
public:
    void process(MessageRecord& record) const override {
        vector<string> filtered_tokens;
        for (const auto& token : record.tokens) {
            if (stopwords.find(token) == stopwords.end()) {
                filtered_tokens.push_back(token);
            }
        }
        record.tokens = filtered_tokens;
    }
};

// Class for stemming tokens by removing common suffixes like "ing" or "s" and inherits from the MessageProcessor class.
class Stemmer : public MessageProcessor {
public:
    void process(MessageRecord& record) const override {
        for (auto& token : record.tokens) {
            if (token.size() > 4 && token.substr(token.size() - 3) == "ing") {
                token = token.substr(0, token.size() - 3);
            }
            else if (token.size() > 3 && token.back() == 's') {
                token = token.substr(0, token.size() - 1);
            }
        }
    }
};

// Function to read the CSV file and load message data into MessageRecord objects.
vector<MessageRecord> readCSV(const string& filename) {
    vector<MessageRecord> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
        return data;
    }

    string line;
    bool header = true;

    while (getline(file, line)) {
        if (header) {
            header = false;
            continue;
        }
        
        stringstream ss(line);
        string label, message;
        getline(ss, label, ',');
        getline(ss, message);
        if (!message.empty() && message.front() == '"') {
            message.erase(0, 1);
        }
        if (!message.empty() && message.back() == '"') {
            message.pop_back();
        }
        
        data.emplace_back(label, message);
    }

    file.close();
    return data;
}

// Function to preprocess messages using a sequence of processors.
void preprocessMessages(vector<MessageRecord>& messages, const vector<MessageProcessor*>& processors) {
    for (auto& msg : messages) {
        for (const auto& processor : processors) {
            processor->process(msg);
        }
    }
}

// Exemple usage:
int main() {

    string filename = "spam.csv";
    vector<MessageRecord> messages = readCSV(filename);


    vector<MessageProcessor*> processors = {
        new PunctuationRemover(),
        new NumberRemover(),
        new LowercaseConverter(),
        new Tokenizer(),
        new StopwordRemover(),
        new Stemmer(),
        new WhitespaceNormalizer()  
    };


    preprocessMessages(messages, processors);


    for (size_t i = 0; i < min(messages.size(), size_t(5)); ++i) {
        cout << "Label: " << messages[i].label << ", Original Message: " << messages[i].message << endl;
        cout << "Processed Tokens: ";
        for (const auto& token : messages[i].tokens) {
            cout << token << " ";
        }
        cout << endl << endl;
    }


    for (auto processor : processors) {
        delete processor;
    }
    

    return 0;
}
