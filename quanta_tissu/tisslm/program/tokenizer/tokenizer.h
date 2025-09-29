#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <utility>

class Tokenizer {
public:
    Tokenizer(const std::string& prefix);
    std::vector<int> encode(const std::string& text);
    std::string decode(const std::vector<int>& token_ids);
    int get_vocab_size() const;

private:
    std::map<std::pair<int, int>, int> merges;
    std::map<int, std::vector<unsigned char>> vocab;
    std::map<std::vector<unsigned char>, int> reverse_vocab;

    void load_merges(const std::string& merges_path);
    void load_vocab(const std::string& vocab_path);
    std::vector<int> bpe_encode(const std::vector<unsigned char>& bytes) const;
};

#endif // TOKENIZER_H