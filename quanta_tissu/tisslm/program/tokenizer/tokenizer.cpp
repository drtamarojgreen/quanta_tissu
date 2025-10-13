#include "tokenizer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>

// A simple JSON parser for the vocabulary
std::map<int, std::vector<unsigned char>> parse_vocab_from_json(const std::string& content) {
    std::map<int, std::vector<unsigned char>> vocab;
    std::regex re_pair("\"(\\d+)\": \\[(\\d+(?:, \\d+)*)\\]");
    std::smatch match;
    std::string::const_iterator search_start(content.cbegin());

    while (std::regex_search(search_start, content.cend(), match, re_pair)) {
        int key = std::stoi(match[1]);
        std::string byte_str = match[2];
        std::vector<unsigned char> bytes;
        std::stringstream ss(byte_str);
        std::string byte_val;
        while (std::getline(ss, byte_val, ',')) {
            // Trim leading spaces
            size_t first = byte_val.find_first_not_of(" \t");
            if (std::string::npos != first) {
                byte_val = byte_val.substr(first);
            }
            bytes.push_back(static_cast<unsigned char>(std::stoi(byte_val)));
        }
        vocab[key] = bytes;
        search_start = match.suffix().first;
    }
    return vocab;
}

Tokenizer::Tokenizer(const std::string& prefix) {
    std::string vocab_path = prefix + "_vocab.json";
    std::string merges_path = prefix + "_merges.txt";
    load_vocab(vocab_path);
    load_merges(merges_path);
}

void Tokenizer::load_vocab(const std::string& vocab_path) {
    std::ifstream file(vocab_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open vocab file " << vocab_path << std::endl;
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    this->vocab = parse_vocab_from_json(content);
    for (const auto& pair : this->vocab) {
        this->reverse_vocab[pair.second] = pair.first;
    }
}

void Tokenizer::load_merges(const std::string& merges_path) {
    std::ifstream file(merges_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open merges file " << merges_path << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        int p1, p2, new_id;
        ss >> p1 >> p2 >> new_id;
        this->merges[{p1, p2}] = new_id;
    }
}

std::vector<int> Tokenizer::bpe_encode(const std::vector<unsigned char>& bytes) const {
    std::vector<int> ids(bytes.begin(), bytes.end());

    while (ids.size() >= 2) {
        auto best_pair_it = merges.end();
        int min_rank = -1;
        std::pair<int, int> best_pair;

        for (size_t i = 0; i < ids.size() - 1; ++i) {
            std::pair<int, int> pair = {ids[i], ids[i+1]};
            auto it = merges.find(pair);
            if (it != merges.end()) {
                if (min_rank == -1 || it->second < min_rank) {
                    min_rank = it->second;
                    best_pair = pair;
                }
            }
        }

        if (min_rank == -1) {
            break;
        }

        std::vector<int> new_ids;
        for (size_t i = 0; i < ids.size();) {
            if (i < ids.size() - 1 && ids[i] == best_pair.first && ids[i+1] == best_pair.second) {
                new_ids.push_back(merges.at(best_pair));
                i += 2;
            } else {
                new_ids.push_back(ids[i]);
                i += 1;
            }
        }
        ids = new_ids;
    }
    return ids;
}


std::vector<int> Tokenizer::encode(const std::string& text) {
    std::regex bpe_regex(R"('s|'t|'re|'ve|'m|'ll|'d| ?[A-Za-z]+| ?[0-9]+| ?[^\sA-Za-z0-9]+|\s+(?!\S)|\s+)");
    std::vector<int> all_ids;

    auto words_begin = std::sregex_iterator(text.begin(), text.end(), bpe_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string word = match.str();
        std::vector<unsigned char> word_bytes(word.begin(), word.end());
        std::vector<int> word_ids = bpe_encode(word_bytes);
        all_ids.insert(all_ids.end(), word_ids.begin(), word_ids.end());
    }
    return all_ids;
}

std::string Tokenizer::decode(const std::vector<int>& token_ids) {
    std::vector<unsigned char> all_bytes;
    for (int id : token_ids) {
        auto it = vocab.find(id);
        if (it != vocab.end()) {
            all_bytes.insert(all_bytes.end(), it->second.begin(), it->second.end());
        }
    }
    return std::string(all_bytes.begin(), all_bytes.end());
}

int Tokenizer::get_vocab_size() const {
    return vocab.size();
}