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

std::vector<std::pair<int, int>> get_pairs(const std::vector<int>& ids) {
    std::vector<std::pair<int, int>> pairs;
    if (ids.size() < 2) return pairs;
    for (size_t i = 0; i < ids.size() - 1; ++i) pairs.emplace_back(ids[i], ids[i+1]);
    return pairs;
}

std::map<int, std::vector<unsigned char>> parse_vocab_from_json(const std::string& content) {
    std::map<int, std::vector<unsigned char>> vocab;
    std::regex re_pair(R"(\"(\d+)\"\s*:\s*\[([\s\d,]*)\])");
    std::smatch match;
    std::string::const_iterator search_start(content.cbegin());
    while (std::regex_search(search_start, content.cend(), match, re_pair)) {
        int key = std::stoi(match[1]);
        std::vector<unsigned char> bytes;
        std::stringstream ss(match[2].str());
        std::string val;
        while (std::getline(ss, val, ',')) {
            val.erase(0, val.find_first_not_of(" \t\n\r")); val.erase(val.find_last_not_of(" \t\n\r") + 1);
            if (!val.empty()) bytes.push_back(static_cast<unsigned char>(std::stoi(val)));
        }
        vocab[key] = bytes; search_start = match.suffix().first;
    }
    return vocab;
}

Tokenizer::Tokenizer(const std::string& prefix) {
    if (!prefix.empty()) {
        load_vocab(prefix + "_vocab.json");
        load_merges(prefix + "_merges.txt");
    }
}

void Tokenizer::load_vocab(const std::string& path) {
    std::ifstream f(path); if (!f.is_open()) return;
    std::string c((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    this->vocab = parse_vocab_from_json(c);
    for (const auto& p : this->vocab) this->reverse_vocab[p.second] = p.first;
}

void Tokenizer::load_merges(const std::string& path) {
    std::ifstream f(path); if (!f.is_open()) return;
    std::string l; int r = 0;
    while (std::getline(f, l)) {
        if (l.empty() || l[0] == '#') continue;
        std::stringstream ss(l); std::string p1, p2; ss >> p1 >> p2;
        if (!p1.empty() && !p2.empty()) this->merges[{std::stoi(p1), std::stoi(p2)}] = r++;
    }
}

std::vector<int> Tokenizer::bpe_encode(const std::vector<unsigned char>& bytes) const {
    if (bytes.empty()) return {};
    std::vector<int> ids(bytes.begin(), bytes.end());
    while (ids.size() >= 2) {
        auto pairs = get_pairs(ids);
        std::pair<int, int> best = {-1, -1}; int min_r = -1;
        for (const auto& p : pairs) {
            auto it = this->merges.find(p);
            if (it != this->merges.end() && (min_r == -1 || it->second < min_r)) { min_r = it->second; best = p; }
        }
        if (min_r == -1) break;
        std::vector<unsigned char> nb = vocab.at(best.first);
        nb.insert(nb.end(), vocab.at(best.second).begin(), vocab.at(best.second).end());
        int nid = reverse_vocab.at(nb);
        std::vector<int> nids;
        for (size_t i = 0; i < ids.size();) {
            if (i < ids.size() - 1 && ids[i] == best.first && ids[i+1] == best.second) { nids.push_back(nid); i += 2; }
            else { nids.push_back(ids[i]); i += 1; }
        }
        ids = nids;
    }
    return ids;
}

std::vector<int> Tokenizer::encode(const std::string& text) {
    if (vocab.empty() || merges.empty()) { std::vector<int> b; for(unsigned char c : text) b.push_back(c); return b; }
    std::vector<int> res;
    for (const auto& w : pre_tokenize(text)) {
        std::vector<unsigned char> wb(w.begin(), w.end());
        std::vector<int> wi = bpe_encode(wb);
        res.insert(res.end(), wi.begin(), wi.end());
    }
    return res;
}

std::string Tokenizer::decode(const std::vector<int>& ids) {
    std::vector<unsigned char> buf;
    for (int id : ids) { auto it = vocab.find(id); if (it != vocab.end()) buf.insert(buf.end(), it->second.begin(), it->second.end()); }
    std::string res;
    for (size_t i = 0; i < buf.size(); ) {
        unsigned char b = buf[i];
        if (b < 0x80) { res += static_cast<char>(b); i += 1; }
        else if ((b & 0xE0) == 0xC0) { if (i + 1 < buf.size()) { res += (char)b; res += (char)buf[i+1]; i += 2; } else i++; }
        else if ((b & 0xF0) == 0xE0) { if (i + 2 < buf.size()) { res += (char)b; res += (char)buf[i+1]; res += (char)buf[i+2]; i += 3; } else i++; }
        else if ((b & 0xF8) == 0xF0) { if (i + 3 < buf.size()) { res += (char)b; res += (char)buf[i+1]; res += (char)buf[i+2]; res += (char)buf[i+3]; i += 4; } else i++; }
        else i++;
    }
    return res;
}

int Tokenizer::get_vocab_size() const { return vocab.size(); }

void Tokenizer::train(const std::string& text, int vocab_size, bool verbose) {
    if (vocab_size < 256) throw std::invalid_argument("Vocab size >= 256.");
    std::vector<std::vector<int>> seqs;
    for (const auto& w : pre_tokenize(text)) { std::vector<int> s; for (char c : w) s.push_back((unsigned char)c); seqs.push_back(s); }
    this->vocab.clear(); for (int i = 0; i < 256; ++i) this->vocab[i] = {(unsigned char)i};
    int n_m = vocab_size - 256;
    for (int i = 0; i < n_m; ++i) {
        std::map<std::pair<int, int>, int> counts;
        for (const auto& s : seqs) { auto ps = get_pairs(s); for (const auto& p : ps) counts[p]++; }
        if (counts.empty()) break;
        auto best = std::max_element(counts.begin(), counts.end(), [](const auto& a, const auto& b) { return a.second < b.second; })->first;
        int nid = 256 + i; this->merges[best] = i; this->ranked_merges.push_back(best);
        std::vector<unsigned char> nb = this->vocab[best.first]; nb.insert(nb.end(), this->vocab[best.second].begin(), this->vocab[best.second].end());
        this->vocab[nid] = nb;
        std::vector<std::vector<int>> nseqs;
        for (const auto& s : seqs) {
            std::vector<int> ns;
            for (size_t j = 0; j < s.size();) {
                if (j < s.size() - 1 && s[j] == best.first && s[j+1] == best.second) { ns.push_back(nid); j += 2; }
                else { ns.push_back(s[j]); j += 1; }
            }
            nseqs.push_back(ns);
        }
        seqs = nseqs;
        if (verbose) std::cout << "Merge " << i+1 << "/" << n_m << std::endl;
    }
    this->reverse_vocab.clear(); for (const auto& p : this->vocab) this->reverse_vocab[p.second] = p.first;
}

void Tokenizer::save(const std::string& prefix) {
    std::ofstream vf(prefix + "_vocab.json");
    if (vf.is_open()) {
        vf << "{"; bool f = true;
        for (const auto& p : this->vocab) {
            if (!f) vf << ", "; vf << "\"" << p.first << "\": ["; bool fb = true;
            for (unsigned char b : p.second) { if (!fb) vf << ", "; vf << (int)b; fb = false; }
            vf << "]"; f = false;
        }
        vf << "}";
    }
    std::ofstream mf(prefix + "_merges.txt");
    if (mf.is_open()) for (const auto& p : this->ranked_merges) mf << p.first << " " << p.second << std::endl;
}

}
}
