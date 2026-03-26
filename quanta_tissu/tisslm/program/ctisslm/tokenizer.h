#ifndef QUANTA_TISSU_TOKENIZER_H
#define QUANTA_TISSU_TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "tissdb/json/json.h"

namespace quanta_tissu {

std::map<std::pair<int, int>, int> get_pairs(const std::vector<int>& ids) {
    std::map<std::pair<int, int>, int> pairs;
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        pairs[{ids[i], ids[i+1]}]++;
    }
    return pairs;
}

class BPETokenizer {
public:
    std::map<std::pair<int, int>, int> merges;
    std::map<int, std::vector<unsigned char>> vocab;
    std::map<std::vector<unsigned char>, int> reverse_vocab;

    BPETokenizer() {}

    void train(const std::string& text, int vocab_size, bool verbose = false) {
        vocab.clear(); merges.clear(); reverse_vocab.clear();
        std::vector<int> ids;
        for (unsigned char c : text) {
            ids.push_back(static_cast<int>(c));
            if (vocab.find(static_cast<int>(c)) == vocab.end()) {
                vocab[static_cast<int>(c)] = {c};
                reverse_vocab[{c}] = static_cast<int>(c);
            }
        }
        int next_id = 256;
        int num_merges = vocab_size - 256;
        for (int i = 0; i < num_merges; ++i) {
            auto pairs = get_pairs(ids);
            if (pairs.empty()) break;
            std::pair<int, int> best_pair = { -1, -1 };
            int max_freq = -1;
            for (auto const& [pair, freq] : pairs) {
                if (freq > max_freq) { max_freq = freq; best_pair = pair; }
            }
            if (max_freq < 2) break;
            merges[best_pair] = next_id;
            std::vector<unsigned char> new_bytes = vocab[best_pair.first];
            new_bytes.insert(new_bytes.end(), vocab[best_pair.second].begin(), vocab[best_pair.second].end());
            vocab[next_id] = new_bytes;
            reverse_vocab[new_bytes] = next_id;
            std::vector<int> new_ids;
            for (size_t j = 0; j < ids.size(); ++j) {
                if (j < ids.size() - 1 && ids[j] == best_pair.first && ids[j+1] == best_pair.second) {
                    new_ids.push_back(next_id); j++;
                } else new_ids.push_back(ids[j]);
            }
            ids = new_ids; next_id++;
            if (verbose && (i + 1) % 10 == 0) std::cout << "Merge " << (i + 1) << "/" << num_merges << " completed." << std::endl;
        }
    }

    const std::regex BPE_SPLIT_PATTERN = std::regex(R"('(?:[sdmt]|ll|ve|re)| ?[a-zA-Z]+| ?[0-9]+| ?[^\sa-zA-Z0-9]+|\s+(?!\S)|\s+)", std::regex::ECMAScript | std::regex::optimize);

    std::vector<int> _encode_chunk(const std::vector<unsigned char>& text_bytes) {
        std::vector<int> ids;
        for (unsigned char byte : text_bytes) ids.push_back(static_cast<int>(byte));
        while (true) {
            std::pair<int, int> best_pair = {-1, -1};
            int min_new_id = std::numeric_limits<int>::max();
            for (size_t i = 0; i < ids.size() - 1; ++i) {
                std::pair<int, int> current_pair = {ids[i], ids[i+1]};
                if (merges.count(current_pair)) {
                    int new_id = merges[current_pair];
                    if (new_id < min_new_id) { min_new_id = new_id; best_pair = current_pair; }
                }
            }
            if (best_pair.first == -1) break;
            int new_id = merges[best_pair];
            std::vector<int> new_ids;
            size_t i = 0;
            while (i < ids.size()) {
                if (i < ids.size() - 1 && ids[i] == best_pair.first && ids[i+1] == best_pair.second) {
                    new_ids.push_back(new_id); i += 2;
                } else { new_ids.push_back(ids[i]); i += 1; }
            }
            ids = new_ids;
        }
        return ids;
    }

    std::vector<int> encode(const std::string& text) {
        std::vector<int> all_ids;
        std::sregex_iterator current_match(text.begin(), text.end(), BPE_SPLIT_PATTERN), last_match;
        while (current_match != last_match) {
            std::string word = current_match->str();
            std::vector<unsigned char> text_bytes(word.begin(), word.end());
            std::vector<int> encoded_chunk = _encode_chunk(text_bytes);
            all_ids.insert(all_ids.end(), encoded_chunk.begin(), encoded_chunk.end());
            current_match++;
        }
        return all_ids;
    }

