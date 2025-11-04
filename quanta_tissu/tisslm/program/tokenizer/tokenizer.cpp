#include "tokenizer.h"
#include "pre_tokenizer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>
#include <set>

namespace TissLM {
namespace Tokenizer {

// Helper function to find consecutive pairs of IDs in a list.
std::set<std::pair<int, int>> get_pairs(const std::vector<int>& ids) {
    std::set<std::pair<int, int>> pairs;
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        pairs.insert({ids[i], ids[i+1]});
    }
    return pairs;
}

// A simple JSON parser for the vocabulary
std::map<int, std::vector<unsigned char>> parse_vocab_from_json(const std::string& content) {
    std::map<int, std::vector<unsigned char>> vocab;
    std::regex re_pair(R"(\"(\d+)\"\s*:\s*\[([\s\d,]*)\])");
    std::smatch match;
    std::string::const_iterator search_start(content.cbegin());

    while (std::regex_search(search_start, content.cend(), match, re_pair)) {
        int key = std::stoi(match[1]);
        std::string byte_str = match[2];
        std::vector<unsigned char> bytes;
        std::stringstream ss(byte_str);
        std::string byte_val;
        while (std::getline(ss, byte_val, ',')) {
            // Trim leading/trailing whitespace from the number string
            byte_val.erase(0, byte_val.find_first_not_of(" \t\n\r"));
            byte_val.erase(byte_val.find_last_not_of(" \t\n\r") + 1);
            if (!byte_val.empty()) {
                bytes.push_back(static_cast<unsigned char>(std::stoi(byte_val)));
            }
        }
        vocab[key] = bytes;
        search_start = match.suffix().first;
    }
    return vocab;
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
    int rank = 0;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::stringstream ss(line);
        std::string part1, part2;
        ss >> part1 >> part2;
        if (!part1.empty() && !part2.empty()) {
            try {
                this->merges[{std::stoi(part1), std::stoi(part2)}] = rank++;
            } catch (const std::invalid_argument& e) {
                // Ignore malformed lines
            }
        }
    }
}

std::vector<int> Tokenizer::bpe_encode(const std::vector<unsigned char>& bytes) const {
    if (bytes.empty()) {
        return {};
    }
    std::vector<int> ids(bytes.begin(), bytes.end());

    while (ids.size() >= 2) {
        auto pairs = get_pairs(ids);

        // Find the best pair with the lowest merge rank
        std::pair<int, int> best_pair = {-1, -1};
        int min_rank = -1;

        for (const auto& p : pairs) {
            auto it = this->merges.find(p);
            if (it != this->merges.end()) {
                if (min_rank == -1 || it->second < min_rank) {
                    min_rank = it->second;
                    best_pair = p;
                }
            }
        }

        if (min_rank == -1) {
            break; // No more merges are possible
        }

        auto it1 = vocab.find(best_pair.first);
        auto it2 = vocab.find(best_pair.second);
        if (it1 == vocab.end() || it2 == vocab.end()) {
            break;
        }

        std::vector<unsigned char> new_bytes = it1->second;
        new_bytes.insert(new_bytes.end(), it2->second.begin(), it2->second.end());

        auto rev_it = reverse_vocab.find(new_bytes);
        if (rev_it == reverse_vocab.end()) {
            break;
        }
        int new_id = rev_it->second;

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
     if (vocab.empty() || merges.empty()) { // Fallback if vocab/merges not loaded
        std::vector<int> byte_ids;
        for(unsigned char c : text) {
            byte_ids.push_back(static_cast<int>(c));
        }
        return byte_ids;
    }
    std::vector<int> all_ids;
    std::vector<std::string> words = pre_tokenize(text);
    for (const auto& word : words) {
        std::vector<unsigned char> word_bytes(word.begin(), word.end());
        std::vector<int> word_ids = bpe_encode(word_bytes);
        all_ids.insert(all_ids.end(), word_ids.begin(), word_ids.end());
    }
    return all_ids;
}

std::string Tokenizer::decode(const std::vector<int>& token_ids) {
    std::vector<unsigned char> byte_buffer;
    for (int id : token_ids) {
        auto it = vocab.find(id);
        if (it != vocab.end()) {
            byte_buffer.insert(byte_buffer.end(), it->second.begin(), it->second.end());
        }
    }

    // This is a simplified UTF-8 decoder. A more robust implementation would handle invalid sequences.
    std::string text;
    for (size_t i = 0; i < byte_buffer.size(); ) {
        unsigned char byte = byte_buffer[i];
        if (byte < 0x80) { // 1-byte sequence
            text += static_cast<char>(byte);
            i += 1;
        } else if ((byte & 0xE0) == 0xC0) { // 2-byte sequence
            if (i + 1 < byte_buffer.size()) {
                text += static_cast<char>(byte);
                text += static_cast<char>(byte_buffer[i+1]);
                i += 2;
            } else {
                i += 1; // Incomplete sequence
            }
        } else if ((byte & 0xF0) == 0xE0) { // 3-byte sequence
            if (i + 2 < byte_buffer.size()) {
                text += static_cast<char>(byte);
                text += static_cast<char>(byte_buffer[i+1]);
                text += static_cast<char>(byte_buffer[i+2]);
                i += 3;
            } else {
                i += 1; // Incomplete sequence
            }
        } else if ((byte & 0xF8) == 0xF0) { // 4-byte sequence
            if (i + 3 < byte_buffer.size()) {
                text += static_cast<char>(byte);
                text += static_cast<char>(byte_buffer[i+1]);
                text += static_cast<char>(byte_buffer[i+2]);
                text += static_cast<char>(byte_buffer[i+3]);
                i += 4;
            } else {
                i += 1; // Incomplete sequence
            }
        } else {
            // Invalid byte, skip it
            i += 1;
        }
    }
    return text;
}

int Tokenizer::get_vocab_size() const {
    return vocab.size();
}

void Tokenizer::train(const std::string& text, int vocab_size, bool verbose) {
    if (vocab_size < 256) {
        throw std::invalid_argument("Vocabulary size must be at least 256 to cover all bytes.");
    }

    // 1. Pre-tokenize the text
    std::vector<std::string> words = pre_tokenize(text);
    std::vector<std::vector<int>> word_byte_sequences;
    for (const auto& word : words) {
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
            merges_file << pair.first.first << " " << pair.first.second << std::endl;
        }
    }
}

} // namespace Tokenizer
} // namespace TissLM
