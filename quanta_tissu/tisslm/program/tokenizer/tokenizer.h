#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <utility>

namespace TissLM {
namespace Tokenizer {

class Tokenizer {
public:
    Tokenizer(const std::string& prefix);
    std::vector<int> encode(const std::string& text);
    std::string decode(const std::vector<int>& token_ids);
    int get_vocab_size() const;

    void train(const std::string& text, int vocab_size, bool verbose = false);
    void save(const std::string& prefix);

private:
    std::map<std::pair<int, int>, int> merges;
    std::vector<std::pair<int, int>> ranked_merges;
    std::map<int, std::vector<unsigned char>> vocab;
    std::map<std::vector<unsigned char>, int> reverse_vocab;

    void load_merges(const std::string& merges_path);
    void load_vocab(const std::string& vocab_path);
    std::vector<int> bpe_encode(const std::vector<unsigned char>& bytes) const;
};

} // namespace Tokenizer
} // namespace TissLM

#endif // TOKENIZER_H