    std::string decode(const std::vector<int>& ids) {
        std::vector<unsigned char> text_bytes;
        for (int id : ids) {
            if (vocab.count(id)) {
                const auto& bytes = vocab[id];
                text_bytes.insert(text_bytes.end(), bytes.begin(), bytes.end());
            } else text_bytes.push_back(static_cast<unsigned char>('?'));
        }
        return std::string(text_bytes.begin(), text_bytes.end());
    }

    void save(const std::string& prefix) {
        std::string vocab_file = prefix + "_vocab.json";
        std::string merges_file = prefix + "_merges.txt";
        std::ofstream vocab_ofs(vocab_file);
        if (vocab_ofs.is_open()) {
            TissDB::Json::JsonObject obj;
            for (const auto& pair : vocab) {
                TissDB::Json::JsonArray arr;
                for (auto b : pair.second) arr.push_back((double)b);
                obj[std::to_string(pair.first)] = arr;
            }
            vocab_ofs << TissDB::Json::JsonValue(obj).serialize();
            vocab_ofs.close();
        }
        std::ofstream merges_ofs(merges_file);
        if (merges_ofs.is_open()) {
            for (const auto& pair : merges) merges_ofs << pair.first.first << " " << pair.first.second << " " << pair.second << "\n";
            merges_ofs.close();
        }
    }

    void load(const std::string& prefix) {
        std::string vocab_file = prefix + "_vocab.json";
        std::string merges_file = prefix + "_merges.txt";
        std::ifstream vocab_ifs(vocab_file);
        if (!vocab_ifs.is_open()) throw std::runtime_error("Failed to open vocab file.");
        std::stringstream ss; ss << vocab_ifs.rdbuf(); vocab_ifs.close();
        TissDB::Json::JsonValue root = TissDB::Json::JsonValue::parse(ss.str());
        vocab.clear();
        for (const auto& pair : root.as_object()) {
            int id = std::stoi(pair.first);
            std::vector<unsigned char> bytes;
            for (const auto& b : pair.second.as_array()) bytes.push_back(static_cast<unsigned char>(b.as_number()));
            vocab[id] = bytes;
        }
        merges.clear();
        std::ifstream merges_ifs(merges_file);
        if (merges_ifs.is_open()) {
            int p1, p2, new_id;
            while (merges_ifs >> p1 >> p2 >> new_id) merges[{p1, p2}] = new_id;
            merges_ifs.close();
        }
        reverse_vocab.clear();
        for (const auto& pair : vocab) reverse_vocab[pair.second] = pair.first;
    }

    size_t get_vocab_size() const { return vocab.size(); }

    int get_token_id(const std::string& token) {
        std::vector<unsigned char> bytes(token.begin(), token.end());
        if (reverse_vocab.count(bytes)) return reverse_vocab[bytes];
        return 0;
    }

    std::string get_token(int token_id) {
        if (vocab.count(token_id)) return std::string(vocab[token_id].begin(), vocab[token_id].end());
        return "?";
    }
};

class Tokenizer {
public:
    BPETokenizer bpe_tokenizer;
    int unk_token_id = 0, pad_token_id = 1;
    Tokenizer(const std::string& path = "") { if (!path.empty()) bpe_tokenizer.load(path); }
    std::vector<int> tokenize(const std::string& text) { return bpe_tokenizer.encode(text); }
    std::string detokenize(const std::vector<int>& ids) {
        std::string res;
        for (size_t i = 0; i < ids.size(); ++i) {
            std::string t = bpe_tokenizer.get_token(ids[i]);
            if (i > 0 && !t.empty() && std::isalnum(t[0]) && !res.empty() && std::isalnum(res.back())) res += ' ';
            res += t;
        }
        return res;
    }
    size_t get_vocab_size() const { return bpe_tokenizer.get_vocab_size(); }
    int get_token_id(const std::string& t) { return bpe_tokenizer.get_token_id(t); }
    std::string get_token(int id) { return bpe_tokenizer.get_token(id); }
};

}

#endif // QUANTA_TISSU_TOKENIZER_H
