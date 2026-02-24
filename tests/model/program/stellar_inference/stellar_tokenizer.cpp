#include "stellar_tokenizer.h"
#include "tokenizer/pre_tokenizer.h"
#include "stellar_visualizer.h"
#include <iostream>
#include <algorithm>
#include <set>

namespace TissLM {
namespace Stellar {

namespace {
    std::vector<std::pair<int, int>> get_pairs(const std::vector<int>& ids) {
        std::vector<std::pair<int, int>> pairs;
        if (ids.size() < 2) return pairs;
        for (size_t i = 0; i < ids.size() - 1; ++i) pairs.emplace_back(ids[i], ids[i+1]);
        return pairs;
    }
}

StellarTokenizer::StellarTokenizer() {
    // Initialize with bytes
    for (int i = 0; i < 256; ++i) {
        vocab_[i] = {static_cast<unsigned char>(i)};
        reverse_vocab_[{static_cast<unsigned char>(i)}] = i;
    }
}

void StellarTokenizer::train(const std::string& text, int vocab_size, bool verbose) {
    if (vocab_size < 256) return;

    std::vector<std::string> words = TissLM::Tokenizer::pre_tokenize(text);
    std::vector<std::vector<int>> sequences;
    for (const auto& w : words) {
        std::vector<int> seq;
        for (char c : w) seq.push_back(static_cast<unsigned char>(c));
        sequences.push_back(seq);
    }

    int num_merges = vocab_size - 256;
    for (int i = 0; i < num_merges; ++i) {
        std::map<std::pair<int, int>, int> counts;
        for (const auto& seq : sequences) {
            auto ps = get_pairs(seq);
            for (const auto& p : ps) counts[p]++;
        }

        if (counts.empty()) break;

        auto best = std::max_element(counts.begin(), counts.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; })->first;

        int new_id = 256 + i;
        merges_[best] = i;
        ranked_merges_.push_back(best);

        std::vector<unsigned char> bytes = vocab_[best.first];
        bytes.insert(bytes.end(), vocab_[best.second].begin(), vocab_[best.second].end());
        vocab_[new_id] = bytes;
        reverse_vocab_[bytes] = new_id;

        for (auto& seq : sequences) {
            std::vector<int> next_seq;
            for (size_t j = 0; j < seq.size(); ) {
                if (j < seq.size() - 1 && seq[j] == best.first && seq[j+1] == best.second) {
                    next_seq.push_back(new_id);
                    j += 2;
                } else {
                    next_seq.push_back(seq[j]);
                    j++;
                }
            }
            seq = next_seq;
        }
        if (verbose && (i+1) % 10 == 0) std::cout << "\r[STELLAR] Merging... " << i+1 << "/" << num_merges << std::flush;
    }
    if (verbose) std::cout << std::endl;
}

void StellarTokenizer::train_stellar(const std::string& text, int vocab_size) {
    std::cout << "[STELLAR] High-Performance Tokenizer Training Initiated..." << std::endl;
    train(text, vocab_size, true);

    std::map<std::string, std::vector<float>> grid;
    grid["Final Vocab"] = {(float)get_vocab_size()};
    grid["Total Merges"] = {(float)ranked_merges_.size()};
    std::cout << StellarVisualizer::render_analysis_grid(grid, {"Value"});
}

std::vector<int> StellarTokenizer::encode(const std::string& text) {
    std::vector<int> all_ids;
    std::vector<std::string> words = TissLM::Tokenizer::pre_tokenize(text);
    for (const auto& w : words) {
        std::vector<unsigned char> bytes(w.begin(), w.end());
        auto ids = bpe_encode(bytes);
        all_ids.insert(all_ids.end(), ids.begin(), ids.end());
    }
    return all_ids;
}

std::vector<int> StellarTokenizer::bpe_encode(const std::vector<unsigned char>& bytes) const {
    if (bytes.empty()) return {};
    std::vector<int> ids(bytes.begin(), bytes.end());
    while (ids.size() >= 2) {
        auto pairs = get_pairs(ids);
        std::pair<int, int> best = {-1, -1};
        int min_rank = -1;
        for (const auto& p : pairs) {
            auto it = merges_.find(p);
            if (it != merges_.end() && (min_rank == -1 || it->second < min_rank)) {
                min_rank = it->second;
                best = p;
            }
        }
        if (min_rank == -1) break;
        std::vector<int> next_ids;
        for (size_t i = 0; i < ids.size(); ) {
            if (i < ids.size() - 1 && ids[i] == best.first && ids[i+1] == best.second) {
                next_ids.push_back(reverse_vocab_.at(vocab_.at(best.first))); // Wait, easier:
                // Actually, the new id is reverse_vocab_.at(combined_bytes)
                std::vector<unsigned char> combined = vocab_.at(best.first);
                combined.insert(combined.end(), vocab_.at(best.second).begin(), vocab_.at(best.second).end());
                next_ids.push_back(reverse_vocab_.at(combined));
                i += 2;
            } else {
                next_ids.push_back(ids[i]);
                i++;
            }
        }
        ids = next_ids;
    }
    return ids;
}

std::string StellarTokenizer::decode(const std::vector<int>& ids) {
    return decode_robust(ids);
}

std::string StellarTokenizer::decode_robust(const std::vector<int>& ids) const {
    std::vector<unsigned char> buf;
    for (int id : ids) {
        auto it = vocab_.find(id);
        if (it != vocab_.end()) buf.insert(buf.end(), it->second.begin(), it->second.end());
    }
    std::string text;
    for (size_t i = 0; i < buf.size(); ) {
        unsigned char b = buf[i];
        size_t len = 0;
        if (b < 0x80) len = 1;
        else if ((b & 0xE0) == 0xC0) len = 2;
        else if ((b & 0xF0) == 0xE0) len = 3;
        else if ((b & 0xF8) == 0xF0) len = 4;
        else { i++; continue; }

        if (i + len <= buf.size()) {
            bool valid = true;
            for (size_t j = 1; j < len; ++j) if ((buf[i+j] & 0xC0) != 0x80) { valid = false; break; }
            if (valid) {
                for (size_t j = 0; j < len; ++j) text += static_cast<char>(buf[i+j]);
                i += len;
            } else { i++; }
        } else { i++; }
    }
    return text;
}

int StellarTokenizer::get_vocab_size() const { return (int)vocab_.size(); }

} // namespace Stellar
} // namespace TissLM
