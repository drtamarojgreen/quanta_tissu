#include "tokenizer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>
#include <set>

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

// Helper function to find consecutive pairs of IDs in a list.
std::set<std::pair<int, int>> get_pairs(const std::vector<int>& ids) {
    std::set<std::pair<int, int>> pairs;
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        pairs.insert({ids[i], ids[i+1]});
    }
    return pairs;
}

Tokenizer::Tokenizer(const std::string& prefix) {
    if (!prefix.empty()) {
        std::string vocab_path = prefix + "_vocab.json";
        std::string merges_path = prefix + "_merges.txt";
        load_vocab(vocab_path);
        load_merges(merges_path);
    }
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
        auto pairs = get_pairs(ids);
        auto best_pair_it = std::min_element(pairs.begin(), pairs.end(),
            [this](const auto& a, const auto& b) {
                auto it_a = this->merges.find(a);
                auto it_b = this->merges.find(b);
                if (it_a == this->merges.end()) return false;
                if (it_b == this->merges.end()) return true;
                return it_a->second < it_b->second;
            });

        if (this->merges.find(*best_pair_it) == this->merges.end()) {
            break; // No more merges are possible
        }

        std::pair<int, int> best_pair = *best_pair_it;
        int new_id = this->merges.at(best_pair);
        std::vector<int> new_ids;
        for (size_t i = 0; i < ids.size();) {
            if (i < ids.size() - 1 && ids[i] == best_pair.first && ids[i+1] == best_pair.second) {
                new_ids.push_back(new_id);
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
    std::regex bpe_regex(R"('s|'t|'re|'ve|'m|'ll|'d| ?[[:alpha:]]+| ?[[:digit:]]+| ?[^\s[[:alpha:]][[:digit:]]]+|\s+(?!\S)|\s+)");
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

    std::string text;
    text.reserve(all_bytes.size());
    for (size_t i = 0; i < all_bytes.size(); ) {
        unsigned char byte1 = all_bytes[i];
        int num_bytes = 0;

        if (byte1 < 0x80) { // 0xxxxxxx (ASCII)
            num_bytes = 1;
        } else if ((byte1 & 0xE0) == 0xC0) { // 110xxxxx
            num_bytes = 2;
        } else if ((byte1 & 0xF0) == 0xE0) { // 1110xxxx
            num_bytes = 3;
        } else if ((byte1 & 0xF8) == 0xF0) { // 11110xxx
            num_bytes = 4;
        } else {
            // Invalid start byte, replace with replacement character
            text += "\xEF\xBF\xBD";
            i += 1;
            continue;
        }

        if (i + num_bytes > all_bytes.size()) {
            // Incomplete character at the end
            text += "\xEF\xBF\xBD";
            break;
        }

        // Check for valid continuation bytes
        bool valid = true;
        for (int j = 1; j < num_bytes; ++j) {
            if ((all_bytes[i + j] & 0xC0) != 0x80) {
                valid = false;
                break;
            }
        }

        if (valid) {
            text.append(reinterpret_cast<const char*>(&all_bytes[i]), num_bytes);
        } else {
            text += "\xEF\xBF\xBD";
        }
        i += num_bytes;
    }
    
    return text;
}

int Tokenizer::get_vocab_size() const {
    return vocab.size();
}

#include <regex>
#include <algorithm>
#include <set>

void Tokenizer::train(const std::string& text, int vocab_size, bool verbose) {
    if (vocab_size < 256) {
        throw std::invalid_argument("Vocabulary size must be at least 256 to cover all bytes.");
    }

    // 1. Pre-tokenize the text
    std::regex pre_tokenizer(R"('s|'t|'re|'ve|'m|'ll|'d| ?[[:alpha:]]+| ?[[:digit:]]+| ?[^\s[[:alpha:]][[:digit:]]]+|\s+(?!\S)|\s+)");
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), pre_tokenizer);
    auto words_end = std::sregex_iterator();

    std::vector<std::vector<int>> word_byte_sequences;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string word = match.str();
        std::vector<int> byte_sequence;
        for (char c : word) {
            byte_sequence.push_back(static_cast<unsigned char>(c));
        }
        word_byte_sequences.push_back(byte_sequence);
    }

    // 2. Initialize vocabulary with all individual bytes (0-255)
    this->vocab.clear();
    for (int i = 0; i < 256; ++i) {
        this->vocab[i] = {static_cast<unsigned char>(i)};
    }

    // 3. Iteratively merge the most frequent pair of tokens
    int num_merges = vocab_size - 256;
    for (int i = 0; i < num_merges; ++i) {
        std::map<std::pair<int, int>, int> pair_counts;
        for (const auto& sequence : word_byte_sequences) {
            for (const auto& pair : get_pairs(sequence)) {
                pair_counts[pair]++;
            }
        }

        if (pair_counts.empty()) {
            break; // No more pairs to merge
        }

        auto best_pair = std::max_element(pair_counts.begin(), pair_counts.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            })->first;

        int new_token_id = 256 + i;
        this->merges[best_pair] = new_token_id;
        std::vector<unsigned char> new_token_bytes = this->vocab[best_pair.first];
        new_token_bytes.insert(new_token_bytes.end(), this->vocab[best_pair.second].begin(), this->vocab[best_pair.second].end());
        this->vocab[new_token_id] = new_token_bytes;

        std::vector<std::vector<int>> new_word_byte_sequences;
        for (const auto& sequence : word_byte_sequences) {
            std::vector<int> new_sequence;
            for (size_t j = 0; j < sequence.size();) {
                if (j < sequence.size() - 1 && sequence[j] == best_pair.first && sequence[j+1] == best_pair.second) {
                    new_sequence.push_back(new_token_id);
                    j += 2;
                } else {
                    new_sequence.push_back(sequence[j]);
                    j += 1;
                }
            }
            new_word_byte_sequences.push_back(new_sequence);
        }
        word_byte_sequences = new_word_byte_sequences;

        if (verbose) {
            std::cout << "Merge " << i+1 << "/" << num_merges << ": (" << best_pair.first << ", " << best_pair.second << ") -> " << new_token_id << std::endl;
        }
    }

    // Create the reverse vocabulary for decoding
    this->reverse_vocab.clear();
    for (const auto& pair : this->vocab) {
        this->reverse_vocab[pair.second] = pair.first;
    }
}

void Tokenizer::save(const std::string& prefix) {
    std::string vocab_path = prefix + "_vocab.json";
    std::string merges_path = prefix + "_merges.txt";

    // Save vocabulary
    std::ofstream vocab_file(vocab_path);
    if (vocab_file.is_open()) {
        vocab_file << "{";
        bool first = true;
        for (const auto& pair : this->vocab) {
            if (!first) {
                vocab_file << ", ";
            }
            vocab_file << "\"" << pair.first << "\": [";
            bool first_byte = true;
            for (unsigned char byte : pair.second) {
                if (!first_byte) {
                    vocab_file << ", ";
                }
                vocab_file << static_cast<int>(byte);
                first_byte = false;
            }
            vocab_file << "]";
            first = false;
        }
        vocab_file << "}";
    }

    // Save merges
    std::ofstream merges_file(merges_path);
    if (merges_file.is_open()) {
        for (const auto& pair : this->merges) {
            merges_file << pair.first.first << " " << pair.first.second << " " << pair.second << std::endl;
        }
    }
}
